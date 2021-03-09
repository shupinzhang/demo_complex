#include "main_window.h"
#include "ui_main_window.h"

#include <QSerialPortInfo>
#include <QMovie>
#include <QDir>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QJsonDocument>
#include <QThread>
#include <QDebug>

#include "product_button.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef _DEV_DEBUG_
    foreach (QSerialPortInfo port_info, QSerialPortInfo::availablePorts()) {
        qDebug() << port_info.portName()
                 << port_info.description()
                 << port_info.systemLocation();
    }
#endif

#ifndef _DEV_DEBUG_
    // set cursor to blank
    QApplication::setOverrideCursor(Qt::BlankCursor);
#endif

    initUI();
    initPort();
    initMachine();
    initLaneInfo();

    // create cms api object
    cms_api_ = new CmsApi(this);
    cms_api_->setDebugEnabled(false);

    tmr_watch_pulling_ = new QTimer(this);
    tmr_watch_pulling_->setInterval(1000 * 3);
    connect(tmr_watch_pulling_, SIGNAL(timeout()), this, SLOT(watch_pulling()));

    tmr_watch_vmc_ = new QTimer(this);
    tmr_watch_vmc_->setInterval(1000);
    connect(tmr_watch_vmc_, SIGNAL(timeout()), this, SLOT(watch_vmc()));

    tmr_watch_scanner_ = new QTimer(this);
    tmr_watch_scanner_->setInterval(1000);
    connect(tmr_watch_scanner_, SIGNAL(timeout()), this, SLOT(watch_scanner()));

    tmr_watch_payment_ = new QTimer(this);
    tmr_watch_payment_->setInterval(1000);
    connect(tmr_watch_payment_, SIGNAL(timeout()), this, SLOT(watch_payment()));

    tmr_watch_operation_ = new QTimer(this);
    tmr_watch_operation_->setInterval(1000);
    connect(tmr_watch_operation_, SIGNAL(timeout()), this, SLOT(watch_operation()));

    tmr_watch_door_status_ = new QTimer(this);
    tmr_watch_door_status_->setInterval(1000);
    connect(tmr_watch_door_status_, SIGNAL(timeout()), this, SLOT(watch_door_status()));

    lane_infos_editing_ = lane_infos_bento_;
    refreshCatalog();

    ui->stackedWidget->setCurrentIndex(Home);
    tmr_watch_pulling_->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    QString resources_path = QString(dir_resources);

    QString transparent_effect;
    transparent_effect.append("{background-color: rgba(255, 255, 255, 0);");
    transparent_effect.append(" border: none;}");

    QString transparent_stylesheet;
    transparent_stylesheet.append("QPushButton" + transparent_effect);
    transparent_stylesheet.append("QPushButton:pressed" + transparent_effect);

    // initial brand label
    ui->lbl_brand_upper->setPixmap(QPixmap(resources_path + "_HE.png"));

    // initial media player
    QMediaPlayer *player = new QMediaPlayer(this);
    QMediaPlaylist *playlist = new QMediaPlaylist(player);
    QDir dir(dir_ad);
    QStringList filters;
    filters << "*.mp4";
    foreach (QString mp4_file, dir.entryList(filters)) {
        playlist->addMedia(QUrl::fromLocalFile(QString(dir_ad) + mp4_file));
    }
    player->setPlaylist(playlist);
    player->setVideoOutput(ui->videoWidget);
    playlist->setPlaybackMode(QMediaPlaylist::Loop);
    //player->play();
    ui->lbl_ad_display->setPixmap(QPixmap(QString(dir_ad) + "ad.jpg"));

    // initialize home button
    QPixmap home_upper_pixmap(resources_path + "btn_home_upper.png");
    ui->pbtn_home_upper->setFlat(true);
    ui->pbtn_home_upper->setText("");
    ui->pbtn_home_upper->setIcon(QIcon(home_upper_pixmap));
    ui->pbtn_home_upper->setIconSize(ui->pbtn_home_upper->size());
    ui->pbtn_home_upper->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_home_upper, SIGNAL(pressed()), this, SLOT(pbtn_home_upper_pressed()));
    connect(ui->pbtn_home_upper, SIGNAL(released()), this, SLOT(pbtn_home_upper_released()));

    // initialize soft keyboard
    ui->keyboard_input->hide();
    connect(ui->keyboard_input, SIGNAL(key_char_clicked(QString)), this, SLOT(keyboard_char_clicked(QString)));
    connect(ui->keyboard_input, SIGNAL(key_backspace_clicked()), this, SLOT(keyboard_backspace_clicked()));

    // initialize home page
    QPixmap home_bg_pixmap(resources_path + "P0_home.jpg");
    ui->lbl_home_bg->setPixmap(home_bg_pixmap);
    ui->pbtn_test->setFlat(true);
    ui->pbtn_test->setText("");
    ui->pbtn_test->setStyleSheet(transparent_stylesheet);
    QPixmap onsite_pixmap(resources_path + "btn_onsite.png");
    ui->pbtn_onsite->setFlat(true);
    ui->pbtn_onsite->setText("");
    ui->pbtn_onsite->setIcon(QIcon(onsite_pixmap));
    ui->pbtn_onsite->setIconSize(ui->pbtn_onsite->size());
    ui->pbtn_onsite->setStyleSheet(transparent_stylesheet);
    QPixmap preorder_pixmap(resources_path + "btn_preorder.png");
    ui->pbtn_preorder->setFlat(true);
    ui->pbtn_preorder->setText("");
    ui->pbtn_preorder->setIcon(QIcon(preorder_pixmap));
    ui->pbtn_preorder->setIconSize(ui->pbtn_preorder->size());
    ui->pbtn_preorder->setStyleSheet(transparent_stylesheet);
    QPixmap ec_pickup_pixmap(resources_path + "btn_ec_pickup.png");
    ui->pbtn_ec_pickup->setFlat(true);
    ui->pbtn_ec_pickup->setText("");
    ui->pbtn_ec_pickup->setIcon(QIcon(ec_pickup_pixmap));
    ui->pbtn_ec_pickup->setIconSize(ui->pbtn_ec_pickup->size());
    ui->pbtn_ec_pickup->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_onsite, SIGNAL(pressed()), this, SLOT(pbtn_onsite_pressed()));
    connect(ui->pbtn_onsite, SIGNAL(released()), this, SLOT(pbtn_onsite_released()));
    connect(ui->pbtn_preorder, SIGNAL(pressed()), this, SLOT(pbtn_preorder_pressed()));
    connect(ui->pbtn_preorder, SIGNAL(released()), this, SLOT(pbtn_preorder_released()));
    connect(ui->pbtn_ec_pickup, SIGNAL(pressed()), this, SLOT(pbtn_ec_pickup_pressed()));
    connect(ui->pbtn_ec_pickup, SIGNAL(released()), this, SLOT(pbtn_ec_pickup_released()));

    // initialize onsite option page
    QPixmap onsite_option_bg_pixmap(resources_path + "P1_onsite_option.jpg");
    ui->lbl_onsite_option_bg->setPixmap(onsite_option_bg_pixmap);
    QPixmap bento_pixmap(resources_path + "btn_bento_checked.png");
    ui->pbtn_onsite_bento->setFlat(true);
    ui->pbtn_onsite_bento->setText("");
    ui->pbtn_onsite_bento->setIcon(QIcon(bento_pixmap));
    ui->pbtn_onsite_bento->setIconSize(ui->pbtn_onsite_bento->size());
    ui->pbtn_onsite_bento->setStyleSheet(transparent_stylesheet);
    ui->pbtn_onsite_bento->setChecked(true);
    QPixmap drink_pixmap(resources_path + "btn_drink.png");
    ui->pbtn_onsite_drink->setFlat(true);
    ui->pbtn_onsite_drink->setText("");
    ui->pbtn_onsite_drink->setIcon(QIcon(drink_pixmap));
    ui->pbtn_onsite_drink->setIconSize(ui->pbtn_onsite_drink->size());
    ui->pbtn_onsite_drink->setStyleSheet(transparent_stylesheet);
    QPixmap dessert_pixmap(resources_path + "btn_dessert.png");
    ui->pbtn_onsite_dessert->setFlat(true);
    ui->pbtn_onsite_dessert->setText("");
    ui->pbtn_onsite_dessert->setIcon(QIcon(dessert_pixmap));
    ui->pbtn_onsite_dessert->setIconSize(ui->pbtn_onsite_dessert->size());
    ui->pbtn_onsite_dessert->setStyleSheet(transparent_stylesheet);
    QPixmap prev_pixmap(resources_path + "btn_prev.png");
    ui->pbtn_onsite_prev->setFlat(true);
    ui->pbtn_onsite_prev->setText("");
    ui->pbtn_onsite_prev->setIcon(QIcon(prev_pixmap));
    ui->pbtn_onsite_prev->setIconSize(ui->pbtn_onsite_prev->size());
    ui->pbtn_onsite_prev->setStyleSheet(transparent_stylesheet);
    ui->pbtn_onsite_prev->setEnabled(false);
    QPixmap next_pixmap(resources_path + "btn_next.png");
    ui->pbtn_onsite_next->setFlat(true);
    ui->pbtn_onsite_next->setText("");
    ui->pbtn_onsite_next->setIcon(QIcon(next_pixmap));
    ui->pbtn_onsite_next->setIconSize(ui->pbtn_onsite_next->size());
    ui->pbtn_onsite_next->setStyleSheet(transparent_stylesheet);
    ui->pbtn_onsite_next->setEnabled(false);
    QPixmap checkout_pixmap(resources_path + "btn_checkout.png");
    ui->pbtn_onsite_checkout->setFlat(true);
    ui->pbtn_onsite_checkout->setText("");
    ui->pbtn_onsite_checkout->setIcon(QIcon(checkout_pixmap));
    ui->pbtn_onsite_checkout->setIconSize(ui->pbtn_onsite_checkout->size());
    ui->pbtn_onsite_checkout->setStyleSheet(transparent_stylesheet);

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            ProductButton *pbtn = new ProductButton(this);
            pbtn->setObjectName(QString("pbtn_p%1%2").arg(row).arg(col));
            ui->layout_onsite_product->addWidget(pbtn, row, col);
            connect(pbtn, SIGNAL(released()), this, SLOT(pbtn_onsite_product_released()));
        }
    }
    connect(ui->pbtn_onsite_bento, SIGNAL(toggled(bool)), this, SLOT(pbtn_onsite_option_toggled(bool)));
    connect(ui->pbtn_onsite_drink, SIGNAL(toggled(bool)), this, SLOT(pbtn_onsite_option_toggled(bool)));
    connect(ui->pbtn_onsite_dessert, SIGNAL(toggled(bool)), this, SLOT(pbtn_onsite_option_toggled(bool)));
    connect(ui->pbtn_onsite_prev, SIGNAL(pressed()), this, SLOT(pbtn_onsite_prev_pressed()));
    connect(ui->pbtn_onsite_prev, SIGNAL(released()), this, SLOT(pbtn_onsite_prev_released()));
    connect(ui->pbtn_onsite_next, SIGNAL(pressed()), this, SLOT(pbtn_onsite_next_pressed()));
    connect(ui->pbtn_onsite_next, SIGNAL(released()), this, SLOT(pbtn_onsite_next_released()));
    connect(ui->pbtn_onsite_checkout, SIGNAL(pressed()), this, SLOT(pbtn_onsite_checkout_pressed()));
    connect(ui->pbtn_onsite_checkout, SIGNAL(released()), this, SLOT(pbtn_onsite_checkout_released()));
    connect(ui->tView_onsite_option, SIGNAL(clicked(const QModelIndex &)), this, SLOT(tableView_clicked(const QModelIndex &)));

    // initialize onsite confirm page
    QPixmap onsite_confirm_bg_pixmap(resources_path + "P2_onsite_confirm.jpg");
    ui->lbl_onsite_confirm_bg->setPixmap(onsite_confirm_bg_pixmap);
    QPixmap add_into_cart_pixmap(resources_path + "btn_add_into_cart.png");
    ui->pbtn_product_confirm->setFlat(true);
    ui->pbtn_product_confirm->setText("");
    ui->pbtn_product_confirm->setIcon(QIcon(add_into_cart_pixmap));
    ui->pbtn_product_confirm->setIconSize(ui->pbtn_product_confirm->size());
    ui->pbtn_product_confirm->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_product_close, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_product_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_product_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));

    // initialize onsite cart page
    QPixmap onsite_cart_bg_pixmap(resources_path + "P3_onsite_cart.jpg");
    ui->lbl_onsite_cart_bg->setPixmap(onsite_cart_bg_pixmap);

    // initialize onsite checkout page
    QPixmap onsite_checkout_bg_pixmap(resources_path + "P4_onsite_checkout.jpg");
    ui->lbl_onsite_checkout_bg->setPixmap(onsite_checkout_bg_pixmap);
    QPixmap back_pixmap(resources_path + "btn_back.png");
    ui->pbtn_checkout_back->setFlat(true);
    ui->pbtn_checkout_back->setText("");
    ui->pbtn_checkout_back->setIcon(QIcon(back_pixmap));
    ui->pbtn_checkout_back->setIconSize(ui->pbtn_checkout_back->size());
    ui->pbtn_checkout_back->setStyleSheet(transparent_stylesheet);
    QPixmap confirm_pixmap(resources_path + "btn_confirm.png");
    ui->pbtn_checkout_confirm->setFlat(true);
    ui->pbtn_checkout_confirm->setText("");
    ui->pbtn_checkout_confirm->setIcon(QIcon(confirm_pixmap));
    ui->pbtn_checkout_confirm->setIconSize(ui->pbtn_checkout_confirm->size());
    ui->pbtn_checkout_confirm->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_checkout_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_checkout_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_checkout_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_checkout_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));

    // initialize payment option page
    QPixmap payment_option_bg_pixmap(resources_path + "P5_payment_option.jpg");
    ui->lbl_payment_option_bg->setPixmap(payment_option_bg_pixmap);
    ui->pbtn_payment_option_back->setFlat(true);
    ui->pbtn_payment_option_back->setText("");
    ui->pbtn_payment_option_back->setIcon(QIcon(back_pixmap));
    ui->pbtn_payment_option_back->setIconSize(ui->pbtn_payment_option_back->size());
    ui->pbtn_payment_option_back->setStyleSheet(transparent_stylesheet);
    QPixmap famiPay_pixmap(resources_path + "btn_famiPay.png");
    ui->pbtn_payment_famiPay->setFlat(true);
    ui->pbtn_payment_famiPay->setText("");
    ui->pbtn_payment_famiPay->setIcon(QIcon(famiPay_pixmap));
    ui->pbtn_payment_famiPay->setIconSize(ui->pbtn_payment_famiPay->size());
    ui->pbtn_payment_famiPay->setStyleSheet(transparent_stylesheet);
    QPixmap creditcard_pixmap(resources_path + "btn_creditcard.png");
    ui->pbtn_payment_creditcard->setFlat(true);
    ui->pbtn_payment_creditcard->setText("");
    ui->pbtn_payment_creditcard->setIcon(QIcon(creditcard_pixmap));
    ui->pbtn_payment_creditcard->setIconSize(ui->pbtn_payment_creditcard->size());
    ui->pbtn_payment_creditcard->setStyleSheet(transparent_stylesheet);
    QPixmap ezcard_pixmap(resources_path + "btn_easycard.png");
    ui->pbtn_payment_easycard->setFlat(true);
    ui->pbtn_payment_easycard->setText("");
    ui->pbtn_payment_easycard->setIcon(QIcon(ezcard_pixmap));
    ui->pbtn_payment_easycard->setIconSize(ui->pbtn_payment_easycard->size());
    ui->pbtn_payment_easycard->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_payment_option_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_payment_option_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_payment_famiPay, SIGNAL(pressed()), this, SLOT(pbtn_payment_option_pressed()));
    connect(ui->pbtn_payment_famiPay, SIGNAL(released()), this, SLOT(pbtn_payment_option_released()));
    connect(ui->pbtn_payment_creditcard, SIGNAL(pressed()), this, SLOT(pbtn_payment_option_pressed()));
    connect(ui->pbtn_payment_creditcard, SIGNAL(released()), this, SLOT(pbtn_payment_option_released()));
    connect(ui->pbtn_payment_easycard, SIGNAL(pressed()), this, SLOT(pbtn_payment_option_pressed()));
    connect(ui->pbtn_payment_easycard, SIGNAL(released()), this, SLOT(pbtn_payment_option_released()));
    ui->pbtn_payment_easycard->setGeometry(ui->pbtn_payment_creditcard->geometry());
    ui->pbtn_payment_creditcard->setGeometry(ui->pbtn_payment_famiPay->geometry());
    ui->pbtn_payment_famiPay->hide();

    // initialize payment hint page
    QPixmap payment_hint_bg_pixmap(resources_path + "P6_payment_hint.jpg");
    ui->lbl_payment_hint_bg->setPixmap(payment_hint_bg_pixmap);
    connect(ui->pbtn_payment_hint_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));

    // initialize einvoice option page
    QPixmap einvoice_option_bg_pixmap(resources_path + "P7_einvoice_option.jpg");
    ui->lbl_einvoice_option_bg->setPixmap(einvoice_option_bg_pixmap);
    ui->pbtn_einvoice_option_back->setFlat(true);
    ui->pbtn_einvoice_option_back->setText("");
    ui->pbtn_einvoice_option_back->setIcon(QIcon(back_pixmap));
    ui->pbtn_einvoice_option_back->setIconSize(ui->pbtn_einvoice_option_back->size());
    ui->pbtn_einvoice_option_back->setStyleSheet(transparent_stylesheet);
    QPixmap carrier_pixmap(resources_path + "btn_carrier.png");
    ui->pbtn_einvoice_carrier->setFlat(true);
    ui->pbtn_einvoice_carrier->setText("");
    ui->pbtn_einvoice_carrier->setIcon(QIcon(carrier_pixmap));
    ui->pbtn_einvoice_carrier->setIconSize(ui->pbtn_einvoice_carrier->size());
    ui->pbtn_einvoice_carrier->setStyleSheet(transparent_stylesheet);
    QPixmap lovecode_pixmap(resources_path + "btn_lovecode.png");
    ui->pbtn_einvoice_lovecode->setFlat(true);
    ui->pbtn_einvoice_lovecode->setText("");
    ui->pbtn_einvoice_lovecode->setIcon(QIcon(lovecode_pixmap));
    ui->pbtn_einvoice_lovecode->setIconSize(ui->pbtn_einvoice_lovecode->size());
    ui->pbtn_einvoice_lovecode->setStyleSheet(transparent_stylesheet);
    QPixmap print_pixmap(resources_path + "btn_print.png");
    ui->pbtn_einvoice_print->setFlat(true);
    ui->pbtn_einvoice_print->setText("");
    ui->pbtn_einvoice_print->setIcon(QIcon(print_pixmap));
    ui->pbtn_einvoice_print->setIconSize(ui->pbtn_einvoice_print->size());
    ui->pbtn_einvoice_print->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_einvoice_option_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_einvoice_option_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_einvoice_carrier, SIGNAL(pressed()), this, SLOT(pbtn_einvoice_option_pressed()));
    connect(ui->pbtn_einvoice_carrier, SIGNAL(released()), this, SLOT(pbtn_einvoice_option_released()));
    connect(ui->pbtn_einvoice_lovecode, SIGNAL(pressed()), this, SLOT(pbtn_einvoice_option_pressed()));
    connect(ui->pbtn_einvoice_lovecode, SIGNAL(released()), this, SLOT(pbtn_einvoice_option_released()));
    connect(ui->pbtn_einvoice_print, SIGNAL(pressed()), this, SLOT(pbtn_einvoice_option_pressed()));
    connect(ui->pbtn_einvoice_print, SIGNAL(released()), this, SLOT(pbtn_einvoice_option_released()));

    // initialize einvoice hint page
    QPixmap einvoice_hint_bg_pixmap(resources_path + "P8_einvoice_hint.jpg");
    ui->lbl_einvoice_hint_bg->setPixmap(einvoice_hint_bg_pixmap);
    ui->lbl_einvoice_hint_arrow->setText("");
    ui->lbl_einvoice_hint_arrow->setFrameStyle(QFrame::NoFrame);
    ui->lbl_einvoice_hint_arrow->setMovie(new QMovie(resources_path + "lbl_scanner_arrow.gif"));
    ui->lbl_einvoice_hint_arrow->movie()->start();
    QPixmap gback_pixmap(resources_path + "btn_gback.png");
    ui->pbtn_einvoice_hint_back->setFlat(true);
    ui->pbtn_einvoice_hint_back->setText("");
    ui->pbtn_einvoice_hint_back->setIcon(QIcon(gback_pixmap));
    ui->pbtn_einvoice_hint_back->setIconSize(ui->pbtn_einvoice_hint_back->size());
    ui->pbtn_einvoice_hint_back->setStyleSheet(transparent_stylesheet);
    QPixmap gconfirm_pixmap(resources_path + "btn_gconfirm.png");
    ui->pbtn_einvoice_hint_confirm->setFlat(true);
    ui->pbtn_einvoice_hint_confirm->setText("");
    ui->pbtn_einvoice_hint_confirm->setIcon(QIcon(gconfirm_pixmap));
    ui->pbtn_einvoice_hint_confirm->setIconSize(ui->pbtn_einvoice_hint_confirm->size());
    ui->pbtn_einvoice_hint_confirm->setStyleSheet(transparent_stylesheet);
    connect(ui->lineEdit_einvoice_code, SIGNAL(textChanged(QString)), this, SLOT(lineEdit_textChanged(QString)));
    connect(ui->num_input, SIGNAL(key_num_clicked(QString)), this, SLOT(num_input_num_clicked(QString)));
    connect(ui->num_input, SIGNAL(key_clear_clicked()), this, SLOT(num_input_clear_clicked()));
    connect(ui->num_input, SIGNAL(key_backspace_clicked()), this, SLOT(num_input_backspace_clicked()));
    connect(ui->pbtn_einvoice_hint_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_einvoice_hint_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_einvoice_hint_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_einvoice_hint_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));

    // initialize onsite result page
    QPixmap onsite_result_bg_pixmap(resources_path + "P9_onsite_result.jpg");
    ui->lbl_onsite_result_bg->setPixmap(onsite_result_bg_pixmap);
    ui->lbl_onsite_open_hint_A->setText("");
    ui->lbl_onsite_open_hint_A->setFrameStyle(QFrame::NoFrame);
    ui->lbl_onsite_open_hint_A->setMovie(new QMovie(resources_path + "lbl_pickup_hint.gif"));
    ui->lbl_onsite_open_hint_A->movie()->start();
    ui->lbl_onsite_open_hint_D->setText("");
    ui->lbl_onsite_open_hint_D->setFrameStyle(QFrame::NoFrame);
    ui->lbl_onsite_open_hint_D->setMovie(new QMovie(resources_path + "lbl_pickup_hint.gif"));
    ui->lbl_onsite_open_hint_D->movie()->start();
    QPixmap pickup_pixmap(resources_path + "btn_pickup.png");
    ui->pbtn_onsite_open_A->setFlat(true);
    ui->pbtn_onsite_open_A->setText("");
    ui->pbtn_onsite_open_A->setIcon(QIcon(pickup_pixmap));
    ui->pbtn_onsite_open_A->setIconSize(ui->pbtn_onsite_open_A->size());
    ui->pbtn_onsite_open_A->setStyleSheet(transparent_stylesheet);
    ui->pbtn_onsite_open_B->setFlat(true);
    ui->pbtn_onsite_open_B->setText("");
    ui->pbtn_onsite_open_B->setIcon(QIcon(pickup_pixmap));
    ui->pbtn_onsite_open_B->setIconSize(ui->pbtn_onsite_open_B->size());
    ui->pbtn_onsite_open_B->setStyleSheet(transparent_stylesheet);
    ui->pbtn_onsite_open_D->setFlat(true);
    ui->pbtn_onsite_open_D->setText("");
    ui->pbtn_onsite_open_D->setIcon(QIcon(pickup_pixmap));
    ui->pbtn_onsite_open_D->setIconSize(ui->pbtn_onsite_open_D->size());
    ui->pbtn_onsite_open_D->setStyleSheet(transparent_stylesheet);
    QPixmap finish_pixmap(resources_path + "btn_finish.png");
    ui->pbtn_onsite_finish->setFlat(true);
    ui->pbtn_onsite_finish->setText("");
    ui->pbtn_onsite_finish->setIcon(QIcon(finish_pixmap));
    ui->pbtn_onsite_finish->setIconSize(ui->pbtn_onsite_finish->size());
    ui->pbtn_onsite_finish->setStyleSheet(transparent_stylesheet);
    ui->lbl_onsite_result_hint->setText("");
    ui->lbl_onsite_result_hint->setFrameStyle(QFrame::NoFrame);
    ui->lbl_onsite_result_hint->setMovie(new QMovie(resources_path + "lbl_result_hint.gif"));
    ui->lbl_onsite_result_hint->movie()->start();
    connect(ui->pbtn_onsite_open_A, SIGNAL(pressed()), this, SLOT(pbtn_open_pressed()));
    connect(ui->pbtn_onsite_open_A, SIGNAL(released()), this, SLOT(pbtn_open_released()));
    connect(ui->pbtn_onsite_open_B, SIGNAL(pressed()), this, SLOT(pbtn_open_pressed()));
    connect(ui->pbtn_onsite_open_B, SIGNAL(released()), this, SLOT(pbtn_open_released()));
    connect(ui->pbtn_onsite_open_D, SIGNAL(pressed()), this, SLOT(pbtn_open_pressed()));
    connect(ui->pbtn_onsite_open_D, SIGNAL(released()), this, SLOT(pbtn_open_released()));
    connect(ui->pbtn_onsite_finish, SIGNAL(pressed()), this, SLOT(pbtn_finish_pressed()));
    connect(ui->pbtn_onsite_finish, SIGNAL(released()), this, SLOT(pbtn_finish_released()));

    // initialize preorder hint page
    QPixmap preorder_hint_bg_pixmap(resources_path + "P10_preorder_hint.jpg");
    ui->lbl_preorder_hint_bg->setPixmap(preorder_hint_bg_pixmap);
    ui->pbtn_preorder_hint_back->setFlat(true);
    ui->pbtn_preorder_hint_back->setText("");
    ui->pbtn_preorder_hint_back->setIcon(QIcon(gback_pixmap));
    ui->pbtn_preorder_hint_back->setIconSize(ui->pbtn_preorder_hint_back->size());
    ui->pbtn_preorder_hint_back->setStyleSheet(transparent_stylesheet);
    ui->pbtn_preorder_hint_confirm->setFlat(true);
    ui->pbtn_preorder_hint_confirm->setText("");
    ui->pbtn_preorder_hint_confirm->setIcon(QIcon(gconfirm_pixmap));
    ui->pbtn_preorder_hint_confirm->setIconSize(ui->pbtn_preorder_hint_confirm->size());
    ui->pbtn_preorder_hint_confirm->setStyleSheet(transparent_stylesheet);
    ui->lbl_preorder_hint_arrow->setText("");
    ui->lbl_preorder_hint_arrow->setFrameStyle(QFrame::NoFrame);
    ui->lbl_preorder_hint_arrow->setMovie(new QMovie(resources_path + "lbl_scanner_arrow.gif"));
    ui->lbl_preorder_hint_arrow->movie()->start();
    connect(ui->pbtn_preorder_hint_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_preorder_hint_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_preorder_hint_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_preorder_hint_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));
    connect(ui->lineEdit_preorder_code, SIGNAL(textChanged(QString)), this, SLOT(lineEdit_textChanged(QString)));

    // initialize preorder confirm page
    QPixmap preorder_confirm_bg_pixmap(resources_path + "P11_preorder_confirm.jpg");
    ui->lbl_preorder_confirm_bg->setPixmap(preorder_confirm_bg_pixmap);
    ui->pbtn_preorder_back->setFlat(true);
    ui->pbtn_preorder_back->setText("");
    ui->pbtn_preorder_back->setIcon(QIcon(back_pixmap));
    ui->pbtn_preorder_back->setIconSize(ui->pbtn_preorder_back->size());
    ui->pbtn_preorder_back->setStyleSheet(transparent_stylesheet);
    ui->pbtn_preorder_confirm->setFlat(true);
    ui->pbtn_preorder_confirm->setText("");
    ui->pbtn_preorder_confirm->setIcon(QIcon(confirm_pixmap));
    ui->pbtn_preorder_confirm->setIconSize(ui->pbtn_preorder_confirm->size());
    ui->pbtn_preorder_confirm->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_preorder_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_preorder_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_preorder_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_preorder_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));

    // initialize preorder result page
    QPixmap preorder_result_bg_pixmap(resources_path + "P12_preorder_result.jpg");
    ui->lbl_preorder_result_bg->setPixmap(preorder_result_bg_pixmap);
    ui->lbl_preorder_open_hint->setText("");
    ui->lbl_preorder_open_hint->setFrameStyle(QFrame::NoFrame);
    ui->lbl_preorder_open_hint->setMovie(new QMovie(resources_path + "lbl_pickup_hint.gif"));
    ui->lbl_preorder_open_hint->movie()->start();
    ui->pbtn_preorder_open->setFlat(true);
    ui->pbtn_preorder_open->setText("");
    ui->pbtn_preorder_open->setIcon(QIcon(pickup_pixmap));
    ui->pbtn_preorder_open->setIconSize(ui->pbtn_preorder_open->size());
    ui->pbtn_preorder_open->setStyleSheet(transparent_stylesheet);
    ui->pbtn_preorder_finish->setFlat(true);
    ui->pbtn_preorder_finish->setText("");
    ui->pbtn_preorder_finish->setIcon(QIcon(finish_pixmap));
    ui->pbtn_preorder_finish->setIconSize(ui->pbtn_preorder_finish->size());
    ui->pbtn_preorder_finish->setStyleSheet(transparent_stylesheet);
    ui->lbl_preorder_result_hint->setText("");
    ui->lbl_preorder_result_hint->setFrameStyle(QFrame::NoFrame);
    ui->lbl_preorder_result_hint->setMovie(new QMovie(resources_path + "lbl_result_hint.gif"));
    ui->lbl_preorder_result_hint->movie()->start();
    connect(ui->pbtn_preorder_open, SIGNAL(pressed()), this, SLOT(pbtn_open_pressed()));
    connect(ui->pbtn_preorder_open, SIGNAL(released()), this, SLOT(pbtn_open_released()));
    connect(ui->pbtn_preorder_finish, SIGNAL(pressed()), this, SLOT(pbtn_finish_pressed()));
    connect(ui->pbtn_preorder_finish, SIGNAL(released()), this, SLOT(pbtn_finish_released()));

    // initialize EC pickup hint page
    QPixmap ec_pickup_hint_bg_pixmap(resources_path + "P13_ec_pickup_hint.jpg");
    ui->lbl_ec_pickup_hint_bg->setPixmap(ec_pickup_hint_bg_pixmap);
    ui->pbtn_ec_pickup_hint_back->setFlat(true);
    ui->pbtn_ec_pickup_hint_back->setText("");
    ui->pbtn_ec_pickup_hint_back->setIcon(QIcon(gback_pixmap));
    ui->pbtn_ec_pickup_hint_back->setIconSize(ui->pbtn_ec_pickup_hint_back->size());
    ui->pbtn_ec_pickup_hint_back->setStyleSheet(transparent_stylesheet);
    ui->pbtn_ec_pickup_hint_confirm->setFlat(true);
    ui->pbtn_ec_pickup_hint_confirm->setText("");
    ui->pbtn_ec_pickup_hint_confirm->setIcon(QIcon(gconfirm_pixmap));
    ui->pbtn_ec_pickup_hint_confirm->setIconSize(ui->pbtn_ec_pickup_hint_confirm->size());
    ui->pbtn_ec_pickup_hint_confirm->setStyleSheet(transparent_stylesheet);
    ui->lbl_ec_pickup_hint_arrow->setText("");
    ui->lbl_ec_pickup_hint_arrow->setFrameStyle(QFrame::NoFrame);
    ui->lbl_ec_pickup_hint_arrow->setMovie(new QMovie(resources_path + "lbl_scanner_arrow.gif"));
    ui->lbl_ec_pickup_hint_arrow->movie()->start();
    connect(ui->pbtn_ec_pickup_hint_back, SIGNAL(pressed()), this, SLOT(pbtn_back_pressed()));
    connect(ui->pbtn_ec_pickup_hint_back, SIGNAL(released()), this, SLOT(pbtn_back_released()));
    connect(ui->pbtn_ec_pickup_hint_confirm, SIGNAL(pressed()), this, SLOT(pbtn_confirm_pressed()));
    connect(ui->pbtn_ec_pickup_hint_confirm, SIGNAL(released()), this, SLOT(pbtn_confirm_released()));
    connect(ui->lineEdit_ec_pickup_code, SIGNAL(textChanged(QString)), this, SLOT(lineEdit_textChanged(QString)));

    // initialize EC pickup result page
    QPixmap ec_pickup_result_bg_pixmap(resources_path + "P14_ec_pickup_result.jpg");
    ui->lbl_ec_pickup_result_bg->setPixmap(ec_pickup_result_bg_pixmap);
    ui->lbl_ec_pickup_open_hint->setText("");
    ui->lbl_ec_pickup_open_hint->setFrameStyle(QFrame::NoFrame);
    ui->lbl_ec_pickup_open_hint->setMovie(new QMovie(resources_path + "lbl_pickup_hint.gif"));
    ui->lbl_ec_pickup_open_hint->movie()->start();
    QPixmap open_pixmap(resources_path + "btn_open.png");
    ui->pbtn_ec_pickup_open->setFlat(true);
    ui->pbtn_ec_pickup_open->setText("");
    ui->pbtn_ec_pickup_open->setIcon(QIcon(open_pixmap));
    ui->pbtn_ec_pickup_open->setIconSize(ui->pbtn_ec_pickup_open->size());
    ui->pbtn_ec_pickup_open->setStyleSheet(transparent_stylesheet);
    ui->pbtn_ec_pickup_finish->setFlat(true);
    ui->pbtn_ec_pickup_finish->setText("");
    QPixmap finish_ec_pixmap(resources_path + "btn_finish_ec.png");
    ui->pbtn_ec_pickup_finish->setIcon(QIcon(finish_ec_pixmap));
    ui->pbtn_ec_pickup_finish->setIconSize(ui->pbtn_ec_pickup_finish->size());
    ui->pbtn_ec_pickup_finish->setStyleSheet(transparent_stylesheet);
    ui->lbl_ec_pickup_result_hint->setText("");
    ui->lbl_ec_pickup_result_hint->setFrameStyle(QFrame::NoFrame);
    ui->lbl_ec_pickup_result_hint->setMovie(new QMovie(resources_path + "lbl_result_hint.gif"));
    ui->lbl_ec_pickup_result_hint->movie()->start();
    connect(ui->pbtn_ec_pickup_open, SIGNAL(pressed()), this, SLOT(pbtn_open_pressed()));
    connect(ui->pbtn_ec_pickup_open, SIGNAL(released()), this, SLOT(pbtn_open_released()));
    connect(ui->pbtn_ec_pickup_finish, SIGNAL(pressed()), this, SLOT(pbtn_finish_pressed()));
    connect(ui->pbtn_ec_pickup_finish, SIGNAL(released()), this, SLOT(pbtn_finish_released()));

    // initialize finish page
    QPixmap finish_bg_pixmap(resources_path + "P15_finish.jpg");
    ui->lbl_finish_bg->setPixmap(finish_bg_pixmap);
    QPixmap finish_hint_pixmap(resources_path + "lbl_thank_you.png");
    ui->lbl_finish_hint->setPixmap(finish_hint_pixmap);
    QPixmap home_pixmap(resources_path + "btn_home.png");
    ui->pbtn_home->setFlat(true);
    ui->pbtn_home->setText("");
    ui->pbtn_home->setIcon(QIcon(home_pixmap));
    ui->pbtn_home->setIconSize(ui->pbtn_home->size());
    ui->pbtn_home->setStyleSheet(transparent_stylesheet);
    connect(ui->pbtn_home, SIGNAL(pressed()), this, SLOT(pbtn_home_pressed()));
    connect(ui->pbtn_home, SIGNAL(released()), this, SLOT(pbtn_home_released()));

    // initialize table view
    cart_model_ = new QStandardItemModel(this);
    cart_model_->setColumnCount(5);
    cart_model_->setHorizontalHeaderItem(0, new QStandardItem("餐櫃編號"));
    cart_model_->setHorizontalHeaderItem(1, new QStandardItem("貨道編號"));
    cart_model_->setHorizontalHeaderItem(2, new QStandardItem("商品名稱及數量"));
    cart_model_->setHorizontalHeaderItem(3, new QStandardItem("小計"));
    cart_model_->setHorizontalHeaderItem(4, new QStandardItem("價錢"));
    cart_model_->setHorizontalHeaderItem(5, new QStandardItem("刪除"));

    ui->tView_onsite_option->setModel(cart_model_);
    ui->tView_onsite_option->setColumnWidth(0, 0);      // hide
    ui->tView_onsite_option->setColumnWidth(1, 0);      // hide
    ui->tView_onsite_option->setColumnWidth(2, 675);
    ui->tView_onsite_option->setColumnWidth(3, 55);
    ui->tView_onsite_option->setColumnWidth(4, 80);
    ui->tView_onsite_option->setColumnWidth(5, 40);
    ui->tView_onsite_option->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tView_onsite_checkout->setModel(cart_model_);
    ui->tView_onsite_checkout->setColumnWidth(0, 0);    // hide
    ui->tView_onsite_checkout->setColumnWidth(1, 0);    // hide
    ui->tView_onsite_checkout->setColumnWidth(2, 685);
    ui->tView_onsite_checkout->setColumnWidth(3, 55);
    ui->tView_onsite_checkout->setColumnWidth(4, 70);
    ui->tView_onsite_checkout->setColumnWidth(5, 0);    // hide
    ui->tView_onsite_checkout->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->tView_preorder_confirm->setModel(cart_model_);
    ui->tView_preorder_confirm->setColumnWidth(0, 0);   // hide
    ui->tView_preorder_confirm->setColumnWidth(1, 0);   // hide
    ui->tView_preorder_confirm->setColumnWidth(2, 685);
    ui->tView_preorder_confirm->setColumnWidth(3, 55);
    ui->tView_preorder_confirm->setColumnWidth(4, 70);
    ui->tView_preorder_confirm->setColumnWidth(5, 0);   // hide
    ui->tView_preorder_confirm->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // listen page changed
    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(page_changed(int)));
}

