#include "nccc_edc.h"

#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QDateTime>

NcccEDC::NcccEDC(QObject *parent)
    : QSerialPort(parent)
{
    this->setBaudRate(QSerialPort::Baud9600);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    // initialize single shot timer
    tmr_wait_receive_ = new QTimer(this);
    tmr_wait_receive_->setInterval(60 * 1000);
    tmr_wait_receive_->setSingleShot(true);

    // initialize signals and slots
    connect(tmr_wait_receive_, SIGNAL(timeout()), this, SLOT(receiveTimeout()));
    connect(this, SIGNAL(readyRead()), this, SLOT(rxDataReady()));

    state_ = IDLE;
}

NcccEDC::~NcccEDC()
{

}

bool NcccEDC::executeTransaction(PaymentType p_type, int p_amount)
{
    qDebug() << "[EDC] transaction start...";

#ifdef _DEV_PAY_FREE_
    ecr_version_ = "";
    trans_type_ = "";
    trans_id_ = "";
    host_id_ = "";
    receipt_no_ = "";
    card_no_ = "";
    trans_amount_ = QString("%1").arg(p_amount, 10, 10, QLatin1Char('0'));
    trans_date_ = QDateTime::currentDateTime().toString("yyMMdd");
    trans_time_ =  QDateTime::currentDateTime().toString("hhmmss");
    approval_no_ = "";
    response_code_ = RCODE_Approval;
    merchant_id_ = "";
    terminal_id_ = "";
    batch_no_ = "";
    if (p_type == iPass)
        card_type_ = C_TYPE_iPass;
    else if (p_type == EasyCard)
        card_type_ = C_TYPE_EasyCard;
    else if (p_type == iCash)
        card_type_ = C_TYPE_iCash;
    else if (p_type == HappyCash)
        card_type_ = C_TYPE_HappyCash;
    else
        card_type_ = "";

    qDebug() << "[EDC] transaction success";
    emit executeTransactionResponse(true);
#else
    QByteArray tx_data;
    QByteArray rx_data;

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[EDC] device can't be opened";
        return false;
    }

    // create tx data
    tx_data.append(STX);
    tx_data.append('I');
    tx_data.append(LEN_ECR_Version, ZERO);
    tx_data.append(LEN_Trans_Type_Indicator, ZERO);
    tx_data.append(T_TYPE_Normal);

    if (p_type == iPass || p_type == EasyCard || p_type == iCash || p_type == HappyCash)
        tx_data.append('E');
    else if (p_type == CreditCard)
        tx_data.append('N');
    else {
        qDebug() << "[EDC] payment type is illegal";
        return false;
    }

    tx_data.append(LEN_Host_ID, ZERO);
    tx_data.append(LEN_Receipt_No, ZERO);
    tx_data.append(LEN_Card_No, ZERO);
    tx_data.append(LEN_Card_Expire_Date, ZERO);
    tx_data.append(QString("%1%2").arg(p_amount, 10, 10, QLatin1Char('0')).arg("00"));
    tx_data.append(LEN_Trans_Date, ZERO);
    tx_data.append(LEN_Trans_Time, ZERO);
    tx_data.append(LEN_Approval_No, ZERO);
    tx_data.append(LEN_Wave_Card_ID, ZERO);
    tx_data.append(LEN_ECR_Response_Code, ZERO);
    tx_data.append(LEN_Merchant_ID, ZERO);
    tx_data.append(LEN_Terminal_ID, ZERO);
    tx_data.append(LEN_Exp_Amount, ZERO);
    tx_data.append(LEN_Store_ID, ZERO);
    tx_data.append(LEN_Installment_Indicator, ZERO);
    tx_data.append(LEN_RDM_Amount, ZERO);
    tx_data.append(LEN_RDM_Point, ZERO);
    tx_data.append(LEN_Balance_Point, ZERO);
    tx_data.append(LEN_Redeem_Amount, ZERO);
    tx_data.append(LEN_Installment_Period, ZERO);
    tx_data.append(LEN_Down_Amount, ZERO);
    tx_data.append(LEN_Installment_Amount, ZERO);
    tx_data.append(LEN_Installment_Fee, ZERO);
    tx_data.append(LEN_Card_Type, ZERO);
    tx_data.append(LEN_Batch_No, ZERO);
    tx_data.append(LEN_Start_Trans_Type, ZERO);
    tx_data.append(LEN_MP_Flag, ZERO);
    tx_data.append(LEN_SP_ISSUER_ID, ZERO);
    tx_data.append(LEN_SP_Origin_Return, ZERO);
    tx_data.append(LEN_SP_Origin_RRN, ZERO);
    tx_data.append(LEN_Pay_Item, ZERO);
    tx_data.append(LEN_Card_No_Hash, ZERO);
    tx_data.append(LEN_MP_Response_Code, ZERO);
    tx_data.append(LEN_ASM_Award_Flag, ZERO);
    tx_data.append(LEN_MCP_Indicator, ZERO);
    tx_data.append(LEN_Bank_No, ZERO);
    tx_data.append(LEN_Reserved, ZERO);
    tx_data.append(LEN_Happy_Go_Data, ZERO);
    tx_data.append(ETX);

    // calculate check sum
    char check_sum = 0x00;
    for (int i = 1; i < (LEN_Total_Package - 1); i++) {
        check_sum ^= tx_data[i];
    }
    tx_data.append(check_sum);

    // check the length of pageage
    if (tx_data.size() != LEN_Total_Package) {
        qDebug() << "[EDC] the length of TX data is illegal" << tx_data.size();
        return false;
    }

    qDebug() << "[EDC] TX data:" << tx_data;

    // write data
    this->write(tx_data);
    this->waitForBytesWritten(5000);

    // step 1. wait for ack
    if (this->waitForReadyRead(5000)) {
        rx_data = this->readAll();

        while (this->waitForReadyRead(20) && rx_data.length() < 2) {
            QByteArray possible_lost = this->readAll();
            rx_data += possible_lost;
        }
    }

    qDebug() << "[EDC] RX data:" << rx_data;

    // step 2. start timer to wait for result
    state_ = WAIT_TRANSACATION;
    tmr_wait_receive_->start();
