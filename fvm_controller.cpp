#include "fvm_controller.h"

#include <QDebug>
#include <QDateTime>

FVMController::FVMController(QObject *parent)
    : QSerialPort(parent)
{
    this->setBaudRate(QSerialPort::Baud9600);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    // initialize single shot timer
    tmr_wait_receive_ = new QTimer(this);
    tmr_wait_receive_->setInterval(3000);
    tmr_wait_receive_->setSingleShot(true);

    // initialize signals and slots
    connect(tmr_wait_receive_, SIGNAL(timeout()), this, SLOT(receiveTimeout()));
    connect(this, SIGNAL(readyRead()), this, SLOT(rxDataReady()));

    // initialize control state
    state_ = Normal;
}

FVMController::~FVMController()
{

}

bool FVMController::queryState(int station)
{
    qDebug() << "[FVMC] query machine state...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Query_State);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Query_State;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    // emit response
    char state = rx_data.at(3);
    emit queryStateResponse(station, (State)state);

    return true;
}

bool FVMController::openDoor(int station, int numbering, bool all)
{
    qDebug() << "[FVMC] open door" << numbering << "...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;

    if (all) {
        tx_data.append((char)FVM_STX);
        tx_data.append((char)station);
        tx_data.append((char)CMD_Open_Door);
        tx_data.append((char)0x30);
        tx_data.append((char)0x30);
        tx_data.append((char)FVM_ETX);
    }
    else {
        tx_data.append((char)FVM_STX);
        tx_data.append((char)station);
        tx_data.append((char)CMD_Open_Door);
        tx_data.append((char)0x01);
        tx_data.append((char)(numbering - 1));
        tx_data.append((char)FVM_ETX);
    }

    rx_expected_len_ = RX_LEN_Open_Door;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    // emit response
    char result = rx_data.at(3);
    if (all)
        emit openDoorResponse(station, -1, (result == 0x30));
    else
        emit openDoorResponse(station, numbering, (result == 0x01));

    return true;
}

bool FVMController::readDoorStatus(int station)
{
    qDebug() << "[FVMC] read door status...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Read_Door_Status);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Read_Door_Status;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    // emit response
    //char main_door_status = rx_data.at(9);

    QList<bool> door_status = door_status_map_.value(station);
    door_status.clear();
    for(int i = 0; i < 48; i += 8) {
        char status = rx_data.at(3 + i / 8);
        door_status.append(!(status & 0x01));
        door_status.append(!(status & 0x02));
        door_status.append(!(status & 0x04));
        door_status.append(!(status & 0x08));
        door_status.append(!(status & 0x10));
        door_status.append(!(status & 0x20));
        door_status.append(!(status & 0x40));
        door_status.append(!(status & 0x80));
    }
    door_status_map_.insert(station, door_status);
    emit readDoorStatusResponse(station, true);

    return true;
}

bool FVMController::setUVSwitch(int station, bool on_off)
{
    qDebug() << "[FVMC] set UV switch...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Set_UV_OnOff);
    tx_data.append((on_off)? (char)0x01 : (char)0x00);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Set_UV_OnOff;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    char result = rx_data.at(3);
    emit setUVSwitchResponse(station, (result == 0x01));

    return true;
}

bool FVMController::setHeaterSwitch(int station, bool on_off)
{
    qDebug() << "[FVMC] set heater switch...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Set_Heater_OnOff);
    tx_data.append((on_off)? (char)0x01 : (char)0x00);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Set_Heater_OnOff;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    char result = rx_data.at(3);
    emit setUVSwitchResponse(station, (result == 0x01));

    return true;
}

bool FVMController::writeHeaterSettings(int station, SettingMode mode, QString temperature)
{
    qDebug() << "[FVMC] write heater settings...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    int point = temperature.indexOf('.');
    int part_int = temperature.left(point).toInt();
    int part_dec = temperature.mid(point + 1).toInt();

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Write_Heater_Setting);
    if (mode == HeaterTemp)
        tx_data.append((char)0x01);
    else if (mode == DeltaTemp)
        tx_data.append((char)0x02);
    tx_data.append((char)part_int);
    tx_data.append((char)part_dec);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Write_Heater_Setting;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    char result = rx_data.at(3);
    emit writeHeaterSettingsResponse(station, (result == 0x01));

    return true;
}

