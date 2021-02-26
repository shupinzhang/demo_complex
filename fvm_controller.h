#ifndef FVM_CONTROLLER_H
#define FVM_CONTROLLER_H

#include <QSerialPort>
#include <QTimer>
#include <QMap>

#define FVM_STX     0xAA
#define FVM_ETX     0xCC

#define CMD_Query_State                 0x00
#define CMD_Open_Door                   0x01
#define CMD_Read_Door_Status            0x02
#define CMD_Set_UV_OnOff                0x03
#define CMD_Set_Heater_OnOff            0x04
#define CMD_Write_Heater_Setting        0x05
#define CMD_Read_Heater_Setting         0x06
#define CMD_Read_Heater_Status          0x07

#define RX_LEN_Query_State              5
#define RX_LEN_Open_Door                6
#define RX_LEN_Read_Door_Status        11   // STX(1) + EqID(1) + CmdID(1) + SmallStatus(6) + BigStatus(1) + ETX(1)
#define RX_LEN_Set_UV_OnOff             5
#define RX_LEN_Set_Heater_OnOff         5
#define RX_LEN_Write_Heater_Setting     5
#define RX_LEN_Read_Heater_Setting      7
#define RX_LEN_Read_Heater_Status       8

class FVMController : public QSerialPort
{
    Q_OBJECT

public:
    enum State {
        Normal      = 0x01,
        Error       = 0x02,
        Working     = 0x03
    };

    enum SettingMode {
        HeaterTemp  = 0x01,
        DeltaTemp   = 0x02
    };

public:
    FVMController(QObject *parent = nullptr);
    ~FVMController();

    QList<bool> getDoorStatus(int station);

public slots:
    bool queryState(int station);
    bool openDoor(int station, int numbering, bool all = false);
    bool readDoorStatus(int station);
    bool setUVSwitch(int station, bool on_off);
    bool setHeaterSwitch(int station, bool on_off);
    bool writeHeaterSettings(int station, SettingMode mode, QString temperature);
    bool readHeaterSettings(int station, SettingMode mode);
    bool readHeaterStatus(int station);

Q_SIGNALS:
    void error(QString err_msg);
    void queryStateResponse(int station, int state);
    void openDoorResponse(int station, int numbering, bool result);
    void readDoorStatusResponse(int station, bool result);
    void setUVSwitchResponse(int station, bool result);
    void setHeaterSwitchResponse(int station, bool result);
    void writeHeaterSettingsResponse(int station, bool result);
    void readHeaterSettingsResponse(int station, bool result, int mode, QString temperature);
    void readHeaterStatusResponse(int station, bool result, bool on_off, QString temperature);

private slots:
    void receiveTimeout();
    void rxDataReady();

private:
    bool isOpened();
    bool writeAndWaitForReadyRead(QByteArray tx_data, int expected_len, QByteArray *rx_data);

private:
    State state_;

    int rx_expected_len_;
    QTimer *tmr_wait_receive_;

    QMap<int, QList<bool>> door_status_map_;
};

#endif // FVM_CONTROLLER_H
