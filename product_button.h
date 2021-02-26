#ifndef PRODUCT_BUTTON_H
#define PRODUCT_BUTTON_H

#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class ProductButton; }
QT_END_NAMESPACE

class ProductButton : public QPushButton
{
    Q_OBJECT

public:
    ProductButton(QWidget *parent = nullptr, int numbering = 0, QString item_name = "", int price = 0);
    ~ProductButton();

    int numbering();
    QString itemName();
    QString itemImage();
    int itemPrice();

    void setItemVisible(bool visible);
    void setNumbering(int number);
    void setItemName(QString name);
    void setItemImage(QString image_path);
    void setItemPrice(int price);
    void setItemEnabled(bool enabled);

private:
    Ui::ProductButton *ui;

    int numbering_;
    QString item_name_;
    QString image_path_;
};

#endif // PRODUCT_BUTTON_H
