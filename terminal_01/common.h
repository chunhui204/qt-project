#ifndef COMMON_H
#define COMMON_H
#include <QSemaphore>

const int BufferDurationS       = 2;// 单位： seconds
const int AudioBufferSize = 44100 * BufferDurationS * 2;
extern QSemaphore sema_usedbytes;
extern QSemaphore sema_freebytes;
extern char AudioBuffer[AudioBufferSize];

const double CDF_THRESHOLD = 0.03;
const double PROB_THRESHOLD = 0.01;
const std::vector<int> GROUP={5,3,7};
const int SPECTRUM_FEAT_DIM = 15; //特征维数

const int FFTLengthPowerOfTwo = 13; //4096点 ~ 23ms（44100hz采样频率）
const int SpectrumLengthSamples = 1 << FFTLengthPowerOfTwo;


#define qcout qDebug()<< __FILE__<<__LINE__

#endif // COMMON_H
