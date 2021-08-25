/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin
**
****************************************************************************/

#ifndef SERVICEDISC_H
#define SERVICEDISC_H

#include <qobject.h>
#include <qbluetoothglobal.h>
#include <QBluetoothUuid>
#include <QBluetoothAddress>

QT_FORWARD_DECLARE_CLASS(QBluetoothLocalDevice)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceInfo)
QT_FORWARD_DECLARE_CLASS(QBluetoothServiceDiscoveryAgent)

QT_USE_NAMESPACE

class ServiceDiscovery : public QObject
{
    Q_OBJECT

public:
    ServiceDiscovery(const QBluetoothAddress& address, const QBluetoothLocalDevice& localDevice, QObject *parent = 0);
    ~ServiceDiscovery();

public slots:
    void addService(const QBluetoothServiceInfo&);
    void startScan(QBluetoothUuid filter);

signals:
    void serviceFound(const QBluetoothServiceInfo &info);

private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QBluetoothAddress localAdapter;
};

#endif
