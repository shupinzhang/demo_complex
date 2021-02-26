#include "printer_kp247.h"

#include <QTextCodec>

// print and feed commands
#define CMD_LF              0x0A    // print, feed, return
#define CMD_CR              0x0D    // return
#define CMD_ESC             0x1B40  // reset
#define CMD_ESC_j_n         0x1B4A  // print and feed n pts, 0 <= n <= 255
#define CMD_ESC_d_n         0x1B64  // print and feed n lines,  0 <= n <= 255
#define CMD_CUT_n           0x1D56  // feed and cut

// printer setting commands
#define CMD_ESC__n          0x1B21  // set the format of character, bit0: small
                                    //                              bit3: bold
                                    //                              bit4: double height
                                    //                              bit5: double width
                                    //                              bit7: underline
#define CMD_ESC_$_nL_nH     0x1B24  // set (nL + nH x256) pts after start position, 0 <= nL <= 255, 0 <= nH <= 255
#define CMD_ESC___n         0x1B2D  // set underline, 0: OFF, 1: ON(1pt), 2: ON(2pt)
#define CMD_ESC_2           0x1B32  // set 33 pts between lines
#define CMD_ESC_3_n         0x1B33  // set n pts between lines, 0 <= n(33) <= 255
#define CMD_ESC_V_n         0x1B56  // set rotation, 0: OFF, 1: ON
#define CMD_ESC_a_n         0x1B61  // set alignment, 0: left, 1: center, 2: right
#define CMD_GS__n           0x1D21  // set the format of character, bit0~3: height, bit4~7: width
#define CMD_GS_B_n          0x1D42  // set highlight, 0: OFF, 1: ON
#define CMD_GS_L_nL_nH      0x1D4C  // set (nL + nH x256) pts as left empty

// print image commands

// print table commands

// print barcode commands
#define CMD_GS_H_n          0x1D48  // set the location of HRI, 0: invisible, 1: upper, 2: lower, 3: upper and lower
#define CMD_GS_h_n          0x1D68  // set the height of barcode, 1 <= n(64) <= 255
#define CMD_GS_w_n          0x1D77  // set the width of barcode, 1 <= n(2) <= 6
#define CMD_GS_f_n          0x1D66  // set the font of HRI, n = 0(A), n = 1(B)
#define CMD_GS_k_m_n_k      0x1D6B  // print barcode, m: encode(4:CODE 39), n: length, k: length

// query status commands
#define CMD_DLE_EOT_n       0x1004  // query status of..., 1: printer, 2: offline, 3: error, 4: paper sensor

// print QRcode commands
#define CMD_QR_pL_pH_cn_fn_n    0x1D286b  // set the module type of QR Code
#define CMD_QR_v_r_nl_nH_n      0x1D6B61  // print QR Code

// print double QRcode commands
#define CMD_DQR_m_n_p1HL_l1HL_ecc1_v1_d1    0x1F51  // print double QR Code, m: numbers, n: size(1~8),
                                                    // p1H*256+p1L: the location of QR1
                                                    // l1H*256+l1L: the data length of QR1
                                                    // ecc1: the tolerance level
                                                    // v1: version of the symbol
                                                    // d1: the data of QR1
                                                    // p2H*256+p2L: the location of QR2
                                                    // l2H*256+l2L: the data length of QR2
                                                    // ....

PrinterKP247::PrinterKP247(QObject *parent)
    : QSerialPort(parent)
{
    // basic setting
    this->setBaudRate(QSerialPort::Baud9600);
    this->setDataBits(QSerialPort::Data8);
    this->setParity(QSerialPort::NoParity);
    this->setStopBits(QSerialPort::OneStop);
    this->setFlowControl(QSerialPort::NoFlowControl);
}

PrinterKP247::~PrinterKP247()
{
}

void PrinterKP247::setDefault()
{
    QByteArray tx_data;
    tx_data.append((char)(CMD_ESC >> 8));
    tx_data.append((char)(CMD_ESC & 0xFF));

    this->write(tx_data);
    this->waitForBytesWritten(100);
    this->clear();
}

void PrinterKP247::setTextFormat(bool small, bool bold, bool double_height, bool double_width, bool underline)
{
    QByteArray tx_data;
    tx_data.append((char)(CMD_ESC__n >> 8));
    tx_data.append((char)(CMD_ESC__n & 0xFF));

    char format = 0x00;
    if (small) {
        format |= (char)0x01;
    }
    if (bold) {
        format |= (char)0x08;
    }
    if (double_height) {
        format |= (char)0x10;
    }
    if (double_width) {
        format |= (char)0x20;
    }
    if (underline) {
        format |= (char)0x80;
    }
    tx_data.append(format);

    this->write(tx_data);
    this->waitForBytesWritten(100);
    this->clear();
}