void MainWindow::initPort()
{
    port_hvmc_ = new VMController(this);
    port_hvmc_->setPortName("/dev/ttyACM0");
    connect(port_hvmc_, SIGNAL(timeoutWithState(QString, int)),
            this, SLOT(vmcTimeoutWithState(QString, int)));
    connect(port_hvmc_, SIGNAL(getFirmwareInfosResponse(QString)),
            this, SLOT(getVmcFirmwareInfosResponse(QString)));
    connect(port_hvmc_, SIGNAL(getTemperatureStatusResponse(QString)),
            this, SLOT(getVmcTemperatureStatusResponse(QString)));
    connect(port_hvmc_, SIGNAL(setCompressorSwitchResponse(bool)),
            this, SLOT(setVmcCompressorSwitchResponse(bool)));
    connect(port_hvmc_, SIGNAL(setDoorSwitchResponse(bool)),
            this, SLOT(setVmcDoorSwitchResponse(bool)));
    connect(port_hvmc_, SIGNAL(executeChannelResponse(bool, int)),
            this, SLOT(executeVmcChannelResponse(bool, int)));

    port_fvmc_ = new FVMController(this);
    port_fvmc_->setPortName("/dev/ttyS2");
    connect(port_fvmc_, SIGNAL(openDoorResponse(int, int, bool)),
            this, SLOT(fvmcOpenDoorResponse(int, int, bool)));
    connect(port_fvmc_, SIGNAL(readDoorStatusResponse(int, bool)),
            this, SLOT(fvmcReadDoorStatusResponse(int, bool)));

    port_cvmc_ = new CVMController(this);
    port_cvmc_->setPortName("/dev/ttyUSB0");
    connect(port_cvmc_, SIGNAL(readDoorStatusResponse()),
            this, SLOT(cvmcDoorStatusResponse()));

    port_card_ = new NcccEDC(this);
    port_card_->setPortName("/dev/ttyS1");
    connect(port_card_, SIGNAL(executeTransactionResponse(bool)),
            this, SLOT(executeCardTransactionResponse(bool)));
    connect(port_card_, SIGNAL(executeSettlementResponse(bool)),
            this, SLOT(executeCardSettlementResponse(bool)));

    port_scanner_ = new QSerialPort(this);
    port_scanner_->setPortName("/dev/ttyACM1");
    port_scanner_->setBaudRate(QSerialPort::Baud9600);
    port_scanner_->setDataBits(QSerialPort::Data8);
    port_scanner_->setParity(QSerialPort::NoParity);
    port_scanner_->setStopBits(QSerialPort::OneStop);
    port_scanner_->setFlowControl(QSerialPort::NoFlowControl);
    connect(port_scanner_, SIGNAL(readyRead()), this, SLOT(scannerDataReady()));

    port_printer_ = new PrinterKP247(this);
    port_printer_->setPortName("/dev/ttyUSB1");
}

