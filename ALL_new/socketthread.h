#ifndef SOCKETTHREAD_H
#define SOCKETTHREAD_H
#include <QThread>
#include <QTcpSocket>
#include <memory>
#include <QObject>

class SocketThread : public QThread
{
    Q_OBJECT
public:
    SocketThread(int socketDescriptor, QObject* parent = nullptr) :
        tcpSocket_(new QTcpSocket(parent)),
        socketDescriptor_(socketDescriptor)
    {
        tcpSocket_->setSocketDescriptor(socketDescriptor_);
    }

    std::shared_ptr<QTcpSocket> getSocket()
    {
        return tcpSocket_;
    }

private:
    int socketDescriptor_;
    std::shared_ptr<QTcpSocket> tcpSocket_;
};

#endif // SOCKETTHREAD_H
