#ifndef AUDIOBASE_H
#define AUDIOBASE_H

#include <QObject>
#include <QAudio>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioInput>
#include <QByteArray>
#include <QList>
#include <QDataStream>
#include <QDebug>
#include <QTime>
#include <QAudioOutput>
#include "common.h"
#include <algorithm>

const int    NotifyIntervalMs       = 50;

class AudioBase : public QObject
{
    Q_OBJECT
public:
    explicit AudioBase(QObject *parent = 0);
    ~AudioBase();
    QByteArray audioInfo() const;

    void startAudio();
    void stopAudio();

//    char * buffer() const;
signals:
    void dataReadyEvent( QByteArray ,qint64 startPos, qint64 endPos);
public slots:
    void onAudioFormatChanged( AudioSettingFormat format);
private slots:
    void audioDataReady();

private:
    int bufferpos;
    //audio 设备
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QAudioDeviceInfo inputDevice;
    QIODevice *audioIO;
    QAudioOutput *output;
};

#endif // AUDIOBASE_H
