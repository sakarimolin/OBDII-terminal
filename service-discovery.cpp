/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin
**
****************************************************************************/

#include "service-discovery.h"

#include <qbluetoothaddress.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>


ServiceDiscovery::ServiceDiscovery(const QBluetoothAddress& remoteAddress,
        const QBluetoothLocalDevice& localDevice, QObject *parent)
:   QObject(parent)
{
    localAdapter = localDevice.address();
    discoveryAgent = new QBluetoothServiceDiscoveryAgent(localAdapter);
    discoveryAgent->clear();
    discoveryAgent->setRemoteAddress(remoteAddress);

//    QList<QBluetoothUuid> uuidList;
//    uuidList.append(QBluetoothUuid::Rfcomm);
//    uuidList.append(QBluetoothUuid::SerialPort);
//    discoveryAgent->setUuidFilter(uuidList);
//    discoveryAgent->dumpObjectInfo();

    connect(discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(addService(QBluetoothServiceInfo)));
}

ServiceDiscovery::~ServiceDiscovery()
{
    delete discoveryAgent;
}

void ServiceDiscovery::startScan(QBluetoothUuid filter)
{
    QList<QBluetoothUuid> uuidList;
    uuidList.append(filter);
    discoveryAgent->setUuidFilter(uuidList);

    qDebug() << "Starting service scan, filter uuid:" << filter.toString();
    discoveryAgent->start();
}

void ServiceDiscovery::addService(const QBluetoothServiceInfo &info)
{
    if (info.serviceName().isEmpty())
        return;
    qDebug() << "--> addService";
    QList<quint16> attrs = info.attributes();
    int count = attrs.count();
    for(quint16 i=0; i <count; i++)
        qDebug() << "attribute " << attrs.at(i) << "has a value: " << info.attribute(attrs.at(i)).toByteArray()
                 << " str: " << info.attribute(attrs.at(i)).toString();
    emit serviceFound(info);
}
