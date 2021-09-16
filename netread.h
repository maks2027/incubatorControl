#ifndef NETREAD_H
#define NETREAD_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHashIterator>
#include <QRegularExpression>

class NetRead : public QObject
{
    Q_OBJECT
public:
    explicit NetRead(QObject *parent = nullptr);

    void netConnect(int port);
    void netDisconnect();
private slots:

    void newConnection();
    void disconnected();
    void readDataNet();

signals:

    void newData(QByteArray data, int adr);

private:
    QTcpServer *server = new QTcpServer(this);
    QHash<QTcpSocket*, QByteArray*> buffers;

    void parsData(QByteArray data);

};

#endif // NETREAD_H
