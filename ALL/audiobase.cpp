#include "audiobase.h"

AudioBase::AudioBase(QObject *parent) : QObject(parent)
  , bufferpos(0)
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
    }
    m_format = format;
    qcout << "curent format: "<< m_format.channelCount()<<m_format.sampleRate()
         <<m_format.sampleSize()<<m_format.codec()<< inputDevice.deviceName();

    //初始化AudioInput
    m_audioInput = new QAudioInput(inputDevice, m_format, this);
    audioIO = NULL;

}
AudioBase::~AudioBase()
{
}
void AudioBase::audioDataReady()
{
    //更新m_buffer
    qint64 dataLength = m_audioInput->bytesReady();
    QByteArray array = audioIO->read(dataLength);
    qint64 readLength = array.size();

    Q_ASSERT_X(dataLength >= readLength,"","dataLength < readLength???");

    AudioBufFree.acquire(readLength);//lock when free size == 0
    AudioMutex.lock();

    int ava = AudioBufSize - bufferpos;
    if(ava >= readLength)
        std::copy(array.data(), array.data()+readLength, AudioBuffer + bufferpos);
    else
    {
        std::copy(array.data(), array.data()+ava, AudioBuffer + bufferpos);
        std::copy(array.data()+ava, array.data()+readLength, AudioBuffer);
    }
    bufferpos = (bufferpos + readLength) % AudioBufSize;

    AudioMutex.unlock();
    AudioBufUsed.release(readLength);

}

//开启声音采集
void AudioBase::startAudio()
{
    audioIO = m_audioInput->start();
    //每次声卡采集到数据，IOdevice会受到新数据触发readyread信号。
    connect(audioIO, &QIODevice::readyRead, this, &AudioBase::audioDataReady);
}
/**
 * @brief 我觉得不用在切换本地采集还是网络采集的时候重置共享数组和信号量，因为数组中存储的是字节，
 * 切换设备时如果位数或通道改变了，但同时它的格式信息一样会反馈给我们，处理数据是按照当前数据格式处理就不会出错
 * Note：必须每次按start时都要反馈格式信息
 */
void AudioBase::stopAudio()
{
    m_audioInput->stop();
    //清空buffer，重新采集

    audioIO = NULL;
}

void AudioBase::onAudioFormatChanged(AudioSettingFormat asf)
{
    delete m_audioInput;

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(asf.sampleRates.toInt());
    format.setChannelCount(asf.channel.toInt());
    format.setSampleSize(asf.sampleSizes.toInt());
    format.setCodec(asf.codec);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    foreach(QAudioDeviceInfo de, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        if(de.deviceName() == asf.deviceName)
        {
            inputDevice  = de;
            break;
        }

    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = inputDevice.nearestFormat(format);
    }
    m_format = format;
    qcout <<"changed format:"<< m_format.channelCount()<<m_format.sampleRate()
         <<m_format.sampleSize()<<m_format.codec()<< inputDevice.deviceName();
    //初始化AudioInput
    m_audioInput = new QAudioInput(inputDevice, m_format, this);
    audioIO = NULL;
}


/* 获取音频设备信息（输入设备）
 * tcp信息头格式：
 * audioDeviceInfo##_deviceName##_sampleRate0,_sampleRate1...##_channel##_sampleSize##_audioCodec###
 * 上式中带"_"均为变量,如果每种变量存在多个值则以","分隔, _channel取1.
 * 如果有多个设备，上述从_deviceName的字符串循环添加。
 */
QByteArray AudioBase::audioInfo() const
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
