#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H
#include "tcpaudiowarper.h"
#include <QTcpServer>
#include <QObject>
#include <QTcpSocket>

class AudioServer : public QTcpServer
{
    Q_OBJECT
public:
	explicit AudioServer(QObject *parent = nullptr) : QTcpServer(parent) {}
	std::unique_ptr<QThread> getSocketThread()
	{
		return std::move(thread);
	}
protected:
    virtual void incomingConnection(qintptr socketDescriptor)
    {
		std::unique_ptr<QThread> td(new QThread(this));
		QTcpSocket *socket = new QTcpSocket;
		socket->setSocketDescriptor(socketDescriptor);
		socket->moveToThread(td.get());
		addPendingConnection(socket);
		qcout << "sub" << td.get();
		td->start();
		thread.reset(td.release());
    }
private :
	std::unique_ptr<QThread> thread;
};

#endif // AUDIOSERVER_H
