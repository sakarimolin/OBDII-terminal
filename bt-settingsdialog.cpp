/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin <sakari.molin@hotmail.com>
**
****************************************************************************/

#include "bt-settingsdialog.h"
#include "ui_bt-settingsdialog.h"
#include "service-discovery.h"

//#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QMenu>
#include <QFlags>

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("BluetoothSettingsDialog", "N/A");

BluetoothSettingsDialog::BluetoothSettingsDialog(QWidget *parent) : QDialog(parent)
{
    ui = new Ui::BluetoothSettingsDialog;
    ui->setupUi(this);

    intValidator = new QIntValidator(0, 4000000, this);

    connect(ui->applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    localDevice = 0;
    btDevDisc = 0;
    servDisc = 0;
    ui->localDevEdit->setText("powering up...");
    updateSettings();
}

BluetoothSettingsDialog::~BluetoothSettingsDialog()
{
    if(localDevice)
        delete localDevice;
    if(btDevDisc)
        delete btDevDisc;
    if(servDisc)
        delete servDisc;
    delete ui;
}

void BluetoothSettingsDialog::showPortInfo(int idx)
{
    if (idx == -1)
        return;
}

void BluetoothSettingsDialog::apply()
{
    updateSettings();
    btSettings.accepted = true;
    hide();
}

void BluetoothSettingsDialog::updateSettings()
{
}

void BluetoothSettingsDialog::on_cancelButton_clicked()
{
    btSettings.accepted = false;
    hide();
}

void BluetoothSettingsDialog::on_btActivateButton_clicked()
{
    startDeviceDiscovery();
}

void BluetoothSettingsDialog::startDeviceDiscovery()
{
    if(localDevice == 0)
        localDevice = new QBluetoothLocalDevice();

    connect(localDevice, SIGNAL(deviceConnected(QBluetoothAddress)),this, SLOT(newDevConnected(QBluetoothAddress)));
    connect(localDevice, SIGNAL(error(QBluetoothLocalDevice::Error)),this, SLOT(localDevError(QBluetoothLocalDevice::Error)));

    connect(ui->btListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(btRowActivated(QListWidgetItem*)));

    ui->applyButton->setDisabled(true);
    QString localDeviceName;

    // Check if Bluetooth is available on this device
    if (localDevice->isValid()) {
            // Turn Bluetooth on
        localDevice->powerOn();
        qDebug() << "bt device power set on" << endl;
            // Read local device name
        localDeviceName = localDevice->name();
        qDebug() << "bt device name: " << localDeviceName << " address=" << localDevice->address().toString();
        ui->localDevEdit->setText(localDeviceName);
            // Make it visible to others
        localDevice->setHostMode(QBluetoothLocalDevice::HostConnectable);

            // Get connected devices
        QList<QBluetoothAddress> remotes;
        remotes = localDevice->connectedDevices();
        int ind = 0;
        foreach(QBluetoothAddress remote, remotes)
        {
            ui->btListWidget->addItem(remote.toString());
            ind++;
//            ui->btListWidget->currentItem()->setTextColor();
        }
    }
    if(btDevDisc == 0)
        btDevDisc = new DevDiscovery(this);

    qDebug() << "bt device discovery at:" << btDevDisc;
    connect(btDevDisc, SIGNAL(deviceFound(const QBluetoothDeviceInfo&)),
            this, SLOT(devFound(const QBluetoothDeviceInfo&)));
    connect(btDevDisc, SIGNAL(pairedDev(const QBluetoothAddress&, QBluetoothLocalDevice::Pairing)),
            this, SLOT(devPaired(const QBluetoothAddress&, QBluetoothLocalDevice::Pairing)));
    ui->applyButton->setDisabled(false);

    qDebug() << "starting to scan...";
    btDevDisc->startScan(*localDevice);
}

void BluetoothSettingsDialog::btAccepted()
{
    qDebug() << "bt Accepted:" << btDevDisc->objectName();
    delete btDevDisc;
}

void BluetoothSettingsDialog::devFound(const QBluetoothDeviceInfo& devInfo)
{
    QString addrStr = devInfo.address().toString();
    QString nameStr = devInfo.name();

    QString remoteDev = addrStr + "  " + nameStr;
    qDebug() << "Sets devFound:" << remoteDev;
    bool found = false;
    QList<QListWidgetItem*> wList = ui->btListWidget->findItems(addrStr,Qt::MatchStartsWith);
    if(wList.count() > 0)
    {
        found = true;
        qDebug() << "Same Dev found";
        QListWidgetItem* item0 = wList.takeFirst();
        int row = ui->btListWidget->row(item0);
        QListWidgetItem* item2 = ui->btListWidget->takeItem(row);
        qDebug() << "Dev picked from list:" << item2->text();
        if(addrStr == item2->text().left(17))
            ui->btListWidget->addItem(remoteDev);
        else
            qDebug() << "But finally not the same:" << remoteDev << endl;

    }
    if(found == false)
    {
        qDebug() << "Add new Dev to list:" << remoteDev << endl;
        ui->btListWidget->addItem(remoteDev);
        btDevInfos.append(devInfo);
    }
    qDebug() << " Device name: " << devInfo.name();
    qDebug() << " Device Uuid: " << devInfo.deviceUuid().toString();
    qDebug() << " Device major class: " << devInfo.majorDeviceClass();
    qDebug() << " Device service class: " << (int)devInfo.serviceClasses() << endl;

/*
    QBluetoothDeviceInfo::DataCompleteness *completeness = 0;
    QList<QBluetoothUuid> servUuids = devInfo.serviceUuids(completeness);
    foreach(QBluetoothUuid sUuid , servUuids) {
        qDebug() << "Service Uuid: " << sUuid.toString() << endl;
        QString
        characteristicToString(CharacteristicType uuid)
        QString
        descriptorToString(DescriptorType uuid)
        QString
        protocolToString(ProtocolUuid uuid)
        QString
        serviceClassToString(ServiceClassUuid uuid)
    }
*/
/*
    if(!servDisc)
        servDisc = new ServiceDiscovery(devInfo.address(), localDevice, this);
    connect(servDisc, SIGNAL(serviceFound(const QBluetoothServiceInfo&)),
            this, SLOT(servFound(const QBluetoothServiceInfo&)));
    servDisc->startScan(QBluetoothUuid::Rfcomm);
*/
}

void BluetoothSettingsDialog::devPaired(const QBluetoothAddress& btAddr, QBluetoothLocalDevice::Pairing pairing)
{
    qDebug() << "Sets devPaired: " << btAddr.toString() << " pairing: " << pairing;
    int count = btDevInfos.count();
    qDebug() << "  dev count=" << count;
    for(int i = 0; i<count; i++)
    {
        if(btAddr == btDevInfos.at(i).address())
            activeBtDev = i;
    }
    remoteService.setDevice(btDevInfos.at(activeBtDev));
//    remoteService.registerService(localDevice->address());
    dumpService(remoteService);
    qDebug() << "Active Dev: " << activeBtDev << endl;

    QString remoteDev = btAddr.toString();

    QListWidgetItem item1(remoteDev);
    QListWidgetItem* item2;
    qDebug() << "item1:" << item1.text();
    for(int i=0; i<count; i++)
    {
        item2 = ui->btListWidget->item(i);
        if(item2->text().left(17) == remoteDev)
        {
            qDebug() << "item1 row is:" << i+1;
            item2 = ui->btListWidget->takeItem(i);
            qDebug() << "Dev found from list:" << item2->text();
            if(pairing) {
                remoteDev += "  paired";
                emit btAddressPaired(btAddr);
            }
            else
                remoteDev += "  NOT paired";

            break;
        }
    }
    remoteDev += " ...scanning services";
    ui->btListWidget->addItem(remoteDev);

    if(!servDisc)
        servDisc = new ServiceDiscovery(btAddr, localDevice, this);

    connect(servDisc, SIGNAL(serviceFound(const QBluetoothServiceInfo&)),
            this, SLOT(servFound(const QBluetoothServiceInfo&)));
    servDisc->startScan(QBluetoothUuid::Rfcomm);
//    servDisc->startScan(QBluetoothUuid::L2cap);
}

void BluetoothSettingsDialog::newDevConnected(QBluetoothAddress newDev)
{
    QString devAddr = newDev.toString();
    qDebug() << "Sets newDevConnected: " << devAddr;

    QList<QListWidgetItem*> wList = ui->btListWidget->findItems(devAddr,Qt::MatchStartsWith);
    if(wList.count() > 0)
        qDebug() << "Same addr exists";
    else
        ui->btListWidget->addItem(devAddr);
}

void BluetoothSettingsDialog::localDevError(QBluetoothLocalDevice::Error err)
{
    qDebug() << "locDev: Error = " << err;
}

void BluetoothSettingsDialog::servFound(const QBluetoothServiceInfo& servInfo)
{
    qDebug() << "--> servFound";
    QListWidgetItem* item3 = ui->btListWidget->takeItem(activeBtDev);
    QString dev = item3->text().left(26);
    dev += " - Done OK";
    ui->btListWidget->insertItem(activeBtDev, dev);

//    QBluetoothAddress lAddr = localDevice->address();
//    bool st = servInfo.registerService(lAddr);
    dumpService(servInfo);

    remoteService = servInfo;
    emit btServiceFound(remoteService);
}

void BluetoothSettingsDialog::on_btListWidget_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "ContextMenuRequested" << endl;
    QMenu menu(ui->btListWidget);
    QAction *pairAction = menu.addAction("Start Pairing");
    QAction *removePairAction = menu.addAction("Remove Pairing");
    QAction *chosenAction = menu.exec(ui->btListWidget->viewport()->mapToGlobal(pos));
    QListWidgetItem *currentItem = ui->btListWidget->currentItem();

    QString text = currentItem->text();
    int index = text.indexOf(' ');
    if (index == -1)
        return;

    QBluetoothAddress address (text.left(index));
    if (chosenAction == pairAction) {
        localDevice->requestPairing(address, QBluetoothLocalDevice::Paired);
    }
    else if (chosenAction == removePairAction) {
        localDevice->requestPairing(address, QBluetoothLocalDevice::Unpaired);
    }
}

