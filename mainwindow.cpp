/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "bt-console.h"
#include "settingsdialog.h"
#include "bt-settingsdialog.h"

#include <QMessageBox>
#include <QtSerialPort/QSerialPort>

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pairedBtAddr = QBluetoothAddress(0);

    console = new Console;
    console->setEnabled(false);

    btConsole = new BtConsole(parent);
    btConsole->setEnabled(false);
    btServInfo = 0;

    serial = new QSerialPort(this);
    settings = new SettingsDialog;
    btSettings = new BluetoothSettingsDialog;

    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionQuit->setEnabled(true);
    ui->actionConfigure->setEnabled(true);

    ui->action_BlueTooth_conn->setEnabled(true);
    ui->actionDisconnect_blue_tooth->setEnabled(false);

    initActionsConnections();

    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));

    connect(btConsole, SIGNAL(connected(QString)), this, SLOT(btConnected(QString)));
    connect(btConsole, SIGNAL(sendData(QByteArray)), this, SLOT(writeBtData(QByteArray)));
    connect(btConsole, SIGNAL(charsReceived(QString, QString)), this, SLOT(receivedBtData(QString, QString)));

    connect(btConsole, SIGNAL(clientConnected(QString)), this, SLOT(clientConnect(QString)));
    connect(btConsole, SIGNAL(clientDisconnected(QString)), this, SLOT(clientDisconnect(QString)));
}

MainWindow::~MainWindow()
{
    delete settings;
    delete ui;
}

//
// serial terminal issues
//
void MainWindow::openSerialPort()
{
    setCentralWidget(console);
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        console->setEnabled(true);
        console->setLocalEchoEnabled(p.localEchoEnabled);
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                   .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                                   .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    }
    else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->statusBar->showMessage(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    console->setEnabled(false);
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
    ui->statusBar->showMessage(tr("Disconnected"));
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About OBD-Terminal"),
                       tr("The <b>OBD-Terminal</b> uses the Qt Serial Port "
                          "to connect ELM-device using BlueTooth connection."));
}

void MainWindow::writeData(const QByteArray &data)
{
    qDebug() << "mainWin write: " << data;
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    qDebug() << "mainWin read: " << data;
    console->putData(data);
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::initActionsConnections()
{
    connect(ui->actionConfigure, SIGNAL(triggered()), settings, SLOT(show()));

    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(openSerialPort()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionClear, SIGNAL(triggered()), console, SLOT(clear()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

//
// Bluetooth connection related issues start here
//
void MainWindow::on_actionBluetooth_settings_triggered()
{
    qDebug() << "-->BT settings";
    connect(btSettings, SIGNAL(btServiceFound(QBluetoothServiceInfo)), this, SLOT(btServDone(QBluetoothServiceInfo)));
    connect(btSettings, SIGNAL(btAddressPaired(QBluetoothAddress)), this, SLOT(btAddrPaired(QBluetoothAddress)));
    btSettings->show();
    qDebug() << "<--BT settings";
    btSettings->startDeviceDiscovery();

    if(btSettings->Accepted) {
        btConsole->setEnabled(true);
    }
}

void MainWindow::on_action_BlueTooth_conn_triggered()
{
    setCentralWidget(btConsole);
    qDebug() << "-->BT connect";
    ui->action_BlueTooth_conn->setEnabled(false);
    ui->actionDisconnect_blue_tooth->setEnabled(true);

    if(btSettings->Accepted) {
        btConsole->setEnabled(true);
        qDebug() << "  start client -->" << pairedBtAddr.toString();
        btConsole->startClient(*btServInfo);    // using BT client approach
//        btConsole->startClient(pairedBtAddr);    // using BT client approach

//        qDebug() << "  start server -->";
//        btConsole->startServer(localBtAddr);    // using server approach
//        btConsole->startServer(localBtAddr, *btServInfo);    // using server approach
    }
}

void MainWindow::on_actionDisconnect_blue_tooth_triggered()
{
    qDebug() << "-->BT close";
    ui->action_BlueTooth_conn->setEnabled(true);
    ui->actionDisconnect_blue_tooth->setEnabled(false);

    btConsole->setEnabled(false);
    btConsole->stopClient();
//    btConsole->stopServer();
}

void MainWindow::btServDone(const QBluetoothServiceInfo &info)
{
    qDebug() << "--> MainWindow::btServDone";
    btServInfo = (QBluetoothServiceInfo*)&info;
    localBtAddr = btSettings->localDevice->address();
}

void MainWindow::btAddrPaired(const QBluetoothAddress &addr)
{
    qDebug() << "--> MainWindow::btAddrPaired" << addr.toString();
    pairedBtAddr = addr;
}

void MainWindow::writeBtData(const QByteArray &data)
{
    qDebug() << "--> MainWindow::writeBtData  from " << data;
    btConsole->sendChars(data);

}

void MainWindow::receivedBtData(const QString &sender, const QString &chars)
{
    qDebug() << "--> MainWindow::receivedBtData, len:" << chars.length() << " from " << sender << endl << chars;
}

void MainWindow::btConnected(const QString &name)
{
    qDebug() << "--> MainWindow::btConnected " << name;
//    btConsole->sendChars("");
}

void MainWindow::clientConnect(const QString &name)
{
    qDebug() << "--> MainWindow::clientConnect " << name;
    btConsole->sendChars("");
}

void MainWindow::clientDisconnect(const QString &name)
{
    qDebug() << "--> MainWindow::clientDosconnect " << name;
}
