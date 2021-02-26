#ifndef MACHINE_INFO_H
#define MACHINE_INFO_H

#include <QObject>
#include <QString>

class ItemInfo : public QObject
{
    Q_OBJECT
public:
    ItemInfo(QObject *parent = nullptr){ this->setParent(parent); }

    QString     barcode;
    QString     numbering;
    QString     name;
    QString     specification;
    QString     description;
    QString     original_price;
    QString     picture;
    QString     picture_ext;
    QString     combine_id_1;       // 0:none, 1: combine-39
    QString     combine_id_2;       // 0:none, 1: combine-49
    QString     combine_id_3;       // 0:none, 1: combine-59
    QString     combine_group;      // empty: none, 0: food, 1:drink
};

class LaneInfo : public QObject
{
    Q_OBJECT
public:
    LaneInfo(QObject *parent = nullptr){ this->setParent(parent); }

    QString     numbering;
    QString     expired_time;
    QString     amount_max;
    QString     amount_remain;

    ItemInfo    item_info;

    QString     payment_1_price = "";
    QString     payment_2_price = "";
    QString     payment_3_price = "";
    QString     payment_4_price = "";
};

class MachineInfo : public QObject
{
    Q_OBJECT
public:
    MachineInfo(QObject *parent = nullptr){ this->setParent(parent); }

    QString     machine_code;
    QString     status;
    QString     lane_amount;
    QString     payment_1;
    QString     payment_2;
    QString     payment_3;
    QString     payment_4;

    QList<LaneInfo*> lane_infos;
};

#endif // MACHINE_INFO_H
