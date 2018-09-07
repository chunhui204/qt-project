#include "videothread.h"

VideoThread::VideoThread(QObject *parent) : QObject(parent)
{
    videoSocket = new QTcpSocket(this);
}

VideoThread::~VideoThread()
{
    videoSocket->disconnectFromHost();
    videoSocket->close();
}
void VideoThread::onConnectHost(QString ip)
{
    if(false == videoSocket->isValid())
    {
        videoSocket->connectToHost(QHostAddress(ip), 8890);
    }

}

void VideoThread::onFramePresented( QImage image)
{
    if(videoSocket->isValid())
    {
        QByteArray byte;
        QBuffer buf(&byte);

        image.save(&buf,"JPEG");
        QByteArray ss=qCompress(byte,1);

        QByteArray ba;
        QDataStream out(&ba,QIODevice::WriteOnly);

        out << int(0) << ss;
        out.device()->seek(0);
        out<< ss.size();

        videoSocket->write(ba);
    }
}