void PrinterKP247::setTextAlignment(int alignment)
{
    QByteArray tx_data;
    tx_data.append((char)(CMD_ESC_a_n >> 8));
    tx_data.append((char)(CMD_ESC_a_n & 0xFF));
    tx_data.append((char)alignment);

    this->write(tx_data);
    this->waitForBytesWritten(100);
    this->clear();
}

void PrinterKP247::printText(QString text)
{
    QTextCodec *codec = QTextCodec::codecForName("Big5");

    QByteArray tx_data;
    tx_data.append(codec->fromUnicode(text));
    tx_data.append((char)CMD_LF);

    this->write(tx_data);
    this->waitForBytesWritten(500);
    this->clear();
}

void PrinterKP247::printBarcode(QString text, uint width, uint height, uint text_location)
{
    QByteArray tx_data;
    tx_data.append((char)(CMD_ESC_a_n >> 8));
    tx_data.append((char)(CMD_ESC_a_n & 0xFF));
    tx_data.append((char)0x00);
    tx_data.append((char)(CMD_ESC_$_nL_nH >> 8));
    tx_data.append((char)(CMD_ESC_$_nL_nH & 0xFF));
    tx_data.append((char)0x00);
    tx_data.append((char)0x00);
    tx_data.append((char)(CMD_GS_w_n >> 8));
    tx_data.append((char)(CMD_GS_w_n & 0xFF));
    tx_data.append((char)width);
    tx_data.append((char)(CMD_GS_h_n >> 8));
    tx_data.append((char)(CMD_GS_h_n & 0xFF));
    tx_data.append((char)height);
    tx_data.append((char)(CMD_GS_f_n >> 8));
    tx_data.append((char)(CMD_GS_f_n & 0xFF));
    tx_data.append((char)0x01); // B-type
    tx_data.append((char)(CMD_GS_H_n >> 8));
    tx_data.append((char)(CMD_GS_H_n & 0xFF));
    tx_data.append((char)text_location);
    tx_data.append((char)(CMD_GS_k_m_n_k >> 8));
    tx_data.append((char)(CMD_GS_k_m_n_k & 0xFF));
    tx_data.append((char)0x45); // CODE39
    tx_data.append((char)text.length());
    tx_data.append(text);

    this->write(tx_data);
    this->waitForBytesWritten(500);
    this->clear();
}

void PrinterKP247::printDoubleQRCode(QByteArray data_ba)
{
    uint left_QR_length = 0, right_QR_length = 0;
    bool right_QR_enabled = (data_ba.length() > 100);
    if (right_QR_enabled) {
        left_QR_length = 100;
        right_QR_length = 2 + (data_ba.length() - 100);
    }
    else {
        left_QR_length = data_ba.length();
        right_QR_length = 2;
    }

    QByteArray tx_data;
    tx_data.append((char)(CMD_DQR_m_n_p1HL_l1HL_ecc1_v1_d1 >> 8));
    tx_data.append((char)(CMD_DQR_m_n_p1HL_l1HL_ecc1_v1_d1 & 0xFF));
    tx_data.append((char)0x02);             // double
    tx_data.append((char)0x03);             // module size
    tx_data.append((char)0x00);             // x location of left QR code
    tx_data.append((char)0x20);             // y location of left QR code
    tx_data.append((char)0x00);             // data length of left QR code
    tx_data.append((char)left_QR_length);   // (0x00)*256 + (0x0A)
    tx_data.append((char)0x01);             // ecc, error correction level
    tx_data.append((char)0x06);             // version of the symbol
    tx_data.append((right_QR_enabled)? data_ba.left(100) : data_ba);
    tx_data.append((char)0x00);             // x location of right QR code
    tx_data.append((char)0xC0);             // y location of right QR code
    tx_data.append((char)0x00);             // data length of right QR code
    tx_data.append((char)right_QR_length);  // (0x00)*256 + (0x0A)
    tx_data.append((char)0x01);             // ecc, error correction level
    tx_data.append((char)0x06);             // version of the symbol
    tx_data.append((right_QR_enabled)? "**" + data_ba.mid(100) : "**");

    this->write(tx_data);
    this->waitForBytesWritten(500);
    this->clear();
}

void PrinterKP247::cut(int mode, int feed)
{
    QByteArray tx_data;
    tx_data.append((char)(CMD_CUT_n >> 8));
    tx_data.append((char)(CMD_CUT_n & 0xFF));

    if (mode == 66) {
        tx_data.append((char)(mode & 0xFF));
        tx_data.append((char)(feed & 0xFF));
    }
    else if (mode == 0 || mode == 1) {
        tx_data.append((char)mode);
    }

    this->write(tx_data);
    this->waitForBytesWritten(500);
    this->clear();
}