bool FVMController::readHeaterSettings(int station, SettingMode mode)
{
    qDebug() << "[FVMC] read heater settings...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Read_Heater_Setting);
    if (mode == HeaterTemp)
        tx_data.append((char)0x01);
    else if (mode == DeltaTemp)
        tx_data.append((char)0x02);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Read_Heater_Setting;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    char result = rx_data.at(3);
    char part_int = rx_data.at(4);
    char part_dec = rx_data.at(5);
    emit readHeaterSettingsResponse(station,
                                    (result == 0x01),
                                    (int)mode,
                                    QString("%1.%2").arg((int)part_int).arg((int)part_dec));

    return true;
}

bool FVMController::readHeaterStatus(int station)
{
    qDebug() << "[FVMC] read heater temperature...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[FVMC] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    QByteArray rx_data;
    tx_data.append((char)FVM_STX);
    tx_data.append((char)station);
    tx_data.append((char)CMD_Read_Heater_Status);
    tx_data.append((char)0x01);
    tx_data.append((char)FVM_ETX);

    // set expected length of rx data
    rx_expected_len_ = RX_LEN_Read_Heater_Status;

    // write data and wait for ready read
    if (writeAndWaitForReadyRead(tx_data, rx_expected_len_, &rx_data) == false) {
        return false;
    }

    char result = rx_data.at(3);
    char part_int = rx_data.at(4);
    char part_dec = rx_data.at(5);
    char status = rx_data.at(6);
    emit readHeaterStatusResponse(station,
                                  (result == 0x01),
                                  (status == 0x01),
                                  QString("%1.%2").arg((int)part_int).arg((int)part_dec));

    return true;
}

QList<bool> FVMController::getDoorStatus(int station)
{
    return door_status_map_.value(station);
}

void FVMController::receiveTimeout()
{
    QString err_message = "[FVMC] Timeout: ";

    err_message.append(this->readAll());
    err_message.append(QDateTime::currentDateTime().toString(" hh:mm:ss:zzz"));

    this->clear();
    emit error(err_message);
}

void FVMController::rxDataReady()
{
    // wait until the expected length is ready
    if (this->bytesAvailable() != rx_expected_len_) {
        return;
    }

    // stop reading timeout timer
    tmr_wait_receive_->stop();

    // read all data
    QByteArray rx_data = this->readAll();
    qDebug() << "[FVMC] RX data:" << rx_data << QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
}

bool FVMController::isOpened()
{
    if (this->isOpen() == false) {
        this->open(QIODevice::ReadWrite);
    }
    return this->isOpen();
}

bool FVMController::writeAndWaitForReadyRead(QByteArray tx_data, int expected_len, QByteArray *rx_data)
{
    // disconnect temporarily for blocking waiting
    disconnect(this, SIGNAL(readyRead()), this, SLOT(rxDataReady()));

    qDebug() << "[FVMC] TX data:" << tx_data << QDateTime::currentDateTime().toString("hh:mm:ss:zzz");

    QByteArray rx_data_tmp;

    // write data
    this->clear();
    this->write(tx_data);
    this->waitForBytesWritten(3000);

    // wait and read all data
    this->waitForReadyRead(3000);
    rx_data_tmp.append(this->readAll());

    int count = 0;
    while (rx_data_tmp.length() < expected_len && count < 30) {
        this->waitForReadyRead(1000);
        rx_data_tmp.append(this->readAll());
        count++;
    }

    qDebug() << "[FVMC] RX data:" << rx_data_tmp << QDateTime::currentDateTime().toString("hh:mm:ss:zzz");

    // recover signal and slot connection
    connect(this, SIGNAL(readyRead()), this, SLOT(rxDataReady()));

    // check data is valid
    if (rx_data_tmp.length() != expected_len) {
        qDebug() << "[FVMC] RX data's length is invalid" << rx_data_tmp.length();
        return false;
    }

    rx_data->append(rx_data_tmp);
    return true;
}