#endif
    return true;
}

bool NcccEDC::executeSettlement()
{
    qDebug() << "[EDC] settlement start...";

    QByteArray tx_data;
    QByteArray rx_data;

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[EDC] device can't be opened";
        return false;
    }

    // create tx data
    tx_data.append(STX);
    tx_data.append('I');
    tx_data.append(LEN_ECR_Version, ZERO);
    tx_data.append(LEN_Trans_Type_Indicator, ZERO);
    tx_data.append(T_TYPE_Settlement);
    tx_data.append(LEN_Payment_Indicator, ZERO);
    tx_data.append(LEN_Host_ID, ZERO);
    tx_data.append(LEN_Receipt_No, ZERO);
    tx_data.append(LEN_Card_No, ZERO);
    tx_data.append(LEN_Card_Expire_Date, ZERO);
    tx_data.append(LEN_Trans_Amount, ZERO);
    tx_data.append(LEN_Trans_Date, ZERO);
    tx_data.append(LEN_Trans_Time, ZERO);
    tx_data.append(LEN_Approval_No, ZERO);
    tx_data.append(LEN_Wave_Card_ID, ZERO);
    tx_data.append(LEN_ECR_Response_Code, ZERO);
    tx_data.append(LEN_Merchant_ID, ZERO);
    tx_data.append(LEN_Terminal_ID, ZERO);
    tx_data.append(LEN_Exp_Amount, ZERO);
    tx_data.append(LEN_Store_ID, ZERO);
    tx_data.append(LEN_Installment_Indicator, ZERO);
    tx_data.append(LEN_RDM_Amount, ZERO);
    tx_data.append(LEN_RDM_Point, ZERO);
    tx_data.append(LEN_Balance_Point, ZERO);
    tx_data.append(LEN_Redeem_Amount, ZERO);
    tx_data.append(LEN_Installment_Period, ZERO);
    tx_data.append(LEN_Down_Amount, ZERO);
    tx_data.append(LEN_Installment_Amount, ZERO);
    tx_data.append(LEN_Installment_Fee, ZERO);
    tx_data.append(LEN_Card_Type, ZERO);
    tx_data.append(LEN_Batch_No, ZERO);
    tx_data.append(LEN_Start_Trans_Type, ZERO);
    tx_data.append(LEN_MP_Flag, ZERO);
    tx_data.append(LEN_SP_ISSUER_ID, ZERO);
    tx_data.append(LEN_SP_Origin_Return, ZERO);
    tx_data.append(LEN_SP_Origin_RRN, ZERO);
    tx_data.append(LEN_Pay_Item, ZERO);
    tx_data.append(LEN_Card_No_Hash, ZERO);
    tx_data.append(LEN_MP_Response_Code, ZERO);
    tx_data.append(LEN_ASM_Award_Flag, ZERO);
    tx_data.append(LEN_MCP_Indicator, ZERO);
    tx_data.append(LEN_Bank_No, ZERO);
    tx_data.append(LEN_Reserved, ZERO);
    tx_data.append(LEN_Happy_Go_Data, ZERO);
    tx_data.append(ETX);

    // calculate check sum
    char check_sum = 0x00;
    for (int i = 1; i < (LEN_Total_Package - 1); i++) {
        check_sum ^= tx_data[i];
    }
    tx_data.append(check_sum);

    // check the length of pageage
    if (tx_data.size() != LEN_Total_Package) {
        qDebug() << "[EDC] the length of TX data is illegal" << tx_data.size();
        return false;
    }

    qDebug() << "[EDC] TX data:" << tx_data;

    // write data
    this->write(tx_data);
    this->waitForBytesWritten(5000);

    // step 1. wait for ack
    if (this->waitForReadyRead(5000)) {
        rx_data = this->readAll();

        while (this->waitForReadyRead(20) && rx_data.length() < 2) {
            QByteArray possible_lost = this->readAll();
            rx_data += possible_lost;
        }
    }

    qDebug() << "[EDC] RX data:" << rx_data;

    // step 2. start timer to wait for result
    state_ = WAIT_SETTLEMENT;
    tmr_wait_receive_->start();
    return true;
}

