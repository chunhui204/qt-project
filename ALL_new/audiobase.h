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
#include <memory>
#include <tuple>
const int    NotifyIntervalMs       = 50;

class AudioBase : public QObject
{
    Q_OBJECT
public:
    explicit AudioBase(QObject *parent = 0);
    ~AudioBase();
	std::tuple<QIODevice*, AudioFormat> startAudio();
	void stopAudio();

private:
    //audio 设备
    QAudioInput *m_audioInput;
    QAudioDeviceInfo inputDevice;
	QIODevice* audioIO;
signals:
    void readyRead(int);
	
};

#endif // AUDIOBASE_H
