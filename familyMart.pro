QT       += core gui serialport network multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_DEPRECATED_WARNINGS
DEFINES += _DEV_STAGE_
DEFINES += _DEV_DEBUG_

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cms_api.cpp \
    cvm_controller.cpp \
    fvm_controller.cpp \
    keyboard_input.cpp \
    main.cpp \
    main_window.cpp \
    nccc_edc.cpp \
    num_input.cpp \
    printer_kp247.cpp \
    product_button.cpp \
    vm_controller.cpp

HEADERS += \
    cms_api.h \
    cvm_controller.h \
    fvm_controller.h \
    keyboard_input.h \
    machine_info.h \
    main_window.h \
    nccc_edc.h \
    num_input.h \
    printer_kp247.h \
    product_button.h \
    vm_controller.h

FORMS += \
    keyboard_input.ui \
    main_window.ui \
    num_input.ui \
    product_button.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