bool NcccEDC::reboot()
{
    qDebug() << "[EDC] reboot start...";

    QByteArray tx_data;
    QByteArray rx_data;

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[EDC] device can't be opened";
        return false;
    }

    // create tx data
    tx_data.append(STX);
    tx_data.append('I');
    tx_data.append(LEN_ECR_Version, ZERO);
    tx_data.append(LEN_Trans_Type_Indicator, ZERO);
    tx_data.append(T_TYPE_Reboot);
    tx_data.append(LEN_Payment_Indicator, ZERO);
    tx_data.append(LEN_Host_ID, ZERO);
    tx_data.append(LEN_Receipt_No, ZERO);
    tx_data.append(LEN_Card_No, ZERO);
    tx_data.append(LEN_Card_Expire_Date, ZERO);
    tx_data.append(LEN_Trans_Amount, ZERO);
    tx_data.append(LEN_Trans_Date, ZERO);
    tx_data.append(LEN_Trans_Time, ZERO);
    tx_data.append(LEN_Approval_No, ZERO);
    tx_data.append(LEN_Wave_Card_ID, ZERO);
    tx_data.append(LEN_ECR_Response_Code, ZERO);
    tx_data.append(LEN_Merchant_ID, ZERO);
    tx_data.append(LEN_Terminal_ID, ZERO);
    tx_data.append(LEN_Exp_Amount, ZERO);
    tx_data.append(LEN_Store_ID, ZERO);
    tx_data.append(LEN_Installment_Indicator, ZERO);
    tx_data.append(LEN_RDM_Amount, ZERO);
    tx_data.append(LEN_RDM_Point, ZERO);
    tx_data.append(LEN_Balance_Point, ZERO);
    tx_data.append(LEN_Redeem_Amount, ZERO);
    tx_data.append(LEN_Installment_Period, ZERO);
    tx_data.append(LEN_Down_Amount, ZERO);
    tx_data.append(LEN_Installment_Amount, ZERO);
    tx_data.append(LEN_Installment_Fee, ZERO);
    tx_data.append(LEN_Card_Type, ZERO);
    tx_data.append(LEN_Batch_No, ZERO);
    tx_data.append(LEN_Start_Trans_Type, ZERO);
    tx_data.append(LEN_MP_Flag, ZERO);
    tx_data.append(LEN_SP_ISSUER_ID, ZERO);
    tx_data.append(LEN_SP_Origin_Return, ZERO);
    tx_data.append(LEN_SP_Origin_RRN, ZERO);
    tx_data.append(LEN_Pay_Item, ZERO);
    tx_data.append(LEN_Card_No_Hash, ZERO);
    tx_data.append(LEN_MP_Response_Code, ZERO);
    tx_data.append(LEN_ASM_Award_Flag, ZERO);
    tx_data.append(LEN_MCP_Indicator, ZERO);
    tx_data.append(LEN_Bank_No, ZERO);
    tx_data.append(LEN_Reserved, ZERO);
    tx_data.append(LEN_Happy_Go_Data, ZERO);
    tx_data.append(ETX);

    // calculate check sum
    char check_sum = 0x00;
    for (int i = 1; i < (LEN_Total_Package - 1); i++) {
        check_sum ^= tx_data[i];
    }
    tx_data.append(check_sum);

    // check the length of pageage
    if (tx_data.size() != LEN_Total_Package) {
        qDebug() << "[EDC] the length of TX data is illegal" << tx_data.size();
        return false;
    }

    qDebug() << "[EDC] TX data:" << tx_data;

    // write data
    this->write(tx_data);
    this->waitForBytesWritten(5000);

    // sleep
    QThread::sleep(30);

    qDebug() << "[EDC] reboot success";
    return true;
}

