#include "audiowarper.h"
#include <QHostAddress>

AudioWarper::AudioWarper(QObject *parent) :
    QObject(parent),
    buf(new char[AudioBufSize]),
    cap(AudioBufSize),
    curvepos(0),
    sptrmpos(0),
    writepos(0),
    curveAvaliable(0),
    sptrmAvaliable(0),
    length_for_spectrum(0),
    paintFlag(false),
    writeoverF(false),
    m_spectrum(new SpectrumHelper(this)),
    timer(new QTimer(this)),
    socket(new QTcpSocket(this)),
    audioBase(new AudioBase(this)),
    state(Status::stopped)
{
    connect(audioBase, &AudioBase::readyRead, this, &AudioWarper::onReadyRead);
    connect(socket.get(), &QTcpSocket::readyRead, this, &AudioWarper::dealResponse);

    connect(timer, &QTimer::timeout, this, &AudioWarper::timeout);
    timer->start(30);
}
AudioWarper::~AudioWarper()
{
    if (timer->isActive())
        timer->stop();
    xs.clear();
    ys.clear();
    QVector<double>().swap(xs);
    QVector<double>().swap(ys);
}

void AudioWarper::timeout()
{
    int length = format.channelCount() * format.sampleRate()
        *format.sampleSize() / 8 * (50) / 1000;
    length -= length % 2;
    int unitByte = format.channelCount() * format.sampleSize() / 8;
    int frame_len = unitByte * SpectrumLengthSamples;//一帧的字节长度
    int samplesize = format.sampleSize();
    static double now = 0;	//注意static是类对象共享的

    if (paintFlag)//curve
    {
        curveAvaliable -= length;
        xs.clear();
        ys.clear();
        if (curveAvaliable > 0)
        {
            int num = length / unitByte;
            for (int i = 0; i < num; i++)
            {
                now = now + 1.0 / (format.sampleRate());
                xs.push_back(now);

                if (samplesize == 8)
                {
                    ys.push_back(*(buf.get() + curvepos) / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
                }
                else if (samplesize == 16)
                {
                    qint16 *ptr = reinterpret_cast<qint16 *>(buf.get() + curvepos);
                    double t = *ptr;
                    ys.push_back(*ptr / 32768.0);
                }
                curvepos = (unitByte + curvepos) % cap;
            }

            if (num) emit  curvePainted(xs, ys);
        }else
            curveAvaliable += length;

    }
    /*spectrum*/
    length_for_spectrum += length;      //积累的数据长度
    if (length_for_spectrum >= frame_len)//要处理的时间长度
    {
        length_for_spectrum = length_for_spectrum - frame_len;
        sptrmAvaliable -= frame_len;
        if (sptrmAvaliable <= 0)
        {
            sptrmAvaliable += frame_len;
            return;
        }
        QByteArray data(buf.get() + sptrmpos, qMin(SpectrumLengthSamples * unitByte, cap - sptrmpos));

        if (SpectrumLengthSamples*unitByte > cap - sptrmpos)
        {
            QByteArray tm(buf.get(), SpectrumLengthSamples*unitByte - (cap - sptrmpos));
            data.append(tm);
        }
        int label = m_spectrum->predict_of_frame(data, samplesize, unitByte);


        sptrmpos = (sptrmpos + SpectrumLengthSamples * unitByte) % cap;
        emit labelPredicted(label);
    }
}
void AudioWarper::paintStart()
{
    paintFlag = true;
    int length = format.channelCount() * format.sampleRate()
        *format.sampleSize() / 8 * (30) / 1000;
    length -= length % 2;
    int unitByte = format.channelCount() * format.sampleSize() / 8;
    int num = length / unitByte;

    xs.reserve(num);
    ys.reserve(num);
    //clear curve
    QVector<double> t;
    emit curvePainted(t, t);
}
void AudioWarper::paintStop()
{
    paintFlag = false;
    xs.clear();
    ys.clear();
    QVector<double>().swap(xs);
    QVector<double>().swap(ys);
}
void AudioWarper::onReadyRead(int bytes)
{
    QByteArray array = audioIO->read(bytes);
    writeCirclebuf(array.data(), array.size());
    if(socket->isValid())
    {
        QByteArray snda;
        QDataStream ss(&snda, QIODevice::WriteOnly);
        ss << QString("data") << qint64(bytes) << array;
        socket->write(snda);
    }

}
void AudioWarper::dealResponse()
{
    QByteArray arr = socket->readAll();
    QDataStream ss(&arr, QIODevice::ReadOnly);
    QString head;
    ss >> head;

    qcout <<head;
    if(head == "startAudio")
    {
        if(state == Status::started)
            return;
        auto ret = audioBase->startAudio();
        audioIO = std::get<0>(ret);
        format = std::get<1>(ret);
        QString dev = std::get<2>(ret);

        QByteArray arr;
        QDataStream ss(&arr, QIODevice::WriteOnly);
        ss<< QString("format");
         ss << dev << QString::number(format.sampleRate())
           << QString::number(format.channelCount()) << QString::number(format.sampleSize())
           << format.codec() << socket->peerAddress().toString();
        socket->write(arr);
        state = Status::started;
        paintStart();

    }else if(head == "stopAudio")
    {
        if(state == Status::stopped)
            return;
        audioBase->stopAudio();
        state = Status::stopped;
        paintStop();
    }else if(head == "paramester")
    {
        qint64 row, col;
        QByteArray arr;
        ss >> row >> col >> arr;
        m_spectrum->updateParamester(arr, row, col);
    }
}
void AudioWarper::onConnectSocket(QString ip, int port)
{
    socket->connectToHost(QHostAddress(ip), port);
}
void AudioWarper::onDisconnectSocket()
{
    qcout << "disconnect";
    socket->disconnectFromHost();
    audioBase->stopAudio();
    state = Status::stopped;
    paintStop();
}
