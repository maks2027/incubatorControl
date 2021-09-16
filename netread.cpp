#include "netread.h"

NetRead::NetRead(QObject *parent) : QObject(parent)
{
    connect(server,&QTcpServer::newConnection,this,&NetRead::newConnection);
}

void NetRead::netConnect(int port)
{
    if(server->isListening())
        netDisconnect();

    server->listen(QHostAddress::Any, port);
}

void NetRead::netDisconnect()
{
    server->close();

    QHashIterator<QTcpSocket*, QByteArray*> i(buffers);
    while (i.hasNext())
    {
        i.next();
        i.key()->deleteLater();
        delete i.value();
    }
    buffers.clear();
}

void NetRead::newConnection()
{
    while (server->hasPendingConnections())
    {
        QTcpSocket *socket = server->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), SLOT(readDataNet()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        QByteArray *buffer = new QByteArray();

        buffers.insert(socket, buffer);
    }
}

void NetRead::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);

    socket->deleteLater();
    delete buffer;

    buffers.remove(socket);//!!!!!!!!!!!
}

void NetRead::readDataNet()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);

    while (socket->bytesAvailable() > 0)
    {
        buffer->append(socket->readAll());

        int endInd = buffer->indexOf("##");
        if(endInd > 0)
        {
            QByteArray data = buffer->mid(0, endInd + 2);
            buffer->remove(0, endInd + 2);

            int startInd = data.indexOf("#");
            data.remove(0, startInd);

            parsData(data);
        }
    }
}

void NetRead::parsData(QByteArray data)
{
    QRegularExpression reData;
    QRegularExpressionMatchIterator it;
    int adr = -1;
    //QString mess;

    reData.setPattern("\\A#([^#]*?)\n");
    it = reData.globalMatch(data);
    if(it.hasNext())
    {
        auto match = it.next();

        QString address = match.captured(1);

        bool ok = false;
        int temp = address.toInt(&ok);

        if(ok)
            adr = temp;
    }

    if(adr >= 0)
        emit newData(data,adr);
}
