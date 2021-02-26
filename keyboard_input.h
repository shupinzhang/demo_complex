#ifndef KEYBOARD_INPUT_H
#define KEYBOARD_INPUT_H

#include <QWidget>

namespace Ui {
class KeyboardInput;
}

class QAbstractButton;

class KeyboardInput : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardInput(QWidget *parent = nullptr);
    ~KeyboardInput();

signals:
    void key_char_clicked(QString character);
    void key_backspace_clicked();

private slots:
    void pbtn_char_clicked();
    void on_pbtn_case_toggled(bool checked);

private:
    void setButtonEffect(QAbstractButton *button);

private:
    Ui::KeyboardInput *ui;
};

#endif // KEYBOARD_INPUT_H
