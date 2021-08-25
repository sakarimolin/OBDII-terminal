greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport bluetooth
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = terminal
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    device.cpp \
    bt-console.cpp \
    bt-settingsdialog.cpp \
    service-discovery.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    device.h \
    bt-console.h \
    bt-settingsdialog.h \
    service-discovery.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    service.ui \
    bt-settingsdialog.ui

RESOURCES += \
    OBD-terminal.qrc