void MainWindow::initMachine()
{
    // create machine data
    machine_info_ = new MachineInfo(this);
    machine_info_->machine_code = "HE01050";
}

void MainWindow::initLaneInfo()
{
    // initialize the information of bento
    lane_infos_bento_.clear();
    lane_infos_bento_.append(new LaneInfo(this));
    lane_infos_bento_.append(new LaneInfo(this));
    lane_infos_bento_.append(new LaneInfo(this));
    lane_infos_bento_.append(new LaneInfo(this));

    for (int i = 0; i < 4; i++) {
        LaneInfo *lane_info = lane_infos_bento_.at(i);
        lane_info->amount_remain = "1";
        if (i == 0) {
            lane_info->item_info.numbering = "1";
            lane_info->item_info.name = "一杯二杯三杯雞";
            lane_info->item_info.picture = "00001.jpg";
            lane_info->payment_1_price = "150";
        }
        else if (i == 1) {
            lane_info->item_info.numbering = "2";
            lane_info->item_info.name = "好想鯖鯖魚";
            lane_info->item_info.picture = "00002.jpg";
            lane_info->payment_1_price = "165";
        }
        else if (i == 2) {
            lane_info->item_info.numbering = "3";
            lane_info->item_info.name = "西遊排骨精";
            lane_info->item_info.picture = "00003.jpg";
            lane_info->payment_1_price = "130";
        }
        else if (i == 3) {
            lane_info->item_info.numbering = "4";
            lane_info->item_info.name = "就醬烤雞腿";
            lane_info->item_info.picture = "00004.jpg";
            lane_info->payment_1_price = "150";
        }
    }

    // initialize the information of drink
    lane_infos_drink_.clear();
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));
    lane_infos_drink_.append(new LaneInfo(this));

    for (int i = 0; i < 7; i++) {
        LaneInfo *lane_info = lane_infos_drink_.at(i);
        lane_info->amount_remain = "1";
        if (i == 0) {
            lane_info->item_info.numbering = "11";
            lane_info->item_info.name = "原味綜合蔬果汁";
            lane_info->item_info.picture = "2920423.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 1) {
            lane_info->item_info.numbering = "12";
            lane_info->item_info.name = "紫色綜合蔬果汁";
            lane_info->item_info.picture = "2920514.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 2) {
            lane_info->item_info.numbering = "13";
            lane_info->item_info.name = "胡蘿蔔綜合蔬果汁";
            lane_info->item_info.picture = "2921001.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 3) {
            lane_info->item_info.numbering = "14";
            lane_info->item_info.name = "鮮榨椪柑原汁";
            lane_info->item_info.picture = "2921015.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 4) {
            lane_info->item_info.numbering = "15";
            lane_info->item_info.name = "麥香紅茶";
            lane_info->item_info.picture = "3110067.png";
            lane_info->payment_1_price = "10";
        }
        else if (i == 5) {
            lane_info->item_info.numbering = "16";
            lane_info->item_info.name = "可樂ＣＡＮ";
            lane_info->item_info.picture = "3320056.png";
            lane_info->payment_1_price = "20";
        }
        else if (i == 6) {
            lane_info->item_info.numbering = "17";
            lane_info->item_info.name = "海鹽檸檬飲";
            lane_info->item_info.picture = "2920378.png";
            lane_info->payment_1_price = "35";
        }
    }

    // initialize the information of dessert
    lane_infos_dessert_.clear();
    lane_infos_dessert_.append(new LaneInfo(this));
    lane_infos_dessert_.append(new LaneInfo(this));
    lane_infos_dessert_.append(new LaneInfo(this));

    for (int i = 0; i < 3; i++) {
        LaneInfo *lane_info = lane_infos_dessert_.at(i);
        lane_info->amount_remain = "1";
        if (i == 0) {
            lane_info->item_info.numbering = "21";
            lane_info->item_info.name = "杏桃乳酪舒芙蕾";
            lane_info->item_info.picture = "00011.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 1) {
            lane_info->item_info.numbering = "22";
            lane_info->item_info.name = "杏桃乳酪麵包";
            lane_info->item_info.picture = "00012.png";
            lane_info->payment_1_price = "35";
        }
        else if (i == 2) {
            lane_info->item_info.numbering = "23";
            lane_info->item_info.name = "一配虎皮蛋糕捲";
            lane_info->item_info.picture = "1922935.png";
            lane_info->payment_1_price = "38";
        }
    }
}

