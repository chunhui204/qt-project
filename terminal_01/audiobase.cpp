#include "audiobase.h"
#include <qglobal.h>

AudioBase::AudioBase(QObject *parent) : QObject(parent)
{
    initializeAudio();

}
AudioBase::~AudioBase()
{
}


//初始化个变量值
void AudioBase::initializeAudio()
{
    //初始化audioFormat
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(1);
    format.setSampleSize(16);//QT 的8位好像有问题，只使用16bit
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    inputDevice = QAudioDeviceInfo::defaultInputDevice();
    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = inputDevice.nearestFormat(format);
        if(inputDevice.deviceName().isEmpty())
        {
            emit audioError(QString("未找到麦克风设备，请插入设备后重启软件"));
        }
    }
        m_format = format;
        qcout << m_format.channelCount()<<m_format.sampleRate()
             <<m_format.sampleSize()<<m_format.codec()<< inputDevice.deviceName();

        initializeAudioInput();

}

void AudioBase::audioDataReady()
{
    //更新m_buffer
    int dataLength = m_audioInput->bytesReady();

    sema_freebytes.acquire(dataLength);
//    mutex_audio.lock();
    audioIO->read(AudioBuffer+m_bufferpos,
                  qMin(dataLength, AudioBufferSize - m_bufferpos));
    if(dataLength > AudioBufferSize - m_bufferpos)
        audioIO->read(AudioBuffer,
                      dataLength - (AudioBufferSize - m_bufferpos));
//    mutex_audio.unlock();
    sema_usedbytes.release(dataLength);
//    Q_ASSERT(dataLength == readLength);

    m_bufferpos = (m_bufferpos + dataLength) % AudioBufferSize;

}

//开启声音采集
void AudioBase::startAudio()
{
    audioIO = m_audioInput->start();
    //每次声卡采集到数据，IOdevice会受到新数据触发readyread信号。
    connect(audioIO, &QIODevice::readyRead, this, &AudioBase::audioDataReady);
    emit audioFormatInfo(m_format.channelCount(), m_format.sampleSize(), m_format.sampleRate());

}
void AudioBase::stopAudio()
{
    m_audioInput->stop();
    //清空buffer，重新采集
    m_bufferpos = 0;
    audioIO = NULL;
}

void AudioBase::onAudioFormatChanged(const QString &device, const QString &rate, const QString &channel, const QString & byte, const QString &codec)
{
    stopAudio();
    delete m_audioInput;

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(rate.toInt());
    format.setChannelCount(channel.toInt());
    format.setSampleSize(byte.toInt());
    format.setCodec(codec);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    foreach(QAudioDeviceInfo de, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        if(de.deviceName() == device)
            inputDevice  = de;

    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = inputDevice.nearestFormat(format);

        emit audioError(QString("音频设置与麦克风不匹配"));
    }
    m_format = format;
    initializeAudioInput();
    emit audioFormatInfo(m_format.channelCount(), m_format.sampleSize(), m_format.sampleRate());
}


void AudioBase::initializeAudioInput()
{
    m_bufferpos =  0;
//初始化AudioInput
    m_audioInput = new QAudioInput(inputDevice, m_format, this);
    //设置通知间隔，每次来通知时通过网络向上位机发送数据

    audioIO = NULL;
}

/* 获取音频设备信息（输入设备）
 * tcp信息头格式：
 * audioDeviceInfo##_deviceName##_sampleRate0,_sampleRate1...##_channel##_sampleSize##_audioCodec###
 * 上式中带"_"均为变量,如果每种变量存在多个值则以","分隔, _channel取1.
 * 如果有多个设备，上述从_deviceName的字符串循环添加。
 */
QByteArray AudioBase::audioInfoToTcp() const
{
    //获取输入设备信息
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    //先发送正在使用的设备信息
    stream << QString("%1").arg(m_format.sampleRate()) << QString("%1").arg(m_format.channelCount())
           << QString("%1").arg(m_format.sampleSize());

    //发送可选的设备信息
    //default format
    QAudioFormat static_format;
    static_format.setSampleRate(44100);
    static_format.setChannelCount(1);
    static_format.setSampleSize(16);
    static_format.setCodec("audio/pcm");
    static_format.setByteOrder(QAudioFormat::LittleEndian);
    static_format.setSampleType(QAudioFormat::SignedInt);

    QList<QAudioDeviceInfo> availableInputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo inDevice, availableInputDevices)
    {
        stream << QString("audioDevice");
        stream << inDevice.deviceName();

        QAudioFormat format = static_format ;
        if(false == inDevice.isFormatSupported(format))
        {
            format = inDevice.nearestFormat(format);
        }

        stream << QString("%1").arg(format.sampleRate()) << QString("%1").arg(format.channelCount())
               << QString("%1").arg(format.sampleSize()) << format.codec();

    }
    return array;
}
