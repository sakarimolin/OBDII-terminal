#ifndef BTCONSOLE_H
#define BTCONSOLE_H
/****************************************************************************
**
** Copyright (C) 2016 Sakari Molin
**
****************************************************************************/

#include <qbluetoothserviceinfo.h>

#include <qbluetoothserver.h>
#include <qbluetoothaddress.h>

#include <QtCore/QObject>
#include <QPlainTextEdit>
#include <QBluetoothSocket>

QT_FORWARD_DECLARE_CLASS(QBluetoothSocket)
QT_USE_NAMESPACE

class BtConsole : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit BtConsole(QWidget *parent = 0);
    ~BtConsole();

    void startClient(const QBluetoothServiceInfo &remoteService);
    void startClient(const QBluetoothAddress& localAdapter);
    void stopClient();

public slots:
    void sendChars(const QByteArray &data);

    void startServer(const QBluetoothAddress& localAdapter);
    void startServer(const QBluetoothAddress& localAdapter, QBluetoothServiceInfo& servInfo);
    void stopServer();
    void serverError(QBluetoothServer::Error);

signals:
    void charsReceived(const QString &sender, const QString &chars);
    void getData(const QByteArray &data);
    void sendData(const QByteArray &data);
    void connected(const QString &name);
    void disconnected();

    void clientConnected(const QString &name);
    void clientDisconnected(const QString &name);

private slots:
    void readSocket();
    void connected();
    void socketError(QBluetoothSocket::SocketError error);

    void clientConnected();
    void clientDisconnected();

private:
    QWidget     *parentApp;
    int         socketTimer;
    QBluetoothSocket *socket;
    quint16     socketPort;
    quint16     retryCount;
    bool        localEchoEnabled;
    QString*    inputLine;
    int         linePos;

    QBluetoothServer *rfcommServer;
    QBluetoothServiceInfo serviceInfo;

    void dumpService(const QBluetoothServiceInfo& serviceInfo);

protected:
    void timerEvent(QTimerEvent *event);

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);

};
#endif // BTCONSOLE_H