void MainWindow::refreshCatalog()
{
    int remain_amount = 0;
    int row = 0;
    int col = 0;

    // update buttons from lane informations
    for (int i = 0; i < 16; i++) {
        row = i / 4;
        col = i % 4;

        ProductButton *pbtn = this->findChild<ProductButton *>(QString("pbtn_p%1%2").arg(row).arg(col));
        if (pbtn != nullptr) {

            if (i < lane_infos_editing_.count()) {
                LaneInfo *lane_info = lane_infos_editing_.at(i);
                remain_amount = lane_info->amount_remain.toInt();

                pbtn->setItemVisible(true);
                pbtn->setNumbering(lane_info->item_info.numbering.toInt());
                pbtn->setItemName(lane_info->item_info.name);
                pbtn->setItemImage(QString(dir_image) + lane_info->item_info.picture);
                pbtn->setItemPrice(lane_info->payment_1_price.toInt());

                // set button state when sold out
                pbtn->setItemEnabled((remain_amount > 0));
            }
            else {
                pbtn->setItemVisible(false);
            }
        }
    }

    if (cart_model_->rowCount() > 0) {
        ui->lbl_onsite_option_hint->hide();
        ui->pbtn_onsite_checkout->setEnabled(true);
    }
    else {
        ui->lbl_onsite_option_hint->show();
        ui->pbtn_onsite_checkout->setEnabled(false);
        ui->lbl_onsite_option_total->setText("0");
    }
}

void MainWindow::finish()
{

}

