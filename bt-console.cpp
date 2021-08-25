/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin
**
****************************************************************************/

#include "bt-console.h"
#include <qbluetoothsocket.h>
#include <qbluetoothdeviceinfo.h>

//static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264ef01");
//static const QLatin1String serviceUuid("e8e10f95-1a70-4b27-9ccf-02010264e9c8");
static const QLatin1String serviceUuid("00001011-0000-1000-8000-00805F9B34FB");
//static const QLatin1String serviceUuid("00001003-0000-1000-8000-00805F9B34FB");
//static const QLatin1String serviceUuid("00000000-0000-0000-0000-000000000000");
//static const QLatin1String serviceUuid("b62e4e8d-62cc-404b-bbbf-003e3bbb1374");

BtConsole::BtConsole(QWidget *parent)
    : QPlainTextEdit(parent), socket(0), rfcommServer(0)
{
    parentApp = parent;
    localEchoEnabled = false;
    retryCount = 0;
    socketTimer = 0;
    document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::gray);
    p.setColor(QPalette::Text, Qt::blue);
    setPalette(p);
    inputLine = new QString();
    linePos = 0;
}

BtConsole::~BtConsole()
{
    stopClient();
    if(inputLine)
        delete inputLine;
    inputLine = 0;
}

void BtConsole::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (socket)
        return;

    // Connect to service
    if (&remoteService) {
        qDebug() << "Creating BT-socket 1";
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol,this);
//        socket = new QBluetoothSocket(QBluetoothServiceInfo::L2capProtocol);
        socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);

        serviceInfo = remoteService;
        qDebug() << "socket.state() = " << socket->state();
        qDebug() << "service uuid = " << remoteService.serviceUuid().toString();
        serviceInfo.setServiceUuid(QBluetoothUuid::Rfcomm);
        qDebug() << "new service uuid = " << serviceInfo.serviceUuid().toString();

        connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socketError(QBluetoothSocket::SocketError)));

        socketPort = socket->localPort();
//        socket->connectToService(remoteService);
        socket->connectToService(serviceInfo);
        qDebug() << "ConnectToService initiated";

        qDebug() << "socket.state() = " << socket->state();
        qDebug() << "socket.localPort() = " << socket->localPort();
        qDebug() << "socket.peerAddress() = " << socket->peerAddress().toString();
        qDebug() << "socket.peerPort() = " << socket->peerPort();
        QFlags<QBluetooth::Security> sf = socket->preferredSecurityFlags();
        qDebug() << "socket.prefSecurity = " << sf;

        localEchoEnabled = true;
        qint64 count = socket->bytesAvailable();
        qDebug() << "Bytes available = " << count;
    }
}

void BtConsole::startClient(const QBluetoothAddress& pairedAdapter)
{
    QBluetoothUuid uuid(QBluetoothUuid::Rfcomm);
    if (socket)
        return;

    // Connect to service
    if (&pairedAdapter) {
        qDebug() << "Creating BT-socket 2";
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol,this);

        qDebug() << "socket.state() = " << socket->state();

        connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socketError(QBluetoothSocket::SocketError)));

        socketPort = socket->localPort();
        socket->connectToService(pairedAdapter, uuid);
        qDebug() << "ConnectToService initiated, uuid = " << uuid.toString();

        qDebug() << "socket.state() = " << socket->state();
        qDebug() << "socket.localPort() = " << socket->localPort();
        qDebug() << "socket.peerAddress() = " << socket->peerAddress().toString();
        qDebug() << "socket.peerPort() = " << socket->peerPort();

        localEchoEnabled = true;
        qint64 count = socket->bytesAvailable();
        qDebug() << "Bytes available = " << count;
    }
}

void BtConsole::stopClient()
{
    if(socket)
        delete socket;
    socket = 0;
}

void BtConsole::readSocket()
{
    qDebug() << "-->BT:readSocket";
    if (!socket)
        return;

    qint64 count = socket->bytesAvailable();
    qDebug() << "Bytes available = " << count;

//    while (socket->canReadLine()) {
    while (socket->bytesAvailable()) {
        qDebug() << "  reading socket...";
        QByteArray line = socket->readLine();
        qDebug() << "  read chars:" << line;
        insertPlainText(line.constData());
        emit charsReceived(socket->peerName(), QString::fromUtf8(line.constData(), line.length()));
    }
}

//void BtConsole::sendChars(const QString &chars)
void BtConsole::sendChars(const QByteArray &chars)
{
    qDebug() << "-->BT:sendChars";
//    QByteArray text = chars.toUtf8() + '\n';
    if (!socket)
        return;
    socket->write(chars);
    qDebug() << "  sent chars:" << chars;
}

void BtConsole::connected()
{
    qDebug() << "-->BT:connected";
    emit connected(socket->peerName());
}

void BtConsole::socketError(QBluetoothSocket::SocketError err)
{
    QString errStr = socket->errorString();
    quint16 port = socket->localPort();
    qDebug() << "-->BT:error() " << err << " -- " << errStr;
    qDebug() << "      socket.state() = " << socket->state();
    qDebug() << "      socket.localPort() = " << port;
    if(retryCount < 5) {
        retryCount++;
//        socketTimer = this->startTimer(30000);
    }
}

void BtConsole::timerEvent(QTimerEvent *event)
{
    qDebug() << "Timer ID:" << event->timerId() << "socketTimer:" << socketTimer
             << " retries:" << retryCount;
    killTimer(event->timerId());
    socket->connectToService(serviceInfo);
}

