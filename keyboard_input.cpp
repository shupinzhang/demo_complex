#include "keyboard_input.h"
#include "ui_keyboard_input.h"

#include <QGraphicsDropShadowEffect>

KeyboardInput::KeyboardInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardInput)
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
    setButtonEffect(ui->pbtn_num_9);
    setButtonEffect(ui->pbtn_char_a);
    setButtonEffect(ui->pbtn_char_b);
    setButtonEffect(ui->pbtn_char_c);
    setButtonEffect(ui->pbtn_char_d);
    setButtonEffect(ui->pbtn_char_e);
    setButtonEffect(ui->pbtn_char_f);
    setButtonEffect(ui->pbtn_char_g);
    setButtonEffect(ui->pbtn_char_h);
    setButtonEffect(ui->pbtn_char_i);
    setButtonEffect(ui->pbtn_char_j);
    setButtonEffect(ui->pbtn_char_k);
    setButtonEffect(ui->pbtn_char_l);
    setButtonEffect(ui->pbtn_char_m);
    setButtonEffect(ui->pbtn_char_n);
    setButtonEffect(ui->pbtn_char_o);
    setButtonEffect(ui->pbtn_char_p);
    setButtonEffect(ui->pbtn_char_q);
    setButtonEffect(ui->pbtn_char_r);
    setButtonEffect(ui->pbtn_char_s);
    setButtonEffect(ui->pbtn_char_t);
    setButtonEffect(ui->pbtn_char_u);
    setButtonEffect(ui->pbtn_char_v);
    setButtonEffect(ui->pbtn_char_w);
    setButtonEffect(ui->pbtn_char_x);
    setButtonEffect(ui->pbtn_char_y);
    setButtonEffect(ui->pbtn_char_z);
    setButtonEffect(ui->pbtn_char_dot);
    setButtonEffect(ui->pbtn_char_slash);
    setButtonEffect(ui->pbtn_case);
    setButtonEffect(ui->pbtn_backspace);

    connect(ui->pbtn_num_0, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_1, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_2, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_3, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_4, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_5, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_6, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_7, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_8, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_num_9, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_a, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_b, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_c, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_d, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_e, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_f, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_g, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_h, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_i, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_j, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_k, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_l, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_m, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_n, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_o, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_p, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_q, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_r, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_s, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_t, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_u, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_v, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_w, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_x, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_y, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_z, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_dot, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_char_slash, SIGNAL(clicked()), this, SLOT(pbtn_char_clicked()));
    connect(ui->pbtn_backspace, SIGNAL(clicked()), this, SIGNAL(key_backspace_clicked()));
}

KeyboardInput::~KeyboardInput()
{
    delete ui;
}

void KeyboardInput::setButtonEffect(QAbstractButton *button)
{
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setColor(QColor(0,0,0,59));
    shadow_effect->setBlurRadius(10);
    shadow_effect->setOffset(0,5);
    button->setGraphicsEffect(shadow_effect);

    QString default_effect = "";
    QString checked_effect = "";
    QString pressed_effect = "";

    if (button->objectName() == "pbtn_case") {
        default_effect.append("{color: rgb(119, 119, 119);");
        default_effect.append(" background-color: qlineargradient(");
        default_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        default_effect.append(" stop:0.2 rgba(236, 236, 236, 255),");
        default_effect.append(" stop:1.0 rgba(255, 255, 255, 255));");
        default_effect.append(" border: 1px solid lightgray; border-radius:10px;}");
        checked_effect.append("{color: rgb(255, 119, 119);");
        checked_effect.append(" background-color: qlineargradient(");
        checked_effect.append(" spread:pad, x1:0.5, y1:1, x2:0.5, y2:0,");
        checked_effect.append(" stop:0.2 rgba(226, 226, 226, 255),");
        checked_effect.append(" stop:1.0 rgba(245, 245, 245, 255));");
        checked_effect.append(" border: 1px solid lightgray; border-radius:10px;}");
        button->setStyleSheet("QPushButton" + default_effect + "QPushButton:checked" + checked_effect);
    }
    else if (button->objectName() == "pbtn_backspace") {
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
        button->setStyleSheet("QPushButton" + default_effect + "QPushButton:pressed" + pressed_effect);
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
        button->setStyleSheet("QPushButton" + default_effect + "QPushButton:pressed" + pressed_effect);
    }
}

void KeyboardInput::pbtn_char_clicked()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        emit key_char_clicked(pbtn->text());
    }
}

void KeyboardInput::on_pbtn_case_toggled(bool checked)
{
    if (checked) {
        ui->pbtn_char_a->setText("A");
        ui->pbtn_char_b->setText("B");
        ui->pbtn_char_c->setText("C");
        ui->pbtn_char_d->setText("D");
        ui->pbtn_char_e->setText("E");
        ui->pbtn_char_f->setText("F");
        ui->pbtn_char_g->setText("G");
        ui->pbtn_char_h->setText("H");
        ui->pbtn_char_i->setText("I");
        ui->pbtn_char_j->setText("J");
        ui->pbtn_char_k->setText("K");
        ui->pbtn_char_l->setText("L");
        ui->pbtn_char_m->setText("M");
        ui->pbtn_char_n->setText("N");
        ui->pbtn_char_o->setText("O");
        ui->pbtn_char_p->setText("P");
        ui->pbtn_char_q->setText("Q");
        ui->pbtn_char_r->setText("R");
        ui->pbtn_char_s->setText("S");
        ui->pbtn_char_t->setText("T");
        ui->pbtn_char_u->setText("U");
        ui->pbtn_char_v->setText("V");
        ui->pbtn_char_w->setText("W");
        ui->pbtn_char_x->setText("X");
        ui->pbtn_char_y->setText("Y");
        ui->pbtn_char_z->setText("Z");
    }
    else {
        ui->pbtn_char_a->setText("a");
        ui->pbtn_char_b->setText("b");
        ui->pbtn_char_c->setText("c");
        ui->pbtn_char_d->setText("d");
        ui->pbtn_char_e->setText("e");
        ui->pbtn_char_f->setText("f");
        ui->pbtn_char_g->setText("g");
        ui->pbtn_char_h->setText("h");
        ui->pbtn_char_i->setText("i");
        ui->pbtn_char_j->setText("j");
        ui->pbtn_char_k->setText("k");
        ui->pbtn_char_l->setText("l");
        ui->pbtn_char_m->setText("m");
        ui->pbtn_char_n->setText("n");
        ui->pbtn_char_o->setText("o");
        ui->pbtn_char_p->setText("p");
        ui->pbtn_char_q->setText("q");
        ui->pbtn_char_r->setText("r");
        ui->pbtn_char_s->setText("s");
        ui->pbtn_char_t->setText("t");
        ui->pbtn_char_u->setText("u");
        ui->pbtn_char_v->setText("v");
        ui->pbtn_char_w->setText("w");
        ui->pbtn_char_x->setText("x");
        ui->pbtn_char_y->setText("y");
        ui->pbtn_char_z->setText("z");
    }
}
