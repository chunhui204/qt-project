#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QStringList>
#include <QSemaphore>
#include <QImage>
#include <QVector>
#include <QList>
#include <QString>

//widget在stackedwidget中的索引
const int AUDIO_WIDGET_INDEX = 0;
const int VIDEO_WIDGET_INDEX =1;


//const QString
#define qcout qDebug()//<<__FILE__<<__LINE__<<": "

/*
 * 命令传递网络端口：8888
 * 音频接受网络端口：8889
 * 视频接受网络端口：8890
 */
const quint16 COMMAND_PORT = 8888;
const quint16 AUDIO_PORT = 8889;
const quint16 VIDEO_PORT = 8890;

const int AudioBufSize = 176400;//44100hz,16bit ~ 2s

//为麦克风设备信息结构体
struct AudioFormat
{
    QString deviceName;
    QString sampleRates;
    QString channel;
    QString sampleSizes;
    QString codec;
    QString ip;

} ;
typedef QList<AudioFormat> AudioFmtList;

struct AudioDevice : public QMap<QString, int>
{
	void addItem(const QString& ip, int dev)
	{
		(*this)[ip] = dev;
	}
	int deleteItem(int dev)
	{
		auto it = this->begin();
		while (it != this->end())
		{
			if (it.value() == dev)
			{
				this->erase(it);
				return 0;
			}
			++it;
		}
		return -1;
	}
	QString dev_to_ip(QString dev)
	{
		for (auto it = begin(); it != end(); it++)
		{
			if (QString::fromLocal8Bit("设备") + QString::number(it.value()) == dev)
				return it.key();
		}
		return QString();
	}
};



//摄像头
const int VideoBufferSize = 5;
extern QSemaphore VideoBufUsed;
extern QSemaphore VideoBufFree;
extern QImage VideoBuffer[VideoBufferSize];

/*
 * 文件保存目录
 * root目录下分
 * --root
 *      --AUDIO_PATH
 *          --AUDIO_FILE_PATH
 *          --AUDIO_TRAIN_PATH
 *      --VIDEO_PATH
 *          --VIDEO_AVI_PATH
 *          --VIDEO_IMAGE_PATH
 */
const QString AUDIO_PATH = "audioData";
const QString VIDEO_PATH = "videoData";
const QString AUDIO_FILE_PATH = "wavefiles";
const QString AUDIO_TRAIN_PATH = "trainfiles";
const QString AUDIO_PARAM_PATH = "params";
const QString AUDIO_DIAGNOSE_PATH = "diagnose";

//音频福利叶变换，训练相关
//one frame
const int FFTLengthPowerOfTwo = 13; //4096点 ~ 23ms（44100hz采样频率）
const int SpectrumLengthSamples = 1 << FFTLengthPowerOfTwo;

const int SpectrumTimeForCalc = SpectrumLengthSamples;//ms
//重叠采样的间隔点数。
const int AudioDummyLength = 3969;//0.09~44100
const int Min_SpectrumTime_Train = 44100 * 10;//最小用于训练的字节长度 hz * time

const double CDF_THRESHOLD = 0.03;
const double PROB_THRESHOLD = 0.01;
const std::vector<int> GROUP={5,3,7};
const int SPECTRUM_FEAT_DIM = 15; //特征维数


#endif // COMMON_H
