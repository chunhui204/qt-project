﻿#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QImage>
#include <QTime>
#include <QBuffer>
#include <QHostAddress>
//#include <QDebug>

//#define cout qDebug()<<__FILE__<<__LINE__
class VideoThread : public QObject
{
    Q_OBJECT
public:
    explicit VideoThread(QObject *parent = 0);
    ~VideoThread();
    void onConnectHost(QString ip);
signals:

private:
    QTcpSocket *videoSocket;
    QTime time;
public slots:
    void onFramePresented( QImage image);
public slots:
};

#endif // VIDEOTHREAD_H
