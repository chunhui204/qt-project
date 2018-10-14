#include "audiobase.h"

AudioBase::AudioBase(QObject *parent) : QObject(parent)
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

    //初始化AudioInput
    m_audioInput = new QAudioInput(inputDevice, format, this);

}
AudioBase::~AudioBase()
{
}
//开启声音采集
std::tuple<QIODevice*, AudioFormat> AudioBase::startAudio()
{
	AudioFormat format;
    QAudioFormat fm = m_audioInput->format();
	format.deviceName = inputDevice.deviceName();
    format.sampleRates = QString::number(fm.sampleRate());
    format.sampleSizes = QString::number(fm.sampleSize());
    format.channel = QString::number(fm.channelCount());
    format.codec = fm.codec();
	format.ip = "localhost";

    audioIO = m_audioInput->start();
 //   //每次声卡采集到数据，IOdevice会受到新数据触发readyread信号。
    connect(audioIO, &QIODevice::readyRead, [&]()
    {
        emit readyRead(m_audioInput->bytesReady());
    });
	return std::make_tuple(audioIO, format);
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

    audioIO->disconnect();
}
