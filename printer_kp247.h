#ifndef PRINTER_KP247_H
#define PRINTER_KP247_H

#include <QSerialPort>

class PrinterKP247 : public QSerialPort
{
    Q_OBJECT

public:
    PrinterKP247(QObject *parent = nullptr);
    ~PrinterKP247();

    void setDefault();
    void setTextFormat(bool small = false,
                       bool bold = false,
                       bool double_height = false,
                       bool double_width = false,
                       bool underline = false);
    void setTextAlignment(int alignment);
    void printText(QString text);
    void printBarcode(QString text, uint width = 0x01, uint height = 0x28, uint text_location = 0x00);
    void printDoubleQRCode(QByteArray data_ba);
    void cut(int mode, int feed = 20);
};

#endif // PRINTER_KP247_H
