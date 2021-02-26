#include "product_button.h"
#include "ui_product_button.h"

#include "main_window.h"

#include <QGraphicsDropShadowEffect>

ProductButton::ProductButton(QWidget *parent, int numbering, QString item_name, int price)
    : QPushButton(parent)
    , ui(new Ui::ProductButton)
    , numbering_(numbering)
{
    ui->setupUi(this);

    QString default_effect;
    default_effect.append("{background-color: rgba(255, 255, 255, 0);");
    default_effect.append(" border: none;}");
    QString pressed_effect;
    pressed_effect.append("{background-color: rgba(255, 255, 255, 0);");
    pressed_effect.append(" border: 4px solid rgb(244, 157, 42); border-radius:10px;}");
    this->setStyleSheet("QPushButton" + default_effect + "QPushButton:pressed" + pressed_effect);

    QPixmap sold_out_pixmap(QString(dir_resources) + "lbl_sold_out.png");
    ui->lbl_sold_out->setPixmap(sold_out_pixmap);

    ui->lbl_item_name->setText(item_name);
    ui->lbl_item_price->setText(QString("NT. %1").arg(price));

    ui->lbl_item_image->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbl_item_name->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->lbl_item_price->setAttribute(Qt::WA_TranslucentBackground, true);
}

ProductButton::~ProductButton()
{

}

int ProductButton::numbering()
{
    return numbering_;
}

QString ProductButton::itemName()
{
    return  item_name_;
}

QString ProductButton::itemImage()
{
    return image_path_;
}

int ProductButton::itemPrice()
{
    return ui->lbl_item_price->text().mid(4).toInt();
}

void ProductButton::setItemVisible(bool visible)
{
    this->setEnabled(visible);
    ui->lbl_item_image->setVisible(visible);
    ui->lbl_item_name->setVisible(visible);
    ui->lbl_item_price->setVisible(visible);
    ui->lbl_sold_out->setVisible(visible);
}

void ProductButton::setNumbering(int number)
{
    numbering_ = number;
}

void ProductButton::setItemName(QString name)
{
    item_name_ = name;
    ui->lbl_item_name->setText(name);
}

void ProductButton::setItemImage(QString image_path)
{
    image_path_ = image_path;

    QPixmap pixmap = QPixmap(image_path_);
    if (pixmap.isNull() == false) {
        pixmap = pixmap.scaled(ui->lbl_item_image->width(), ui->lbl_item_image->height(), Qt::KeepAspectRatio);
        ui->lbl_item_image->setPixmap(pixmap);
    }
}

void ProductButton::setItemPrice(int price)
{
    ui->lbl_item_price->setText(QString("NT. %1").arg(price));
}

void ProductButton::setItemEnabled(bool enabled)
{
    this->setEnabled(enabled);
    ui->lbl_sold_out->setVisible(!enabled);
}
