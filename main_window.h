#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QStandardItemModel>

#include "cms_api.h"
#include "vm_controller.h"
#include "fvm_controller.h"
#include "cvm_controller.h"
#include "nccc_edc.h"
#include "printer_kp247.h"
#include "machine_info.h"

#ifdef _DEV_DEBUG_
#ifdef _WIN32
#define ipc_config      "D:/Qt Projects/_HillEver/demo_complex/data/ipc.json"
#define dir_ad          "D:/Qt Projects/_HillEver/demo_complex/data/ad/"
#define dir_image       "D:/Qt Projects/_HillEver/demo_complex/data/image/"
#define dir_resources   "D:/Qt Projects/_HillEver/demo_complex/resources/"
#else
#define ipc_config      "/home/dev/Qt Projects/demo_complex/data/ipc.json"
#define dir_ad          "/home/dev/Qt Projects/demo_complex/data/ad/"
#define dir_image       "/home/dev/Qt Projects/demo_complex/data/image/"
#define dir_resources   "/home/dev/Qt Projects/demo_complex/resources/"
#endif
#else
#define ipc_config      "/home/dev/demo_complex/data/ipc.json"
#define dir_ad          "/home/dev/demo_complex/data/ad/"
#define dir_image       "/home/dev/demo_complex/data/image/"
#define dir_resources   "/home/dev/demo_complex/resources/"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ProductButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum PageId {
        Home            = 0,
        OnSiteOption    = 1,
        OnSiteConfirm   = 2,
        OnSiteCart      = 3,
        OnSiteCheckout  = 4,
        PaymentOption   = 5,
        PaymentHint     = 6,
        EinvoiceOption  = 7,
        EinvoiceHint    = 8,
        OnSiteResult    = 9,
        PreOrderHint    = 10,
        PreOrderConfirm = 11,
        PreOrderResult  = 12,
        ECPickupHint    = 13,
        ECPickupResult  = 14,
        Finish          = 15,
        Test            = 16
    };

    enum FoodClass {
        Bento,
        Drink,
        Dessert
    };

    enum PaymentType {
        FamiPay     = 0,
        CreditCard  = 1,
        EasyCard    = 2
    };

    enum EInvoiceType {
        Print       = 0,
        BanPrint    = 1,
        Carrier     = 2,
        LoveCode    = 3
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUI();
    void initPort();
    void initMachine();
    void initLaneInfo();
    void refreshCatalog();
    void finish();

private slots:
    void page_changed(int index);
    void pbtn_onsite_pressed();
    void pbtn_onsite_released();
    void pbtn_preorder_pressed();
    void pbtn_preorder_released();
    void pbtn_ec_pickup_pressed();
    void pbtn_ec_pickup_released();
    void pbtn_home_upper_pressed();
    void pbtn_home_upper_released();
    void pbtn_home_pressed();
    void pbtn_home_released();
    void pbtn_back_pressed();
    void pbtn_back_released();
    void pbtn_confirm_pressed();
    void pbtn_confirm_released();
    void pbtn_open_pressed();
    void pbtn_open_released();
    void pbtn_finish_pressed();
    void pbtn_finish_released();
    void pbtn_onsite_option_toggled(bool checked);
    void pbtn_onsite_product_released();
    void pbtn_onsite_prev_pressed();
    void pbtn_onsite_prev_released();
    void pbtn_onsite_next_pressed();
    void pbtn_onsite_next_released();
    void pbtn_onsite_checkout_pressed();
    void pbtn_onsite_checkout_released();
    void pbtn_payment_option_pressed();
    void pbtn_payment_option_released();
    void pbtn_einvoice_option_pressed();
    void pbtn_einvoice_option_released();
    void tableView_clicked(const QModelIndex &index);
    void lineEdit_textChanged(QString text);
    void num_input_num_clicked(QString number);
    void num_input_clear_clicked();
    void num_input_backspace_clicked();
    void keyboard_char_clicked(QString character);
    void keyboard_backspace_clicked();

    void watch_pulling();
    void watch_vmc();
    void watch_scanner();
    void watch_payment();
    void watch_operation();
    void watch_door_status();

    void vmcTimeoutWithState(QString err_msg, int state);
    void getVmcFirmwareInfosResponse(QString infos);
    void getVmcTemperatureStatusResponse(QString status);
    void setVmcCompressorSwitchResponse(bool result);
    void setVmcDoorSwitchResponse(bool result);
    void executeVmcChannelResponse(bool result, int state);
    void fvmcOpenDoorResponse(int station, int numbering, bool result);
    void fvmcReadDoorStatusResponse(int station, bool result);
    void cvmcDoorStatusResponse();
    void executeCardTransactionResponse(bool result);
    void executeCardSettlementResponse(bool result);
    void scannerDataReady();

    void on_pbtn_test_released();
    void on_cbBox_port_currentIndexChanged(const QString &arg1);
    void on_pbtn_test_open_released();

private:
    Ui::MainWindow *ui;

    CmsApi* cms_api_;

    VMController    *port_hvmc_;
    FVMController   *port_fvmc_;
    CVMController   *port_cvmc_;
    NcccEDC         *port_card_;
    QSerialPort     *port_scanner_;
    PrinterKP247    *port_printer_;

    QTimer *tmr_watch_pulling_;
    QTimer *tmr_watch_door_status_;
    int counter_watch_vmc_;
    QTimer *tmr_watch_vmc_;
    int counter_watch_scanner_;
    QTimer *tmr_watch_scanner_;
    int counter_watch_payment_;
    QTimer *tmr_watch_payment_;
    int counter_watch_operation_;
    QTimer *tmr_watch_operation_;

    MachineInfo*        machine_info_;
    QList<LaneInfo*>    lane_infos_bento_;
    QList<LaneInfo*>    lane_infos_drink_;
    QList<LaneInfo*>    lane_infos_dessert_;
    QList<LaneInfo*>    lane_infos_editing_;
    ProductButton       *pbtn_product_editing_;

    QStandardItemModel  *cart_model_;
    PageId              current_page_;
    PaymentType         payment_type_;
    EInvoiceType        einvoice_type_;
    bool                can_input_;
    bool                is_dessert_;
};
#endif // MAIN_WINDOW_H