void NcccEDC::getTransactionInfos(QMap<QString, QByteArray> *infos)
{
    if (card_type_ == C_TYPE_EasyCard ||
        card_type_ == C_TYPE_iPass ||
        card_type_ == C_TYPE_iCash ||
        card_type_ == C_TYPE_HappyCash) {
        infos->insert("payment_method",  card_type_.toUtf8());
    }
    else {
        infos->insert("payment_method",  QString("15").toUtf8());
    }
    infos->insert("transDate",       trans_date_.toUtf8());
    infos->insert("transTime",       trans_time_.toUtf8());
    infos->insert("transAmount",     trans_amount_.toUtf8());
    infos->insert("ecrVersion",      ecr_version_.toUtf8());
    infos->insert("transType",       trans_type_.toUtf8());
    infos->insert("transID",         trans_id_.toUtf8());
    infos->insert("hostID",          host_id_.toUtf8());
    infos->insert("receiptNO",       receipt_no_.toUtf8());
    infos->insert("cardNO",          card_no_.toUtf8());
    infos->insert("approvalNo",      approval_no_.toUtf8());
    infos->insert("ecrresponseCode", response_code_.toUtf8());
    infos->insert("merchantID",      merchant_id_.toUtf8());
    infos->insert("terminalID",      terminal_id_.toUtf8());
    infos->insert("cardType",        card_type_.toUtf8());
    infos->insert("batchNO",         batch_no_.toUtf8());
}

void NcccEDC::getSettlementInfos(QMap<QString, QByteArray> *infos)
{
    infos->insert("transType",       trans_type_.toUtf8());
    infos->insert("transDate",       trans_date_.toUtf8());
    infos->insert("transTime",       trans_time_.toUtf8());
    infos->insert("ecrresponseCode", response_code_.toUtf8());
    infos->insert("merchantID",      merchant_id_.toUtf8());
    infos->insert("terminalID",      terminal_id_.toUtf8());

    infos->insert("ccCount",    creditcard_count_.toUtf8());
    infos->insert("ccAmount",   creditcard_amount_.toUtf8());
    infos->insert("eccCount",   easycard_count_.toUtf8());
    infos->insert("eccAmount",  easycard_amount_.toUtf8());
    infos->insert("ipCount",    ipass_count_.toUtf8());
    infos->insert("ipAmount",   ipass_amount_.toUtf8());
    infos->insert("icCount",    icash_count_.toUtf8());
    infos->insert("icAmount",   icash_amount_.toUtf8());
    infos->insert("hcCount",    happycash_count_.toUtf8());
    infos->insert("hcAmount",   happycash_amount_.toUtf8());
}

