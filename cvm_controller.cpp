#include "cvm_controller.h"

#include <QDebug>

CVMController::CVMController(QObject *parent)
    : QSerialPort(parent)
{
    this->setBaudRate(QSerialPort::Baud9600);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);

    connect(this, SIGNAL(readyRead()), this, SLOT(rxDataReady()));
}

CVMController::~CVMController()
{

}

bool CVMController::openDoor(int station, int numbering, bool all)
{
    Q_UNUSED(station);

    qDebug() << "[Locker] open door" << numbering << "...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[Locker] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;

    if (all) {
        tx_data.append((char)CVM_STX);
        tx_data.append((char)0x05);
        tx_data.append((char)CVM_CMD_Open_Door);
        tx_data.append((char)0x11);
        tx_data.append((char)0x00);
        tx_data.append((char)0x01);
        tx_data.append((char)0x00);
        tx_data.append((char)0x15);
        tx_data.append((char)CVM_ETX);
    }
    else {
        tx_data.append((char)CVM_STX);
        tx_data.append((char)0x05);
        tx_data.append((char)CVM_CMD_Open_Door);
        tx_data.append((char)0x11);
        tx_data.append((char)0x00);
        tx_data.append((char)0x01);
        tx_data.append((char)numbering);
        char checksum = 0x00;
        for (int i = 1; i <= 6; i++) {
            checksum ^= tx_data[i];
        }
        tx_data.append(checksum);
        tx_data.append((char)CVM_ETX);
    }

    rx_expected_len_ = 11;

    // write data
    this->clear();
    this->write(tx_data);
    this->waitForBytesWritten(3000);

    return true;
}

bool CVMController::readDoorStatus(int station)
{
    Q_UNUSED(station);

    qDebug() << "[Locker] read door status...";

    // check serial port is opened
    if (isOpened() == false) {
        qDebug() << "[Locker] device open failed";
        return false;
    }

    // create tx data
    QByteArray tx_data;
    tx_data.append((char)CVM_STX);
    tx_data.append((char)0x05);
    tx_data.append((char)CVM_CMD_Read_Door);
    tx_data.append((char)0x11);
    tx_data.append((char)0x00);
    tx_data.append((char)0x01);
    tx_data.append((char)0x01);
    tx_data.append((char)0x15);
    tx_data.append((char)CVM_ETX);

    rx_expected_len_ = 11;

    // write data
    this->clear();
    this->write(tx_data);
    this->waitForBytesWritten(3000);

    return true;
}

void CVMController::rxDataReady()
{
    // wait until the expected length is ready
    if (this->bytesAvailable() != rx_expected_len_) {
        return;
    }

    // read all data
    QByteArray rx_data = this->readAll();
    qDebug() << "[Locker] RX data:" << rx_data;

    door_status_list_.clear();
    for(int i = 0; i < 12; i += 4) {
        char status = rx_data.at(6 + i / 4);
        door_status_list_.append((status & 0x01));
        door_status_list_.append((status & 0x04));
        door_status_list_.append((status & 0x10));
        door_status_list_.append((status & 0x40));
    }

    emit readDoorStatusResponse();
}

bool CVMController::isOpened()
{
    if (this->isOpen() == false) {
        this->open(QIODevice::ReadWrite);
    }
    return this->isOpen();
}