void MainWindow::page_changed(int index)
{
    current_page_ = (PageId)index;

    tmr_watch_operation_->stop();
    tmr_watch_door_status_->stop();

    if (current_page_ != PaymentHint) {
        tmr_watch_payment_->stop();
    }
    if (current_page_ != EinvoiceHint &&
        current_page_ != PreOrderHint &&
        current_page_ != ECPickupHint) {
        tmr_watch_scanner_->stop();
    }

    switch (current_page_) {
    case Home:
        qDebug() << "[PAGE] to \"Home\"";
        initLaneInfo();
        ui->pbtn_onsite_bento->setChecked(true);
        lane_infos_editing_ = lane_infos_bento_;
        refreshCatalog();
        cart_model_->removeRows(0, cart_model_->rowCount());
        ui->lbl_onsite_option_hint->show();
        ui->lbl_onsite_option_total->setText("0");
        ui->pbtn_onsite_checkout->setEnabled(false);
        tmr_watch_pulling_->start();
        break;
    case OnSiteOption:
        qDebug() << "[PAGE] to \"OnSiteOption\"";
        break;
    case OnSiteConfirm:
        qDebug() << "[PAGE] to \"OnSiteConfirm\"";
        ui->pbtn_product_close->setText("關閉(60)");
        break;
    case OnSiteCart:
        qDebug() << "[PAGE] to \"OnSiteCart\"";
        break;
    case OnSiteCheckout:
        qDebug() << "[PAGE] to \"OnSiteCheckout\"";
        break;
    case PaymentOption:
        qDebug() << "[PAGE] to \"PaymentOption\"";
        break;
    case PaymentHint:
        qDebug() << "[PAGE] to \"PaymentHint\"";
        if (payment_type_ == FamiPay) {
            ui->pbtn_payment_hint_back->setText("返回(10)");
            ui->pbtn_payment_hint_back->setEnabled(true);
        }
        else {
            ui->pbtn_payment_hint_back->setText("   (10)");
            ui->pbtn_payment_hint_back->setEnabled(false);
        }
        counter_watch_payment_ = 10;
        tmr_watch_payment_->start();
        break;
    case EinvoiceOption:
        qDebug() << "[PAGE] to \"EinvoiceOption\"";
        ui->lineEdit_einvoice_code->clear();
        break;
    case EinvoiceHint:
        qDebug() << "[PAGE] to \"EinvoiceHint\"";
        ui->pbtn_einvoice_hint_back->setFocus();
        if (einvoice_type_ != Print) {
            counter_watch_scanner_ = 60;
            tmr_watch_scanner_->start();
        }
        break;
    case OnSiteResult:
        qDebug() << "[PAGE] to \"OnSiteResult\"";
        if (tmr_watch_pulling_->isActive()) {
            tmr_watch_pulling_->stop();
        }
        ui->lbl_onsite_result_hint->hide();
        ui->lbl_onsite_result_door_hint->hide();
        break;
    case PreOrderHint:
        qDebug() << "[PAGE] to \"PreOrderHint\"";
        ui->pbtn_preorder_hint_back->setFocus();
        counter_watch_scanner_ = 60;
        tmr_watch_scanner_->start();
        break;
    case PreOrderConfirm:
        qDebug() << "[PAGE] to \"PreOrderConfirm\"";
        break;
    case PreOrderResult:
        qDebug() << "[PAGE] to \"PreOrderResult\"";
        if (tmr_watch_pulling_->isActive()) {
            tmr_watch_pulling_->stop();
        }
        ui->lbl_preorder_result_hint->hide();
        ui->lbl_preorder_result_door_hint->hide();
        break;
    case ECPickupHint:
        qDebug() << "[PAGE] switch to \"ECPickupHint\"";
        ui->pbtn_ec_pickup_hint_back->setFocus();
        counter_watch_scanner_ = 60;
        tmr_watch_scanner_->start();
        break;
    case ECPickupResult:
        qDebug() << "[PAGE] switch to \"ECPickupResult\"";
        if (tmr_watch_pulling_->isActive()) {
            tmr_watch_pulling_->stop();
        }
        ui->lbl_ec_pickup_result_hint->hide();
        ui->lbl_ec_pickup_result_door_hint->hide();
        break;
    case Finish:
        qDebug() << "[PAGE] switch to \"Finish\"";
        break;
    case Test:
        qDebug() << "[PAGE] switch to \"Test\"";
        if (tmr_watch_pulling_->isActive()) {
            tmr_watch_pulling_->stop();
        }
        ui->cbBox_port->clear();
        foreach (QSerialPortInfo port_info, QSerialPortInfo::availablePorts()) {
            ui->cbBox_port->addItem(QString("%1(%2)").arg(port_info.portName()).arg(port_info.description()));
        }
        ui->cbBox_port->setCurrentIndex(0);
        break;
    }

    if (current_page_ == Home ||
        current_page_ == OnSiteConfirm ||
        current_page_ == PaymentHint ||
        current_page_ == EinvoiceHint ||
        current_page_ == PreOrderHint ||
        current_page_ == ECPickupHint) {
        ui->lbl_brand_upper->hide();
    }
    else {
        ui->lbl_brand_upper->show();
    }

    if (current_page_ == OnSiteOption ||
        current_page_ == OnSiteCheckout ||
        current_page_ == PaymentOption ||
        current_page_ == EinvoiceOption ||
        current_page_ == Test) {
        ui->pbtn_home_upper->show();
    }
    else {
        ui->pbtn_home_upper->hide();
    }

    if (current_page_ == Home ||
        current_page_ == PaymentOption ||
        current_page_ == EinvoiceOption) {
        ui->videoWidget->show();
        ui->lbl_ad_display->show();
    }
    else {
        ui->videoWidget->hide();
        ui->lbl_ad_display->hide();
    }

    if (current_page_ == PreOrderHint ||
        current_page_ == ECPickupHint) {
        ui->keyboard_input->show();
    }
    else {
        ui->keyboard_input->hide();
    }

    if (current_page_ != Home &&
        current_page_ != PaymentHint &&
        current_page_ != EinvoiceHint &&
        current_page_ != OnSiteResult &&
        current_page_ != PreOrderHint &&
        current_page_ != PreOrderResult &&
        current_page_ != ECPickupHint &&
        current_page_ != ECPickupResult &&
        current_page_ != Test) {
        counter_watch_operation_ = 60;
        tmr_watch_operation_->start();
    }
}

void MainWindow::pbtn_onsite_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_onsite_ov.png");
    ui->pbtn_onsite->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_onsite_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_onsite.png");
    ui->pbtn_onsite->setIcon(QIcon(pixmap));

    cart_model_->removeRows(0, cart_model_->rowCount());

    ui->stackedWidget->setCurrentIndex(OnSiteOption);
}

void MainWindow::pbtn_preorder_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_preorder_ov.png");
    ui->pbtn_preorder->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_preorder_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_preorder.png");
    ui->pbtn_preorder->setIcon(QIcon(pixmap));

    ui->lineEdit_preorder_code->clear();
    ui->pbtn_preorder_hint_confirm->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(PreOrderHint);
}

void MainWindow::pbtn_ec_pickup_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_ec_pickup_ov.png");
    ui->pbtn_ec_pickup->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_ec_pickup_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_ec_pickup.png");
    ui->pbtn_ec_pickup->setIcon(QIcon(pixmap));

    ui->lineEdit_ec_pickup_code->clear();
    ui->pbtn_ec_pickup_hint_confirm->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(ECPickupHint);
}

void MainWindow::pbtn_home_upper_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        QPixmap pixmap(QString(dir_resources) + "btn_home_upper_ov.png");
        pbtn->setIcon(QIcon(pixmap));
    }
}

void MainWindow::pbtn_home_upper_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        QPixmap pixmap(QString(dir_resources) + "btn_home_upper.png");
        pbtn->setIcon(QIcon(pixmap));

        ui->stackedWidget->setCurrentIndex(Home);
    }
}

void MainWindow::pbtn_home_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        QPixmap pixmap(QString(dir_resources) + "btn_home_ov.png");
        pbtn->setIcon(QIcon(pixmap));
    }
}

void MainWindow::pbtn_home_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        QPixmap pixmap(QString(dir_resources) + "btn_home.png");
        pbtn->setIcon(QIcon(pixmap));

        ui->stackedWidget->setCurrentIndex(Home);
    }
}

void MainWindow::pbtn_back_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_einvoice_hint_back ||
            pbtn == ui->pbtn_preorder_hint_back ||
            pbtn == ui->pbtn_ec_pickup_hint_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_gback_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else {
            QPixmap pixmap(QString(dir_resources) + "btn_back_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_back_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_product_close) {
            ui->stackedWidget->setCurrentIndex(OnSiteOption);
        }
        else if (pbtn == ui->pbtn_checkout_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_back.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(OnSiteOption);
        }
        else if (pbtn == ui->pbtn_payment_option_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_back.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(OnSiteCheckout);
        }
        else if (pbtn == ui->pbtn_payment_hint_back) {
            ui->stackedWidget->setCurrentIndex(PaymentOption);
        }
        else if (pbtn == ui->pbtn_einvoice_option_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_back.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(PaymentOption);
        }
        else if (pbtn == ui->pbtn_einvoice_hint_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_gback.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(EinvoiceOption);
        }
        else if (pbtn == ui->pbtn_preorder_hint_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_gback.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(Home);
        }
        else if (pbtn == ui->pbtn_preorder_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_back.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->lineEdit_preorder_code->clear();
            ui->pbtn_preorder_hint_confirm->setEnabled(false);
            ui->stackedWidget->setCurrentIndex(PreOrderHint);
        }
        else if (pbtn == ui->pbtn_ec_pickup_hint_back) {
            QPixmap pixmap(QString(dir_resources) + "btn_gback.png");
            pbtn->setIcon(QIcon(pixmap));
            ui->stackedWidget->setCurrentIndex(Home);
        }
    }
}

void MainWindow::pbtn_confirm_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_product_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_add_into_cart_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else if (pbtn == ui->pbtn_einvoice_hint_confirm ||
                 pbtn == ui->pbtn_preorder_hint_confirm ||
                 pbtn == ui->pbtn_ec_pickup_hint_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_gconfirm_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else {
            QPixmap pixmap(QString(dir_resources) + "btn_confirm_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_confirm_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_product_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_add_into_cart.png");
            pbtn->setIcon(QIcon(pixmap));

            if (pbtn_product_editing_ != nullptr) {
                int pdt_numbering = pbtn_product_editing_->numbering();

                QList<QStandardItem*> items;
                items.append(new QStandardItem(""));
                items.append(new QStandardItem(QString("%1").arg(pdt_numbering)));
                items.append(new QStandardItem(QString("%1 *%2").arg(ui->lbl_pdt_name->text()).arg(1)));
                items.append(new QStandardItem("NT."));
                items.append(new QStandardItem(ui->lbl_pdt_total_amount->text().mid(4)));
                items.append(new QStandardItem(""));
                cart_model_->appendRow(items);

                items.at(3)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                items.at(4)->setTextAlignment(Qt::AlignCenter);

                int row = cart_model_->rowCount() - 1;
                QPixmap pixmap(QString(dir_resources) + "btn_delete.png");
                cart_model_->setData(cart_model_->index(row, 5), pixmap, Qt::DecorationRole);

                for (int i = 0; i < lane_infos_editing_.count(); i++) {
                    LaneInfo *lane_info = lane_infos_editing_.at(i);
                    int lane_num = lane_info->item_info.numbering.toInt();
                    if (pdt_numbering == lane_num) {
                        int original_remain = lane_info->amount_remain.toInt();
                        int original_total_amount = ui->lbl_onsite_option_total->text().toInt();
                        lane_info->amount_remain = QString("%1").arg(original_remain - 1);
                        ui->lbl_onsite_option_total->setText(QString("%1").arg(original_total_amount + lane_info->payment_1_price.toInt()));
                        break;
                    }
                }
            }
            refreshCatalog();
            ui->stackedWidget->setCurrentIndex(OnSiteOption);
        }
        else if (pbtn == ui->pbtn_checkout_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_confirm.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_payment_total->setText(ui->lbl_checkout_total->text());
            ui->stackedWidget->setCurrentIndex(PaymentOption);
        }
        else if (pbtn == ui->pbtn_preorder_hint_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_gconfirm.png");
            pbtn->setIcon(QIcon(pixmap));

            cart_model_->removeRows(0, cart_model_->rowCount());

            QList<QStandardItem*> items;
            items.append(new QStandardItem("B"));
            items.append(new QStandardItem(11));
            items.append(new QStandardItem(QString("%1 *%2").arg("就醬烤雞腿").arg(1)));
            items.append(new QStandardItem("NT."));
            items.append(new QStandardItem("150"));
            items.append(new QStandardItem(""));
            cart_model_->appendRow(items);

            items.at(3)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            items.at(4)->setTextAlignment(Qt::AlignCenter);

            ui->stackedWidget->setCurrentIndex(PreOrderConfirm);
        }
        else if (pbtn == ui->pbtn_einvoice_hint_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_gconfirm.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->stackedWidget->setCurrentIndex(PaymentHint);
        }
        else if (pbtn == ui->pbtn_preorder_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_confirm.png");
            pbtn->setIcon(QIcon(pixmap));

            is_dessert_ = false;
            ui->lbl_preorder_open_hint->show();
            ui->pbtn_preorder_open->setEnabled(true);
            ui->pbtn_preorder_finish->setEnabled(false);
            ui->stackedWidget->setCurrentIndex(PreOrderResult);
        }
        else if (pbtn == ui->pbtn_ec_pickup_hint_confirm) {
            QPixmap pixmap(QString(dir_resources) + "btn_gconfirm.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_ec_pickup_open_hint->show();
            ui->pbtn_ec_pickup_open->setEnabled(true);
            ui->pbtn_ec_pickup_finish->setEnabled(false);
            ui->stackedWidget->setCurrentIndex(ECPickupResult);
        }
    }
}

