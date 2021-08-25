
/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin <sakari.molin@hotmail.com>
**
****************************************************************************/

#ifndef BT_SETTINGSDIALOG_H
#define BT_SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QBluetoothServer>
#include "device.h"
#include "service-discovery.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE

namespace Ui {
class BluetoothSettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class BluetoothSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BluetoothSettingsDialog(QWidget *parent = 0);
    ~BluetoothSettingsDialog();

    struct BtSettings {
        QString name;
        bool    accepted;
    };
    BtSettings btSettings;
    DevDiscovery *btDevDisc;
    QBluetoothLocalDevice *localDevice;
//    QBluetoothServer *rfcommServer;
//    QBluetoothSocket *btSocket;
    QBluetoothServiceInfo remoteService;
    ServiceDiscovery *servDisc;

public slots:
    void devFound(const QBluetoothDeviceInfo& info);
    void devPaired(const QBluetoothAddress&, QBluetoothLocalDevice::Pairing);
    void servFound(const QBluetoothServiceInfo& info);
    void startDeviceDiscovery();

signals:
    void btAddressPaired(const QBluetoothAddress &addr);
    void btServiceFound(const QBluetoothServiceInfo &info);

private slots:
    void showPortInfo(int idx);
    void apply();
    void on_cancelButton_clicked();

    void on_btActivateButton_clicked();
    void btAccepted();
    void newDevConnected(QBluetoothAddress);
    void localDevError(QBluetoothLocalDevice::Error);

    void btRowActivated(QListWidgetItem* btDevice);
    void readBtSocket();
    void btConnected();
    void btDisconnected();

    void on_btListWidget_customContextMenuRequested(const QPoint &pos);
    void on_btListWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_btListWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::BluetoothSettingsDialog *ui;
    QIntValidator *intValidator;

    QList<QBluetoothDeviceInfo> btDevInfos;
    int activeBtDev;
    void updateSettings();
    void dumpService(const QBluetoothServiceInfo& serviceInfo);
};

#endif // BT_SETTINGSDIALOG_H
