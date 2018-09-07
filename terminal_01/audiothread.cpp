#include "audiothread.h"
#include <iostream>
#include <qglobal.h>

Tensor bytes_to_tensor(const char* ptr, int row, int col);

AudioThread::AudioThread(QObject *parent) : QObject(parent)
{
    bufferpos_spectrum = 0;
    m_bufferpos = 0;
    audioSocket = new QTcpSocket(this);
    spectrumHelper = new SpectrumHelper(this);
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &AudioThread::timeout);
    timer->start(NotifyIntervalMs);


}
void AudioThread::onConnectHosts(QString ip)
{
    std::cout << "onConnectHost"<<std::endl;
    if(audioSocket->isValid())
        audioSocket->disconnectFromHost();
    audioSocket->connectToHost(QHostAddress(ip), 8889);
}
void AudioThread::onDisconnectHosts()
{
    if(audioSocket->isValid())
        audioSocket->disconnectFromHost();
}
AudioThread::~AudioThread()
{
    if(timer->isActive())
        timer->stop();
    delete timer;

    if(audioSocket->isValid())
    {
        audioSocket->disconnectFromHost();
        audioSocket->close();
    }
}
/**
 * @brief AudioThread::timeout
 */
void AudioThread::timeout()
{
    if(m_format.channelCount() == -1)
        return;

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

	int unitByte = m_format.channelCount() * m_format.sampleSize() / 8;
    int spectrum_len = SpectrumLengthSamples*unitByte;
    int length = m_format.channelCount() * m_format.sampleSize()/8 * m_format.sampleRate()
            * (2*NotifyIntervalMs) / 1000;//消费及时
	length -= length % unitByte;

    sema_usedbytes.acquire(length);
    QByteArray buffer_t(AudioBuffer + m_bufferpos, qMin(length, AudioBufferSize-m_bufferpos));
    if(m_bufferpos + length > AudioBufferSize)
    {
        QByteArray bt(AudioBuffer, length - (AudioBufferSize-m_bufferpos) );
        buffer_t.append(bt);
    }
    sema_freebytes.release(length);

    m_bufferpos = (m_bufferpos + length) % AudioBufferSize;

    stream << qint64(length) << buffer_t;

    if(audioSocket->isValid() && buffer_t.size()>0)
    {
        audioSocket->write(array);
    }
//    qcout << sema_freebytes.available() << time.elapsed();
    //predict
    time= QTime::currentTime();
    //数组还未存满，endpos > bufferpos_spectrum
    if(m_bufferpos - bufferpos_spectrum >= spectrum_len)
    {
        QByteArray d(AudioBuffer + bufferpos_spectrum, spectrum_len);
        bufferpos_spectrum +=  spectrum_len; //不可能越界

        int label = spectrumHelper->predict_of_frame(d, m_format.sampleSize(), unitByte);
        emit labelCalculated(label);
    }
    //数组存满
    else if(m_bufferpos < bufferpos_spectrum && AudioBufferSize - bufferpos_spectrum + m_bufferpos >=spectrum_len)
    {
        QByteArray d(AudioBuffer+bufferpos_spectrum,
                     qMin(AudioBufferSize-bufferpos_spectrum, spectrum_len));

        if(AudioBufferSize-bufferpos_spectrum < spectrum_len)
        {
            QByteArray dt(AudioBuffer, spectrum_len - (AudioBufferSize-bufferpos_spectrum));
            d.append(dt);
        }
        bufferpos_spectrum = (bufferpos_spectrum + spectrum_len) % AudioBufferSize;

        int label = spectrumHelper->predict_of_frame(d, m_format.sampleSize(), unitByte);
        emit labelCalculated(label);
    }
    qcout << time.elapsed() << sema_usedbytes.available();

}
void AudioThread::onParamesterUpdated(QByteArray arr, int row, int col)
{
    std::cout <<"paramester updated..."<<std::endl;
    spectrumHelper->updateParamester(arr, row, col);
}
void AudioThread::onAudioFormatInfo(int chns, int sampleSize, int sampleRate)
{
    m_format.setChannelCount(chns);
    m_format.setSampleSize(sampleSize);
    m_format.setSampleRate(sampleRate);
}
