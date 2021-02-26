#include "num_input.h"
#include "ui_num_input.h"

#include <QGraphicsDropShadowEffect>

NumInput::NumInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NumInput)
{
    ui->setupUi(this);

    setButtonEffect(ui->pbtn_num_0);
    setButtonEffect(ui->pbtn_num_1);
    setButtonEffect(ui->pbtn_num_2);
    setButtonEffect(ui->pbtn_num_3);
    setButtonEffect(ui->pbtn_num_4);
    setButtonEffect(ui->pbtn_num_5);
    setButtonEffect(ui->pbtn_num_6);
    setButtonEffect(ui->pbtn_num_7);
    setButtonEffect(ui->pbtn_num_8);
    setButtonEffect(ui->pbtn_num_9);
    setButtonEffect(ui->pbtn_clear);
    setButtonEffect(ui->pbtn_backspace);

    connect(ui->pbtn_clear, SIGNAL(clicked()), this, SIGNAL(key_clear_clicked()));
    connect(ui->pbtn_backspace, SIGNAL(clicked()), this, SIGNAL(key_backspace_clicked()));
}

NumInput::~NumInput()
{
    delete ui;
}

void NumInput::setButtonEffect(QAbstractButton *button)
{
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setColor(QColor(0,0,0,59));
    shadow_effect->setBlurRadius(10);
    shadow_effect->setOffset(0,5);
    button->setGraphicsEffect(shadow_effect);

    QString default_effect = "";
    QString pressed_effect = "";

    if (button->objectName() == "pbtn_clear" ||
        button->objectName() == "pbtn_backspace") {
        default_effect.append("{color: rgb(119, 119, 119);");
        default_effect.append(" background-color: qlineargradient(");
        default_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        default_effect.append(" stop:0.2 rgba(236, 236, 236, 255),");
        default_effect.append(" stop:1.0 rgba(255, 255, 255, 255));");
        default_effect.append(" border: 1px solid lightgray; border-radius:10px;}");
        pressed_effect.append("{color: rgb(119, 119, 119);");
        pressed_effect.append(" background-color: qlineargradient(");
        pressed_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        pressed_effect.append(" stop:0.2 rgba(226, 226, 226, 255),");
        pressed_effect.append(" stop:1.0 rgba(245, 245, 245, 255));");
        pressed_effect.append(" border: 1px solid lightgray; border-radius:10px;}");
    }
    else {
        default_effect.append("{color: rgb(255, 255, 255);");
        default_effect.append(" background-color: qlineargradient(");
        default_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        default_effect.append(" stop:0 rgba(69, 99, 106, 255),");
        default_effect.append(" stop:1.0 rgba(141, 167, 184, 255));");
        default_effect.append(" border-radius:10px;}");
        pressed_effect.append("{color: rgb(255, 255, 255);");
        pressed_effect.append(" background-color: qlineargradient(");
        pressed_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        pressed_effect.append(" stop:0 rgba(63, 91, 97, 255),");
        pressed_effect.append(" stop:1.0 rgba(129, 153, 168, 255));");
        pressed_effect.append(" border-radius:10px;}");
    }
    button->setStyleSheet("QPushButton" + default_effect + "QPushButton:pressed" + pressed_effect);
}

void NumInput::on_pbtn_num_0_clicked()
{
    emit key_num_clicked("0");
}

void NumInput::on_pbtn_num_1_clicked()
{
    emit key_num_clicked("1");
}

void NumInput::on_pbtn_num_2_clicked()
{
    emit key_num_clicked("2");
}

void NumInput::on_pbtn_num_3_clicked()
{
    emit key_num_clicked("3");
}

void NumInput::on_pbtn_num_4_clicked()
{
    emit key_num_clicked("4");
}

void NumInput::on_pbtn_num_5_clicked()
{
    emit key_num_clicked("5");
}

void NumInput::on_pbtn_num_6_clicked()
{
    emit key_num_clicked("6");
}

void NumInput::on_pbtn_num_7_clicked()
{
    emit key_num_clicked("7");
}

void NumInput::on_pbtn_num_8_clicked()
{
    emit key_num_clicked("8");
}

void NumInput::on_pbtn_num_9_clicked()
{
    emit key_num_clicked("9");
}