void BluetoothSettingsDialog::on_btListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    qDebug() << "btListWidget DoubleClicked";
    QMenu menu(ui->btListWidget);
//    QAction *pairAction = menu.addAction("Start Pairing");
//    QAction *removePairAction = menu.addAction("Remove Pairing");

    QString text = item->text();
    qDebug() << "item = " << text;
    int index = text.indexOf(' ');
    if (index == -1)
        index = text.length();

    QBluetoothAddress address (text.left(index));
    localDevice->requestPairing(address, QBluetoothLocalDevice::Paired);
}

void BluetoothSettingsDialog::on_btListWidget_itemClicked(QListWidgetItem *item)
{
    qDebug() << "btListWidget itemClicked: " << item->text();
    QString text = item->text();
    qDebug() << "item = " << text;
    int index = text.indexOf(' ');
    if (index == -1)
        index = text.length();
    QBluetoothAddress address (text.left(index));
    localDevice->requestPairing(address, QBluetoothLocalDevice::Paired);
}

void BluetoothSettingsDialog::dumpService(const QBluetoothServiceInfo& sinfo)
{
    qDebug() << "Service name: " << sinfo.serviceName();
    qDebug() << "  descr.: " << sinfo.serviceDescription();
    qDebug() << "  protocol: " << sinfo.socketProtocol();
    qDebug() << "  channel: " << sinfo.serverChannel();
    qDebug() << "  complete: " << sinfo.isComplete();
    qDebug() << "  valid: " << sinfo.isValid();
    qDebug() << "  registered: " << sinfo.isRegistered();
    qDebug() << "  availability: " << sinfo.serviceAvailability();
    qDebug() << "  Uuid: " << sinfo.serviceUuid().toString();
}

void BluetoothSettingsDialog::btRowActivated(QListWidgetItem* btDevice)
{
    qDebug() << "Sets, btRowActivated" << btDevice->text();
    // tälle funktiolle ei taida olla käyttöä
}

void BluetoothSettingsDialog::readBtSocket()
{
    qDebug() << "Sets, readBtSocket";
    ;
}

void BluetoothSettingsDialog::btConnected()
{
    qDebug() << "Sets, btConnected";
    ;
}
void BluetoothSettingsDialog::btDisconnected()
{
    qDebug() << "Sets, btDisconnected";
    ;
}
