#ifndef AUDIOWARPER_H
#define AUDIOWARPER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "common.h"
#include <QTcpSocket>
#include <memory>
#include <QVector>
#include <QTimer>
#include <QIODevice>
#include <QAudioFormat>
#include "spectrumhelper.h"
#include "audiobase.h"

enum class Status{
    started = 0,
    stopped = 1
};

class AudioWarper : public QObject
{
    Q_OBJECT
private:
    QTimer *timer;
    bool paintFlag;
    int curvepos;
    int sptrmpos;
    int writepos;
    int curveAvaliable;
    int sptrmAvaliable;

    bool writeoverF; //第一次写满数组
    int length_for_spectrum;

    SpectrumHelper *m_spectrum;
    std::unique_ptr<char> buf;
    std::unique_ptr<QTcpSocket> socket;
    QIODevice *audioIO;
    AudioBase *audioBase;
    Status state;
protected:
    const int cap;
    QVector<double> xs, ys;
    QAudioFormat format; //子类必须显式更新format

signals:
    void curvePainted(QVector<double>, QVector<double>);
    void labelPredicted(int);

public:
    explicit AudioWarper(QObject *parent = nullptr);
    virtual ~AudioWarper();
    void paintStart();
    void paintStop();
    void onConnectSocket(QString ip, int port);
    void onDisconnectSocket();
private:
    void onReadyRead(int);
    void timeout();
    void dealResponse();


    void writeCirclebuf(const char *src, int size)
    {
        int len = qMin(size, cap - writepos);
        memcpy(buf.get() + writepos, src, len);

        if (len < size)
            memcpy(buf.get(), src + len, size - len);
        if (writepos + size > cap)
            writeoverF = true;
        writepos = (writepos + size) % cap;
        curveAvaliable += size;
        sptrmAvaliable += size;
    }

};

#endif // AUDIOWARPER_H
