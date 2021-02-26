#ifndef NUM_INPUT_H
#define NUM_INPUT_H

#include <QWidget>

class QAbstractButton;

namespace Ui {
class NumInput;
}

class NumInput : public QWidget
{
    Q_OBJECT

public:
    explicit NumInput(QWidget *parent = nullptr);
    ~NumInput();

signals:
    void key_num_clicked(QString number);
    void key_clear_clicked();
    void key_backspace_clicked();

private slots:
    void on_pbtn_num_0_clicked();
    void on_pbtn_num_1_clicked();
    void on_pbtn_num_2_clicked();
    void on_pbtn_num_3_clicked();
    void on_pbtn_num_4_clicked();
    void on_pbtn_num_5_clicked();
    void on_pbtn_num_6_clicked();
    void on_pbtn_num_7_clicked();
    void on_pbtn_num_8_clicked();
    void on_pbtn_num_9_clicked();

private:
    void setButtonEffect(QAbstractButton *button);

private:
    Ui::NumInput *ui;
};

#endif // NUM_INPUT_H