void BtConsole::keyPressEvent(QKeyEvent *e)
{
    int len = inputLine->length();
    switch (e->key()) {
    case Qt::Key_Backspace:
        inputLine->remove(linePos, 1);
        if (linePos > 1)
            linePos--;
        break;
    case Qt::Key_Left:
        if (linePos > 1)
            linePos--;
        break;
    case Qt::Key_Right:
        if (linePos < len)
            linePos++;
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
        break;
    case Qt::Key_Return:
        qDebug() << "BT:keyPressEvent, send:" << e->text();
        inputLine->append(e->text().toLocal8Bit());
        emit sendData(inputLine->toLocal8Bit());
        inputLine->clear();
        linePos = 0;
        break;
    default:
        if (localEchoEnabled)
            QPlainTextEdit::keyPressEvent(e);
        qDebug() << "BT:keyPressEvent, get:" << e->text();
        inputLine->append(e->text().toLocal8Bit());
        emit getData(e->text().toLocal8Bit());
    }
}

void BtConsole::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
    if (!socket)
        clientConnected();      // try connection
}

void BtConsole::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

void BtConsole::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}

void BtConsole::startServer(const QBluetoothAddress& localAdapter)
{
    if (rfcommServer)
        return;

    qDebug() << "BT:startServer";
    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    connect(rfcommServer, SIGNAL(error(QBluetoothServer::Error)), this, SLOT(serverError(QBluetoothServer::Error)));
    localEchoEnabled = true;

    //serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceRecordHandle, (uint)0x00010010);

    //! [Class Uuuid must contain at least 1 entry]
    QBluetoothServiceInfo::Sequence classId;

    classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::SerialPort));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList, classId);

    classId.prepend(QVariant::fromValue(QBluetoothUuid(serviceUuid)));

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
    serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,classId);
    //! [Class Uuuid must contain at least 1 entry]


    //! [Service name, description and provider]
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Bt Server for OBD"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                             tr("bluetooth server"));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider, tr("S.Molin"));
    //! [Service name, description and provider]

    //! [Service UUID set]
    serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));
    //! [Service UUID set]

    //! [Service Discoverability]
    QBluetoothServiceInfo::Sequence publicBrowse;
    publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));
    serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);
    //! [Service Discoverability]

    //! [Protocol descriptor list]
    QBluetoothServiceInfo::Sequence protocolDescriptorList;
    QBluetoothServiceInfo::Sequence protocol;
//    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
//    protocolDescriptorList.append(QVariant::fromValue(protocol));
    protocol.clear();
    protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
             << QVariant::fromValue(quint8(rfcommServer->serverPort()));
    protocolDescriptorList.append(QVariant::fromValue(protocol));
    serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                             protocolDescriptorList);
    //! [Protocol descriptor list]

    serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceAvailability, true);

    //! [Register service]
    serviceInfo.registerService(localAdapter);
    //! [Register service]
    dumpService(serviceInfo);

    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Cannot bind chat server to" << localAdapter.toString();
        return;
    }
    else
        qDebug() << "listening rfcommServer";

}

void BtConsole::startServer(const QBluetoothAddress& localAdapter, QBluetoothServiceInfo& servInfo)
{
    QBluetoothServiceInfo remoteServ(servInfo);
    remoteServ.setAttribute(QBluetoothServiceInfo::ServiceName, tr("Bt Server for OBD"));

    if (rfcommServer)
        return;

    qDebug() << "BT:startServer";
    rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(rfcommServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));
    connect(rfcommServer, SIGNAL(error(QBluetoothServer::Error)), this, SLOT(serverError(QBluetoothServer::Error)));
    localEchoEnabled = true;

    remoteServ.registerService(localAdapter);
    //! [Register service]

    bool result = rfcommServer->listen(localAdapter);
    if (!result) {
        qWarning() << "Cannot bind chat server to" << localAdapter.toString();
        return;
    }
    else
        qDebug() << "listening rfcommServer";

}

void BtConsole::stopServer()
{
    qDebug() << "BT:stopServer";
    // Unregister service
    serviceInfo.unregisterService();

    // Close sockets
//    qDeleteAll(clientSockets);

    // Close server
    delete rfcommServer;
    rfcommServer = 0;
}

void BtConsole::serverError(QBluetoothServer::Error error)
{
    qDebug() << "BT:serverError = " << error;
}

void BtConsole::clientConnected()
{
    qDebug() << "BT:clientConnected";
    socket = rfcommServer->nextPendingConnection();
    if (!socket) {
        qDebug() << "BT:clientConnected , get socket failed";
        return;
    }
    qDebug() << "BT:clientConnected , socket OK";
    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
//    clientSockets.append(socket);
    emit clientConnected(socket->peerName());
}

void BtConsole::clientDisconnected()
{
    qDebug() << "BT:clientDisconnected";
    socket = qobject_cast<QBluetoothSocket *>(sender());
    if (!socket)
        return;

    emit clientDisconnected(socket->peerName());

//    clientSockets.removeOne(socket);

    socket->deleteLater();
}

void BtConsole::dumpService(const QBluetoothServiceInfo& sinfo)
{
    qDebug() << "Service name: " << sinfo.serviceName();
    qDebug() << "Service descr.: " << sinfo.serviceDescription();
    qDebug() << "Service protocol: " << sinfo.socketProtocol();
    qDebug() << "Service complete: " << sinfo.isComplete();
    qDebug() << "Service valid: " << sinfo.isValid();
    qDebug() << "Service availability: " << sinfo.serviceAvailability();
    qDebug() << "Service Uuid: " << sinfo.serviceUuid().toString();
}

