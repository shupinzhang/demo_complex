#ifndef NCCC_EDC_H
#define NCCC_EDC_H

#include <QString>
#include <QSerialPort>

#define STX     0x02
#define ETX     0x03
#define ACK     0x06
#define ZERO    0x20

// define the length of each data for normal transaction
#define LEN_ECR_Indicator           1   // 1    // 'I', 'E'
#define LEN_ECR_Version             6   // 2
#define LEN_Trans_Type_Indicator    1   // 8    // 'S' or reserved for settlement
#define LEN_Trans_Type              2   // 9    // "01":Normal, "02":Return, "30":Cancel, "50":Self-checkout, "99":Reboot
#define LEN_Payment_Indicator       1   // 11   // 'C':CUP, 'N':CreditCard, 'S':SmartPay, 'E':ElectonicCard
#define LEN_Host_ID                 2   // 12
#define LEN_Receipt_No              6   // 14   // 14~19 reserved for settlement
#define LEN_Card_No                19   // 20   // 20~38 reserved for settlement
#define LEN_Card_Expire_Date        4   // 39   // the total count of credic-card for settlement
#define LEN_Trans_Amount           12   // 43   // "012345678900" include two decimal places
                                                // the total amount of credic-card for settlement
#define LEN_Trans_Date              6   // 55   // "yyMMdd"
#define LEN_Trans_Time              6   // 61   // "hhmmss"
#define LEN_Approval_No             9   // 67   // 67~75 reserved for settlement
#define LEN_Wave_Card_ID            1   // 76   // 'V','M','J','C','A','Z','P','G','H'
#define LEN_ECR_Response_Code       4   // 77   // "0000":Approval, "0001":Error, ...
#define LEN_Merchant_ID            15   // 81
#define LEN_Terminal_ID             8   // 96
#define LEN_Exp_Amount             12   // 104  // 104~106 the total count of easy-card for settlement
                                                // 107~116 the total amount of easy-card for settlement
                                                // 117~119 the total count of ipass for settlement
                                                // 120~129 the total amount of ipass for settlement
                                                // 130~132 the total count of icash for settlement
                                                // 133~142 the total amount of icash for settlement
                                                // 143~145 the total count of happy-cash for settlement
                                                // 146~155 the total amount of happy-cash for settlement
#define LEN_Store_ID               18   // 116
#define LEN_Installment_Indicator   1   // 134
#define LEN_RDM_Amount             12   // 135
#define LEN_RDM_Point               8   // 147
#define LEN_Balance_Point           8   // 155
#define LEN_Redeem_Amount          12   // 163
#define LEN_Installment_Period      2   // 175
#define LEN_Down_Amount            12   // 177
#define LEN_Installment_Amount     12   // 189
#define LEN_Installment_Fee        12   // 201
#define LEN_Card_Type               2   // 213
#define LEN_Batch_No                6   // 215
#define LEN_Start_Trans_Type        2   // 221
#define LEN_MP_Flag                 1   // 223
#define LEN_SP_ISSUER_ID            8   // 224
#define LEN_SP_Origin_Return        8   // 232
#define LEN_SP_Origin_RRN          12   // 240
#define LEN_Pay_Item                5   // 252
#define LEN_Card_No_Hash           50   // 257
#define LEN_MP_Response_Code        6   // 307
#define LEN_ASM_Award_Flag          1   // 313
#define LEN_MCP_Indicator           1   // 314
#define LEN_Bank_No                 3   // 315
#define LEN_Reserved                5   // 318
#define LEN_Happy_Go_Data          78   // 323
#define LEN_Total_Package         403

// define the transaction type
#define T_TYPE_Normal               "01"
#define T_TYPE_Settlement           "50"
#define T_TYPE_Reboot               "99"

// define the card type
#define C_TYPE_TWIN                 "01"
#define C_TYPE_VISA                 "02"
#define C_TYPE_MASTER               "03"
#define C_TYPE_JCB                  "04"
#define C_TYPE_AE                   "05"
#define C_TYPE_CUP                  "06"
#define C_TYPE_DISCOVER             "07"
#define C_TYPE_SmartPay             "08"
#define C_TYPE_EasyCard             "11"
#define C_TYPE_iPass                "12"
#define C_TYPE_iCash                "13"
#define C_TYPE_HappyCash            "14"

// define the response code of ECR
#define RCODE_Approval              "0000"  // Transcation success
#define RCODE_Error                 "0001"  // Transcation failed
#define RCODE_Call_Bank             "0002"
#define RCODE_Timeout               "0003"
#define RCODE_Operation_Error       "0004"
#define RCODE_Communication_Error   "0005"
#define RCODE_User_Terminate        "0006"

class QTimer;

class NcccEDC : public QSerialPort
{
    Q_OBJECT

    enum State {
        ERROR              = -1,
        IDLE                = 0,
        WAIT_TRANSACATION,
        WAIT_SETTLEMENT
    };

public:
    enum PaymentType {
        Undefined  = -1,
        CreditCard  = 0,
        iPass       = 1,
        EasyCard    = 2,
        iCash       = 3,
        HappyCash   = 4
    };

public:
    NcccEDC(QObject *parent = nullptr);
    ~NcccEDC();

    bool executeTransaction(PaymentType p_type, int amount);
    bool executeSettlement();
    bool reboot();

    void getTransactionInfos(QMap<QString, QByteArray> *infos);
    void getSettlementInfos(QMap<QString, QByteArray> *infos);

Q_SIGNALS:
    void executeTransactionResponse(bool result);
    void executeSettlementResponse(bool result);

private slots:
    void receiveTimeout();
    void rxDataReady();

private:
    bool isOpened();

private:
    State state_;
    QTimer *tmr_wait_receive_;

    QString ecr_version_;
    QString trans_type_;
    QString trans_id_;
    QString host_id_;
    QString receipt_no_;
    QString card_no_;
    QString trans_amount_;
    QString trans_date_;
    QString trans_time_;
    QString approval_no_;
    QString response_code_;
    QString merchant_id_;
    QString terminal_id_;
    QString card_type_;
    QString batch_no_;
    QString esvc_amount_;

    QString creditcard_count_;
    QString creditcard_amount_;
    QString easycard_count_;
    QString easycard_amount_;
    QString ipass_count_;
    QString ipass_amount_;
    QString icash_count_;
    QString icash_amount_;
    QString happycash_count_;
    QString happycash_amount_;
};

#endif // NCCC_EDC_H
