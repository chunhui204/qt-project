#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDataStream>
#include <QAudioFormat>
#include "spectrumhelper.h"
#include "common.h"

const int    NotifyIntervalMs       = 30;

class AudioThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioThread(QObject *parent = 0);

    ~AudioThread();
signals:
    void labelCalculated(int label);

public slots:
    void onConnectHosts(QString ip);
    void onDisconnectHosts();
    void onParamesterUpdated(QByteArray arr, int row, int col);
    void onAudioFormatInfo(int chns, int sampleSize, int sampleRate);

private:
    void timeout();
private:
    QTcpSocket *audioSocket;
    SpectrumHelper *spectrumHelper;
    QAudioFormat m_format;
    int m_bufferpos;
    int bufferpos_spectrum;
    QTimer *timer;
    QTime time;
};

#endif // AUDIOTHREAD_H