void NcccEDC::receiveTimeout()
{
    QString err_message = "[EDC] Timeout: ";

    err_message.append(this->readAll());
    err_message.append(QDateTime::currentDateTime().toString(" hh:mm:ss:zzz"));

    if (state_ == WAIT_TRANSACATION)
        emit executeTransactionResponse(false);
    else if (state_ == WAIT_SETTLEMENT)
        emit executeTransactionResponse(false);

    state_ = ERROR;
    this->clear();
    this->close();
}

void NcccEDC::rxDataReady()
{
    // wait for the all data ready
    if (this->bytesAvailable() < LEN_Total_Package) {
        return;
    }

    // stop timer
    tmr_wait_receive_->stop();

    QByteArray tx_data;
    QByteArray rx_data = this->readAll();

    qDebug() << "[EDC] RX data:" << rx_data;

    if (state_ == WAIT_TRANSACATION) {

        // write data
        tx_data.append(ACK);
        tx_data.append(ACK);
        this->write(tx_data);
        this->waitForBytesWritten(5000);

        // parse rx data
        ecr_version_ = rx_data.mid(2,6);
        trans_type_ = rx_data.mid(9,2);
        trans_id_ = rx_data.mid(11,1);
        host_id_ = rx_data.mid(12,2);
        receipt_no_ = rx_data.mid(14,6);
        card_no_ = rx_data.mid(20,19);
        trans_amount_ = rx_data.mid(43,10);
        trans_date_ = rx_data.mid(55,6);
        trans_time_ = rx_data.mid(61,6);
        approval_no_ = rx_data.mid(67,9);
        response_code_ = rx_data.mid(77,4);
        merchant_id_ = rx_data.mid(81,15);
        terminal_id_ = rx_data.mid(96,8);
        card_type_ = rx_data.mid(213,2);
        batch_no_ = rx_data.mid(215,6);

        // check transaction response
        if(response_code_ != RCODE_Approval) {
            qDebug() << "[EDC] transaction failed" << response_code_;
            emit executeTransactionResponse(false);
        }
        else {
            qDebug() << "[EDC] transaction success";
            emit executeTransactionResponse(true);
        }
    }
    else if (state_ == WAIT_SETTLEMENT) {

        // write data
        tx_data.append(ACK);
        tx_data.append(ACK);
        this->write(tx_data);
        this->waitForBytesWritten(5000);

        // parse rx data
        ecr_version_ = rx_data.mid(2,6);
        trans_type_ = rx_data.mid(9,2);
        trans_id_ = rx_data.mid(11,1);
        host_id_ = rx_data.mid(12,2);
        creditcard_count_ = rx_data.mid(39,4);
        creditcard_amount_ = rx_data.mid(43,10);
        trans_date_ = rx_data.mid(55,6);
        trans_time_ = rx_data.mid(61,6);
        response_code_ = rx_data.mid(77,4);
        merchant_id_ = rx_data.mid(81,15);
        terminal_id_ = rx_data.mid(96,8);
        easycard_count_ = rx_data.mid(104,3);
        easycard_amount_ = rx_data.mid(107,10);
        ipass_count_ = rx_data.mid(117,3);
        ipass_amount_ = rx_data.mid(120,10);
        icash_count_ = rx_data.mid(130,3);
        icash_amount_ = rx_data.mid(133,10);
        happycash_count_ = rx_data.mid(143,3);
        happycash_amount_ = rx_data.mid(146,10);

        // check transaction response
        if(response_code_ != RCODE_Approval) {
            qDebug() << "[EDC] settlement failed" << response_code_;
            emit executeSettlementResponse(false);
        }
        else {
            qDebug() << "[EDC] settlement success";
            emit executeSettlementResponse(true);
        }
    }

    // reset state
    state_ = IDLE;
    this->clear();
    this->close();
}

bool NcccEDC::isOpened()
{
    if (this->isOpen() == false) {
        this->open(QIODevice::ReadWrite);
    }
    return this->isOpen();
}
