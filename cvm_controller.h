#ifndef CVM_CONTROLLER_H
#define CVM_CONTROLLER_H

#include <QSerialPort>

#define CVM_STX     0x06
#define CVM_ETX     0x08

#define CVM_CMD_Open_Door       0x00
#define CVM_CMD_Read_Door       0x01

class CVMController : public QSerialPort
{
    Q_OBJECT

public:
    CVMController(QObject *parent = nullptr);
    ~CVMController();

    QList<bool> getDoorStatus(int station);

public slots:
    bool openDoor(int station, int numbering, bool all = false);
    bool readDoorStatus(int station);

Q_SIGNALS:
    void readDoorStatusResponse();

private slots:
    void rxDataReady();

private:
    bool isOpened();

private:
    int rx_expected_len_;
    QList<bool> door_status_list_;
};

#endif // CVM_CONTROLLER_H
