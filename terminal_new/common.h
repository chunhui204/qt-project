#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QStringList>
#include <QImage>
#include <QVector>
#include <QList>

/*
 * 命令传递网络端口：8888
 * 音频接受网络端口：8889
 * 视频接受网络端口：8890
 */
#define qcout qDebug()
const quint16 AUDIO_PORT = 8888;
const quint16 VIDEO_PORT = 8889;

const int AudioBufSize = 176400;//44100hz,16bit ~ 2s

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
