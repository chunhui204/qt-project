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

class AudioBase : public QObject
{
    Q_OBJECT
public:
    explicit AudioBase(QObject *parent = 0);
    ~AudioBase();
    QByteArray audioInfoToTcp() const;

    void startAudio();
    void stopAudio();

//    char * buffer() const;
signals:
    void audioError(QString str);
    void audioFormatInfo(int chns, int sampleSize, int sampleRate);
public slots:
    void onAudioFormatChanged(const QString &,const QString &,const QString &,const QString &,const QString &);

private:
    qint64 audioBufferLength(const QAudioFormat &format) const;
    void initializeAudio();
    void initializeAudioInput();
private slots:
    void audioDataReady();

private:
    //上位机读到的位置，当前数据长度，总长度
    int m_bufferpos;
    //audio 设备
    QAudioFormat m_format;
    QAudioInput *m_audioInput;
    QAudioDeviceInfo inputDevice;
    QIODevice *audioIO;
    QAudioOutput *output;
};

#endif // AUDIOBASE_H
