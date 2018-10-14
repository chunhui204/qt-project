#ifndef TCPAUDIOWARPER_H
#define TCPAUDIOWARPER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "common.h"
#include <QTcpSocket>
#include <memory>
#include "audiowarper.h"

class TcpAudioWarper : public AudioWarper
{
    Q_OBJECT
public:
    explicit TcpAudioWarper(const QString &ip, std::unique_ptr<QTcpSocket> so, QObject *parent = 0);
    TcpAudioWarper(const TcpAudioWarper&) = delete;
    TcpAudioWarper& operator= (const TcpAudioWarper&) = delete;


public slots:
    virtual void onReadyRead();
	void onCommandIssued(QString ip, QString command);
private:
	int readBytes;
	qint64 totalSize;
    std::unique_ptr<QTcpSocket> socket;
    
};

#endif // TCPAUDIOWARPER_H