void MainWindow::pbtn_open_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_ec_pickup_open) {
            QPixmap pixmap(QString(dir_resources) + "btn_open_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else {
            QPixmap pixmap(QString(dir_resources) + "btn_pickup_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_open_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_onsite_open_A) {
            QPixmap pixmap(QString(dir_resources) + "btn_pickup.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_onsite_open_hint_A->hide();
            if (port_fvmc_ != nullptr) {
                port_fvmc_->openDoor(1, 10);
                port_fvmc_->openDoor(1, 15);
                tmr_watch_door_status_->start();
            }
        }
        else if (pbtn == ui->pbtn_onsite_open_B) {
            QPixmap pixmap(QString(dir_resources) + "btn_pickup.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_onsite_open_hint_A->hide();
            if (port_fvmc_ != nullptr) {
                port_fvmc_->openDoor(0, 18);
                port_fvmc_->openDoor(0, 23);
                tmr_watch_door_status_->start();
            }
        }
        else if (pbtn == ui->pbtn_onsite_open_D) {
            QPixmap pixmap(QString(dir_resources) + "btn_pickup.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_onsite_open_hint_D->hide();
            if (port_hvmc_ != nullptr) {
                port_hvmc_->executeChannelRetry();

                counter_watch_vmc_ = 0;
                tmr_watch_vmc_->start();
            }
        }
        else if (pbtn == ui->pbtn_preorder_open) {
            QPixmap pixmap(QString(dir_resources) + "btn_pickup.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_preorder_open_hint->hide();
            if (port_fvmc_ != nullptr) {
                port_fvmc_->openDoor(0, 11);
                tmr_watch_door_status_->start();
            }
        }
        else if (pbtn == ui->pbtn_ec_pickup_open) {
            QPixmap pixmap(QString(dir_resources) + "btn_open.png");
            pbtn->setIcon(QIcon(pixmap));

            ui->lbl_ec_pickup_open_hint->hide();
            if (port_cvmc_ != nullptr) {
                port_cvmc_->openDoor(0, 7);
                //tmr_watch_door_status_->start();
                ui->pbtn_ec_pickup_finish->setEnabled(true);
            }
        }
    }
}

void MainWindow::pbtn_finish_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_ec_pickup_finish) {
            QPixmap pixmap(QString(dir_resources) + "btn_finish_ec_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else {
            QPixmap pixmap(QString(dir_resources) + "btn_finish_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_finish_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {

        if (pbtn == ui->pbtn_ec_pickup_finish) {
            QPixmap pixmap(QString(dir_resources) + "btn_finish_ec.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else {
            QPixmap pixmap(QString(dir_resources) + "btn_finish.png");
            pbtn->setIcon(QIcon(pixmap));
        }

        ui->stackedWidget->setCurrentIndex(Finish);

        if (port_printer_->isOpen() == false) {
            port_printer_->open(QIODevice::ReadWrite);

            if (port_printer_->isOpen() == false) {
                qDebug() << "[PRINTER] open failed";
                return;
            }
        }

        QByteArray data_ba;
        if (cart_model_->rowCount() > 0) {

            int total_amount = 0;

            // print title of receipt
            port_printer_->setDefault();
            port_printer_->setTextFormat(false, true, true, true, false);
            port_printer_->setTextAlignment(1);
            //port_printer_->printText("FamilyMart");
            port_printer_->printText("交易明細");

            // print empty and line
            port_printer_->setDefault();
            port_printer_->setTextFormat(false, false, false, false, false);
            port_printer_->setTextAlignment(1);
            port_printer_->printText("\r\n-----------------------");

            // print product details(name, quantity, amount)
            port_printer_->setDefault();
            port_printer_->setTextFormat(false, false, false, false, false);
            port_printer_->setTextAlignment(0);
            data_ba.clear();
            data_ba.append("品名             數量     金額\r\n");
            for (int i = 0; i < cart_model_->rowCount(); i++) {
                QString temp = cart_model_->item(i, 2)->text();
                QString name = temp.left(temp.length() - 2);
                QString price = cart_model_->item(i, 4)->text();
                data_ba.append(name);
                data_ba.append(QString("%1").arg(QLatin1Char('1'), (10 - name.length()) * 2 + 1));
                data_ba.append(QString("%1T").arg(price, 9));
                data_ba.append("\r\n");
                total_amount += price.toInt();
            }
            data_ba.append(QString("\r\n總金額%1T\r\n").arg(total_amount, 23));
            port_printer_->printText(QString::fromUtf8(data_ba));
        }
        else {
            // print title of receipt
            port_printer_->setDefault();
            port_printer_->setTextFormat(false, true, true, true, false);
            port_printer_->setTextAlignment(1);
            port_printer_->printText("FamilyMart");
            port_printer_->printText("EC取件收執聯");
        }

        // print empty line
        port_printer_->setDefault();
        port_printer_->setTextFormat(false, false, true, true, false);
        port_printer_->setTextAlignment(1);
        port_printer_->printText(" ");

        // cut paper
        port_printer_->cut(66);
    }
}

void MainWindow::pbtn_onsite_option_toggled(bool checked)
{
    counter_watch_operation_ = 60;

    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_onsite_bento) {
            if (checked) {
                QPixmap pixmap(QString(dir_resources) + "btn_bento_checked.png");
                pbtn->setIcon(QIcon(pixmap));

                lane_infos_editing_ = lane_infos_bento_;
                refreshCatalog();

                ui->pbtn_onsite_prev->setEnabled(false);
                ui->pbtn_onsite_next->setEnabled(false);
            }
            else {
                QPixmap pixmap(QString(dir_resources) + "btn_bento.png");
                pbtn->setIcon(QIcon(pixmap));
            }
        }
        else if (pbtn == ui->pbtn_onsite_drink) {
            if (checked) {
                QPixmap pixmap(QString(dir_resources) + "btn_drink_checked.png");
                pbtn->setIcon(QIcon(pixmap));

                lane_infos_editing_ = lane_infos_drink_;
                refreshCatalog();

                ui->pbtn_onsite_prev->setEnabled(false);
                ui->pbtn_onsite_next->setEnabled(false);
            }
            else {
                QPixmap pixmap(QString(dir_resources) + "btn_drink.png");
                pbtn->setIcon(QIcon(pixmap));
            }
        }
        else if (pbtn == ui->pbtn_onsite_dessert) {
            if (checked) {
                QPixmap pixmap(QString(dir_resources) + "btn_dessert_checked.png");
                pbtn->setIcon(QIcon(pixmap));

                lane_infos_editing_ = lane_infos_dessert_;
                refreshCatalog();

                ui->pbtn_onsite_prev->setEnabled(false);
                ui->pbtn_onsite_next->setEnabled(false);
            }
            else {
                QPixmap pixmap(QString(dir_resources) + "btn_dessert.png");
                pbtn->setIcon(QIcon(pixmap));
            }
        }
    }
}

void MainWindow::pbtn_onsite_product_released()
{
    ProductButton* pbtn = (ProductButton*)QObject::sender();
    if (pbtn != nullptr) {
        // update product display
        ui->lbl_pdt_name->setText(pbtn->itemName());
        ui->lbl_pdt_price->setText(QString("NT. %1").arg(pbtn->itemPrice()));
        ui->lbl_pdt_total_amount->setText(QString("NT. %1").arg(pbtn->itemPrice()));
        QPixmap pixmap(pbtn->itemImage());
        pixmap = pixmap.scaled(ui->lbl_pdt_image->size(), Qt::KeepAspectRatio);
        ui->lbl_pdt_image->setPixmap(pixmap);

        pbtn_product_editing_ = pbtn;
        ui->stackedWidget->setCurrentIndex(OnSiteConfirm);
    }
}

void MainWindow::pbtn_onsite_prev_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_prev_ov.png");
    ui->pbtn_onsite_prev->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_onsite_prev_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_prev.png");
    ui->pbtn_onsite_prev->setIcon(QIcon(pixmap));

    ui->pbtn_onsite_prev->setEnabled(false);
    ui->pbtn_onsite_next->setEnabled(true);
}

void MainWindow::pbtn_onsite_next_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_next_ov.png");
    ui->pbtn_onsite_next->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_onsite_next_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_next.png");
    ui->pbtn_onsite_next->setIcon(QIcon(pixmap));

    ui->pbtn_onsite_prev->setEnabled(true);
    ui->pbtn_onsite_next->setEnabled(false);
}

void MainWindow::pbtn_onsite_checkout_pressed()
{
    QPixmap pixmap(QString(dir_resources) + "btn_checkout_ov.png");
    ui->pbtn_onsite_checkout->setIcon(QIcon(pixmap));
}

void MainWindow::pbtn_onsite_checkout_released()
{
    QPixmap pixmap(QString(dir_resources) + "btn_checkout.png");
    ui->pbtn_onsite_checkout->setIcon(QIcon(pixmap));

    ui->lbl_checkout_total->setText("NT. " + ui->lbl_onsite_option_total->text());
    ui->stackedWidget->setCurrentIndex(OnSiteCheckout);
}

void MainWindow::pbtn_payment_option_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_payment_famiPay) {
            QPixmap pixmap(QString(dir_resources) + "btn_famiPay_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else if (pbtn == ui->pbtn_payment_creditcard) {
            QPixmap pixmap(QString(dir_resources) + "btn_creditcard_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else if (pbtn == ui->pbtn_payment_easycard) {
            QPixmap pixmap(QString(dir_resources) + "btn_easycard_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_payment_option_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_payment_famiPay) {
            QPixmap pixmap(QString(dir_resources) + "btn_famiPay.png");
            pbtn->setIcon(QIcon(pixmap));

            payment_type_ = FamiPay;
            ui->lbl_payment_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P6_payment_hint_famiPay.jpg"));
            ui->stackedWidget->setCurrentIndex(PaymentHint);
        }
        else if (pbtn == ui->pbtn_payment_creditcard) {
            QPixmap pixmap(QString(dir_resources) + "btn_creditcard.png");
            pbtn->setIcon(QIcon(pixmap));

            payment_type_ = CreditCard;
            ui->lbl_payment_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P6_payment_hint_card.jpg"));
            ui->stackedWidget->setCurrentIndex(EinvoiceOption);
        }
        else if (pbtn == ui->pbtn_payment_easycard) {
            QPixmap pixmap(QString(dir_resources) + "btn_easycard.png");
            pbtn->setIcon(QIcon(pixmap));

            payment_type_ = EasyCard;
            ui->lbl_payment_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P6_payment_hint_card.jpg"));
            ui->stackedWidget->setCurrentIndex(EinvoiceOption);
        }
    }
}

void MainWindow::pbtn_einvoice_option_pressed()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_einvoice_carrier) {
            QPixmap pixmap(QString(dir_resources) + "btn_carrier_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else if (pbtn == ui->pbtn_einvoice_lovecode) {
            QPixmap pixmap(QString(dir_resources) + "btn_lovecode_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
        else if (pbtn == ui->pbtn_einvoice_print) {
            QPixmap pixmap(QString(dir_resources) + "btn_print_ov.png");
            pbtn->setIcon(QIcon(pixmap));
        }
    }
}

void MainWindow::pbtn_einvoice_option_released()
{
    QPushButton *pbtn = (QPushButton *)QObject::sender();
    if (pbtn != nullptr) {
        if (pbtn == ui->pbtn_einvoice_carrier) {
            QPixmap pixmap(QString(dir_resources) + "btn_carrier.png");
            pbtn->setIcon(QIcon(pixmap));

            einvoice_type_ = Carrier;
            ui->lbl_einvoice_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P8_einvoice_hint_carrier.jpg"));
            ui->lineEdit_einvoice_code->hide();
            ui->lbl_einvoice_lovecode->hide();
            ui->line_einvoice_hint->hide();
            ui->num_input->hide();
            ui->lbl_einvoice_carrier->clear();
            ui->lbl_einvoice_carrier->show();
            ui->lbl_einvoice_hint_arrow->show();
            ui->lbl_einvoice_hint_arrow->setGeometry(0, 1260, 1080, 128);
            ui->pbtn_einvoice_hint_back->setGeometry(278, 1112, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setGeometry(556, 1112, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setEnabled(false);
        }
        else if (pbtn == ui->pbtn_einvoice_lovecode) {
            QPixmap pixmap(QString(dir_resources) + "btn_lovecode.png");
            pbtn->setIcon(QIcon(pixmap));

            einvoice_type_ = LoveCode;
            ui->lbl_einvoice_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P8_einvoice_hint_loveCode.jpg"));
            ui->lineEdit_einvoice_code->show();
            ui->lineEdit_einvoice_code->setGeometry(270, 580, 540, 80);
            ui->lineEdit_einvoice_code->setPlaceholderText("請掃描或輸入愛心碼");
            ui->lbl_einvoice_lovecode->show();
            ui->lbl_einvoice_lovecode->clear();
            ui->line_einvoice_hint->show();
            ui->line_einvoice_hint->setGeometry(270, 655, 540, 20);
            ui->num_input->show();
            ui->num_input->setGeometry(300, 700, 480, 440);
            ui->lbl_einvoice_carrier->hide();
            ui->lbl_einvoice_hint_arrow->show();
            ui->lbl_einvoice_hint_arrow->setGeometry(0, 1330, 1080, 128);
            ui->pbtn_einvoice_hint_back->setGeometry(278, 1178, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setGeometry(556, 1178, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setEnabled(false);
        }
        else if (pbtn == ui->pbtn_einvoice_print) {
            QPixmap pixmap(QString(dir_resources) + "btn_print.png");
            pbtn->setIcon(QIcon(pixmap));

            einvoice_type_ = Print;
            ui->lbl_einvoice_hint_bg->setPixmap(QPixmap(QString(dir_resources) + "P8_einvoice_hint_print.jpg"));
            ui->lineEdit_einvoice_code->show();
            ui->lineEdit_einvoice_code->setGeometry(270, 640, 540, 80);
            ui->lineEdit_einvoice_code->setPlaceholderText("請輸入統一編號");
            ui->lbl_einvoice_lovecode->hide();
            ui->line_einvoice_hint->show();
            ui->line_einvoice_hint->setGeometry(270, 715, 540, 20);
            ui->num_input->show();
            ui->num_input->setGeometry(300, 760, 480, 440);
            ui->lbl_einvoice_carrier->hide();
            ui->lbl_einvoice_hint_arrow->hide();
            ui->pbtn_einvoice_hint_back->setGeometry(278, 1242, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setGeometry(556, 1242, 246, 127);
            ui->pbtn_einvoice_hint_confirm->setEnabled(true);
        }
        can_input_ = true;
        ui->stackedWidget->setCurrentIndex(EinvoiceHint);
    }
}

void MainWindow::tableView_clicked(const QModelIndex &index)
{
    bool is_found = false;

    if (index.isValid()) {
        if (index.column() == 5) {  // delete button
            int row = index.row();
            int numbering = cart_model_->data(cart_model_->index(row, 1)).toInt();

            cart_model_->removeRow(row);

            for (int i = 0; i < lane_infos_bento_.count() && (is_found == false); i++) {
                LaneInfo *lane_info = lane_infos_bento_.at(i);
                int lane_num = lane_info->item_info.numbering.toInt();
                if (numbering == lane_num) {
                    int original_remain = lane_info->amount_remain.toInt();
                    int original_total_amount = ui->lbl_onsite_option_total->text().toInt();
                    lane_info->amount_remain = QString("%1").arg(original_remain + 1);
                    ui->lbl_onsite_option_total->setText(QString("%1").arg(original_total_amount - lane_info->payment_1_price.toInt()));
                    is_found = true;
                    break;
                }
            }
            for (int i = 0; i < lane_infos_drink_.count() && (is_found == false); i++) {
                LaneInfo *lane_info = lane_infos_drink_.at(i);
                int lane_num = lane_info->item_info.numbering.toInt();
                if (numbering == lane_num) {
                    int original_remain = lane_info->amount_remain.toInt();
                    int original_total_amount = ui->lbl_onsite_option_total->text().toInt();
                    lane_info->amount_remain = QString("%1").arg(original_remain + 1);
                    ui->lbl_onsite_option_total->setText(QString("%1").arg(original_total_amount - lane_info->payment_1_price.toInt()));
                    is_found = true;
                    break;
                }
            }
            for (int i = 0; i < lane_infos_dessert_.count() && (is_found == false); i++) {
                LaneInfo *lane_info = lane_infos_dessert_.at(i);
                int lane_num = lane_info->item_info.numbering.toInt();
                if (numbering == lane_num) {
                    int original_remain = lane_info->amount_remain.toInt();
                    int original_total_amount = ui->lbl_onsite_option_total->text().toInt();
                    lane_info->amount_remain = QString("%1").arg(original_remain + 1);
                    ui->lbl_onsite_option_total->setText(QString("%1").arg(original_total_amount - lane_info->payment_1_price.toInt()));
                    break;
                }
            }
            refreshCatalog();
        }
    }
}

void MainWindow::lineEdit_textChanged(QString text)
{
    QLineEdit *edit = (QLineEdit *)QObject::sender();
    if (edit != nullptr) {
        if (edit == ui->lineEdit_einvoice_code) {
            if (einvoice_type_ == Carrier) {
                ui->pbtn_einvoice_hint_confirm->setEnabled(true);
            }
            else if (einvoice_type_ == LoveCode) {
                if (text.isEmpty()) {
                    ui->lbl_einvoice_lovecode->setText("");
                    ui->pbtn_einvoice_hint_confirm->setEnabled(false);
                    ui->lineEdit_einvoice_code->setFont(QFont("Noto Sans CJK TC Regular", 24));
                }
                else {
                    ui->lineEdit_einvoice_code->setFont(QFont("Arial", 40));
                    can_input_ = false;

                    QByteArray infos;
                    if (cms_api_->queryLoveCode(machine_info_->machine_code, text, &infos)) {
                        ui->lbl_einvoice_lovecode->setText(QString::fromUtf8(infos));
                        ui->pbtn_einvoice_hint_confirm->setEnabled(true);
                    }
                    else {
                        ui->lbl_einvoice_lovecode->setText("");
                        ui->pbtn_einvoice_hint_confirm->setEnabled(false);
                    }
                    can_input_ = true;
                }
            }
            else if (einvoice_type_ == Print) {
                 if (text.isEmpty() == false) {
                     einvoice_type_ = BanPrint;
                     ui->lineEdit_einvoice_code->setFont(QFont("Arial", 40));
                 }
            }
            else if (einvoice_type_ == BanPrint) {
                if (text.isEmpty()) {
                    einvoice_type_ = Print;
                    ui->lineEdit_einvoice_code->setFont(QFont("Noto Sans CJK TC Regular", 24));
                }
            }
        }
        else if (edit == ui->lineEdit_preorder_code) {
            if (text.isEmpty()) {
                ui->lineEdit_preorder_code->setFont(QFont("Noto Sans CJK TC Regular", 24));
            }
            else {
                ui->lineEdit_preorder_code->setFont(QFont("Arial", 40));
            }
            ui->pbtn_preorder_hint_confirm->setEnabled(!(text.isEmpty()));
        }
        else if (edit == ui->lineEdit_ec_pickup_code) {
            if (text.isEmpty()) {
                ui->lineEdit_ec_pickup_code->setFont(QFont("Noto Sans CJK TC Regular", 24));
            }
            else {
                ui->lineEdit_ec_pickup_code->setFont(QFont("Arial", 40));
            }
            ui->pbtn_ec_pickup_hint_confirm->setEnabled(!(text.isEmpty()));
        }
    }
}

void MainWindow::num_input_num_clicked(QString number)
{
    if (can_input_) {
        QString old_text = ui->lineEdit_einvoice_code->text();
        ui->lineEdit_einvoice_code->setText(old_text + number);
    }
}

void MainWindow::num_input_clear_clicked()
{
    ui->lineEdit_einvoice_code->clear();
    ui->lineEdit_einvoice_code->setFont(QFont("Noto Sans CJK TC Regular", 28));
}

void MainWindow::num_input_backspace_clicked()
{
    QString old_text = ui->lineEdit_einvoice_code->text();
    if (old_text.isEmpty() == false)
        ui->lineEdit_einvoice_code->setText(old_text.left(old_text.length() - 1));
}

void MainWindow::keyboard_char_clicked(QString character)
{
    if (current_page_ == PreOrderHint) {
        QString old_text = ui->lineEdit_preorder_code->text();
        ui->lineEdit_preorder_code->setText(old_text + character);
    }
    else if (current_page_ == ECPickupHint) {
        QString old_text = ui->lineEdit_ec_pickup_code->text();
        ui->lineEdit_ec_pickup_code->setText(old_text + character);
    }
}

void MainWindow::keyboard_backspace_clicked()
{
    if (current_page_ == PreOrderHint) {
        QString old_text = ui->lineEdit_preorder_code->text();
        ui->lineEdit_preorder_code->setText(old_text.left(old_text.length() - 1));
    }
    else if (current_page_ == ECPickupHint) {
        QString old_text = ui->lineEdit_ec_pickup_code->text();
        ui->lineEdit_ec_pickup_code->setText(old_text.left(old_text.length() - 1));
    }
}

void MainWindow::watch_pulling()
{
    QByteArray data_ba;
    if (cms_api_->getRemoteCommand(machine_info_->machine_code, &data_ba)) {
        // parser command
        QJsonDocument json_data = QJsonDocument::fromJson(data_ba);
        QJsonObject json_obj = json_data.object();
        QString cmd = json_obj.value("cmd_code").toString();

        qDebug() << "[CMS] pulling..." << cmd;

        if (cmd == "87") {
            if (port_fvmc_ != nullptr) {
                port_fvmc_->openDoor(0, 11);
            }
        }
    }
}

void MainWindow::watch_vmc()
{
    counter_watch_vmc_++;
    qDebug() << "[VMC] watch..." << counter_watch_vmc_;

    if (counter_watch_vmc_ >= 120) {
        tmr_watch_vmc_->stop();
    }
    else {
        if (ui->pbtn_onsite_open_D->isEnabled() == false) {
            int r = counter_watch_vmc_ % 4;
            if (r == 0)
                ui->pbtn_onsite_open_D->setText("運送中");
            else if (r == 1)
                ui->pbtn_onsite_open_D->setText("運送中.");
            else if (r == 2)
                ui->pbtn_onsite_open_D->setText("運送中..");
            else if (r == 3)
                ui->pbtn_onsite_open_D->setText("運送中...");
        }
    }
}

void MainWindow::watch_scanner()
{
    counter_watch_scanner_--;
    qDebug() << "[SCANNER] watch..." << counter_watch_scanner_;

    if (counter_watch_scanner_ <= 0) {
        tmr_watch_scanner_->stop();

        ui->stackedWidget->setCurrentIndex(Home);
    }
    else {
        if (current_page_ == PaymentHint &&
            ui->pbtn_payment_hint_back->isVisible()) {
            ui->pbtn_payment_hint_back->setText(QString("返回(%1)").arg(counter_watch_scanner_));
        }

        if (port_scanner_->isOpen() == false) {
            port_scanner_->open(QIODevice::ReadWrite);

            if (port_scanner_->isOpen() == false) {
                qDebug() << "[SCANNER] open failed";
                return;
            }
        }

        // clear buffer before write
        port_scanner_->clear();

        // write and trigger barcode reader
        QByteArray tx_data;
        tx_data.append(char(0x7E));
        tx_data.append(char(0x00));
        tx_data.append(char(0x08));
        tx_data.append(char(0x01));
        tx_data.append(char(0x00));
        tx_data.append(char(0x02));
        tx_data.append(char(0x01));
        tx_data.append(char(0xAB));
        tx_data.append(char(0xCD));
        port_scanner_->write(tx_data);
        port_scanner_->waitForBytesWritten(1000);
    }
}

void MainWindow::watch_payment()
{
    counter_watch_payment_--;
    qDebug() << "[PAYMENT] watch..." << counter_watch_payment_;

    if (counter_watch_payment_ <= 0) {
        tmr_watch_payment_->stop();

        if (cart_model_->item(0, 2)->text().indexOf("杏桃") >= 0) {
            is_dessert_ = true;
            ui->lbl_onsite_open_hint_A->show();
            ui->lbl_onsite_open_hint_D->hide();
            ui->pbtn_onsite_open_A->show();
            ui->pbtn_onsite_open_B->hide();
            ui->pbtn_onsite_open_D->hide();
            ui->pbtn_onsite_finish->setEnabled(false);
            ui->lbl_onsite_result_A->show();
            ui->lbl_onsite_result_B->hide();
            ui->lbl_onsite_result_D->hide();
            ui->lbl_onsite_result_bg->setPixmap(QPixmap(QString(dir_resources) + "P9_onsite_result_A.jpg"));
            ui->stackedWidget->setCurrentIndex(OnSiteResult);
        }
        else {
            is_dessert_ = false;
            ui->lbl_onsite_open_hint_A->show();
            ui->lbl_onsite_open_hint_D->hide();
            ui->pbtn_onsite_open_A->hide();
            ui->pbtn_onsite_open_B->show();
            ui->pbtn_onsite_open_D->show();
            ui->pbtn_onsite_open_D->setIcon(QIcon());
            ui->pbtn_onsite_open_D->setEnabled(false);
            ui->pbtn_onsite_finish->setEnabled(false);
            ui->lbl_onsite_result_A->hide();
            ui->lbl_onsite_result_B->show();
            ui->lbl_onsite_result_D->show();
            ui->lbl_onsite_result_bg->setPixmap(QPixmap(QString(dir_resources) + "P9_onsite_result_B.jpg"));
            ui->stackedWidget->setCurrentIndex(OnSiteResult);

            if (port_hvmc_ != nullptr) {
                port_hvmc_->executeChannel(2, 2, 4, 4);
            }
            counter_watch_vmc_ = 0;
            tmr_watch_vmc_->start();
        }
    }
    else {
        if (ui->pbtn_payment_hint_back->isEnabled()) {
            ui->pbtn_payment_hint_back->setText(QString("返回(%1)").arg(counter_watch_payment_));
        }
        else {
            ui->pbtn_payment_hint_back->setText(QString("(%1)").arg(counter_watch_payment_));
        }
    }
}

void MainWindow::watch_operation()
{
    counter_watch_operation_--;
    qDebug() << "[PAGE] watch..." << counter_watch_operation_;

    if (counter_watch_operation_ <= 0) {
        tmr_watch_operation_->stop();

        ui->stackedWidget->setCurrentIndex(Home);
    }
    else {
        switch (current_page_) {
        case OnSiteConfirm:
            ui->pbtn_product_close->setText(QString("關閉(%1)").arg(counter_watch_operation_));
        break;
        default:
        break;
        }
    }
}

void MainWindow::watch_door_status()
{
    QList<bool> status;
    QString message = "";

    if (is_dessert_) {
        port_fvmc_->readDoorStatus(1);
        QThread::msleep(10);

        status.clear();
        status = port_fvmc_->getDoorStatus(1);
        for (int i = 0; i < status.count() && i < 36; i++) {
            if (status.at(i) == false)
                message.append(QString("A櫃%1號, ").arg(i + 1, 2, 10, QLatin1Char('0')));
        }
    }
    else {
        port_fvmc_->readDoorStatus(0);
        QThread::msleep(10);

        status.clear();
        status = port_fvmc_->getDoorStatus(0);
        for (int i = 0; i < status.count() && i < 36; i++) {
            if (status.at(i) == false)
                message.append(QString("B櫃%1號, ").arg(i + 1, 2, 10, QLatin1Char('0')));
        }
    }

    if (message.isEmpty()) {
        if (current_page_ == OnSiteResult) {
            ui->lbl_onsite_result_hint->hide();
            //ui->lbl_onsite_result_door_hint->hide();
            ui->pbtn_onsite_finish->setEnabled(true);
        }
        else if (current_page_ == PreOrderResult) {
            ui->lbl_preorder_result_hint->hide();
            //ui->lbl_preorder_result_door_hint->hide();
            ui->pbtn_preorder_finish->setEnabled(true);
        }
        else if (current_page_ == ECPickupResult) {
            ui->lbl_ec_pickup_result_hint->hide();
            //ui->lbl_ec_pickup_result_door_hint->hide();
            ui->pbtn_ec_pickup_finish->setEnabled(true);
        }
    }
    else {
        if (current_page_ == OnSiteResult) {
            ui->lbl_onsite_result_hint->show();
            //ui->lbl_onsite_result_door_hint->show();
            //ui->lbl_onsite_result_door_hint->setText(message.left(message.length() - 2));
            ui->pbtn_onsite_finish->setEnabled(false);
        }
        else if (current_page_ == PreOrderResult) {
            ui->lbl_preorder_result_hint->show();
            //ui->lbl_preorder_result_door_hint->show();
            //ui->lbl_preorder_result_door_hint->setText(message.left(message.length() - 2));
            ui->pbtn_preorder_finish->setEnabled(false);
        }
        else if (current_page_ == ECPickupResult) {
            ui->lbl_ec_pickup_result_hint->show();
            //ui->lbl_ec_pickup_result_door_hint->show();
            //ui->lbl_ec_pickup_result_door_hint->setText(message.left(message.length() - 2));
            ui->pbtn_ec_pickup_finish->setEnabled(false);
        }
    }
}

void MainWindow::vmcTimeoutWithState(QString err_msg, int state)
{
    qDebug() << "[VMC] timeout" << err_msg << state;
}

void MainWindow::getVmcFirmwareInfosResponse(QString infos)
{
    qDebug() << "[VMC] version:" << infos;
}

void MainWindow::getVmcTemperatureStatusResponse(QString status)
{
    Q_UNUSED(status);
}

void MainWindow::setVmcCompressorSwitchResponse(bool result)
{
    Q_UNUSED(result);
}

void MainWindow::setVmcDoorSwitchResponse(bool result)
{
    Q_UNUSED(result);
}

void MainWindow::executeVmcChannelResponse(bool result, int state)
{
    qDebug() << "[VMC] response" << result << state;

    if (result == false) {
        if (state == VMController::WARNING_NOT_PICKUP) {
            QPixmap pickup_pixmap(QString(dir_resources) + "btn_pickup.png");
            ui->pbtn_onsite_open_D->setIcon(QIcon(pickup_pixmap));
            ui->pbtn_onsite_open_D->setText("");
            ui->pbtn_onsite_open_D->setEnabled(true);
        }
    }
    else {
        if (state == VMController::WAIT_CH_DONE ||
            state == VMController::WAIT_CH_RETRY) {
            ui->pbtn_onsite_open_D->setIcon(QIcon());
            ui->pbtn_onsite_open_D->setText("請至取貨口\n領取商品");
            ui->pbtn_onsite_open_D->setEnabled(true);
        }
        else if (state == VMController::WAIT_CARS) {
            tmr_watch_vmc_->stop();
            ui->pbtn_onsite_open_D->hide();
        }
    }
}

void MainWindow::fvmcOpenDoorResponse(int station, int numbering, bool result)
{
    Q_UNUSED(station);
    Q_UNUSED(numbering);
    Q_UNUSED(result);
}

void MainWindow::fvmcReadDoorStatusResponse(int station, bool result)
{
    Q_UNUSED(station);
    Q_UNUSED(result);
}

void MainWindow::cvmcDoorStatusResponse()
{

}

void MainWindow::executeCardTransactionResponse(bool result)
{
    Q_UNUSED(result);
}

void MainWindow::executeCardSettlementResponse(bool result)
{
    Q_UNUSED(result);
}

void MainWindow::scannerDataReady()
{
    // read all data
    QByteArray ba = port_scanner_->readAll();
    while (port_scanner_->waitForReadyRead(20)) {
        ba += port_scanner_->readAll();
    }

    // skip the cmd response data
    if (ba.length() > 0) {
        if (ba.at(0) == (char)0x02) {
            return;
        }
    }

    qDebug() << QString("[Port] barcode data(%1):").arg(ba.length()) << ba.data();
    QString sz_barcode = QString::fromUtf8(ba);
    sz_barcode = sz_barcode.left(sz_barcode.indexOf("\r"));

    if (current_page_ == EinvoiceHint) {

        if (einvoice_type_ == Carrier) {

            // check barcode is valid
            QRegExp rx_carrier("^/[0-9A-Z+-.]{7,8}$");
            QRegExp rx_citizen("^[A-Z]{2}[0-9]{14}$");

            if (rx_carrier.exactMatch(sz_barcode) ||
                rx_citizen.exactMatch(sz_barcode)) {

                ui->lineEdit_einvoice_code->setText(sz_barcode);
                ui->lbl_einvoice_carrier->setPixmap(QPixmap());
                ui->lbl_einvoice_carrier->setText(QString("載具號碼 %1").arg(sz_barcode));
            }
        }
        else if (einvoice_type_ == LoveCode) {
            ui->lineEdit_einvoice_code->setText(sz_barcode);
        }
    }
    else if (current_page_ == PaymentHint) {

    }
    else if (current_page_ == PreOrderHint) {
        ui->lineEdit_preorder_code->setText(sz_barcode);
    }
    else if (current_page_ == ECPickupHint) {
        ui->lineEdit_ec_pickup_code->setText(sz_barcode);
    }
}

void MainWindow::on_pbtn_test_released()
{
    ui->stackedWidget->setCurrentIndex(Test);
}

void MainWindow::on_cbBox_port_currentIndexChanged(const QString &arg1)
{
    ui->spBox_station->setEnabled((arg1.indexOf("S2") > 0));
}

void MainWindow::on_pbtn_test_open_released()
{
    QString port = ui->cbBox_port->currentText().left(ui->cbBox_port->currentText().indexOf("("));
    if (port == "ttyS2") {
        port_fvmc_->openDoor(ui->spBox_station->value(), ui->spBox_number->value());
    }
    else if (port == "ttyUSB0") {
        port_cvmc_->openDoor(0, ui->spBox_number->value());
    }
    else if (port == "ttyACM0") {
        if (ui->spBox_number->value() <= 9) {
            int row = (ui->spBox_number->value() - 1) / 3 + 1;
            int col = (ui->spBox_number->value() - 1) % 3 + 1;
            port_hvmc_->executeChannel(row, col);
        }
        else {
            port_hvmc_->executeChannel(4, (ui->spBox_number->value() - 9));
        }
    }
}
