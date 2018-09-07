#ifndef SPECTRUMANALYSER_H
#define SPECTRUMANALYSER_H

#include <QObject>
#include <ffft/FFTReal.h>
#include <gaussmodel/gaussianmg.h>
#include <gaussmodel/tensor.h>
#include "common.h"
#include <QVector>
#include <QTime>
/**
 * 处理来自不同数据格式的数据，针对不同samplesize做处理，
 *clac fft中new数组会不会引起bad alloc，可使用vector ys直接传入
 */

class SpectrumAnalyser : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumAnalyser(QObject *parent=0);
    ~SpectrumAnalyser();

public:
    Tensor calculateSpectFeature(const QByteArray & ,int, int ,int );
    Tensor calcSpectrum_of_frame(const QByteArray & ,int, int ,int );

private:
    QVector<int> splitAudioStream(const QByteArray &, int);
    void postProcess(const Tensor &inputs, Tensor &outputs);

private:
    ffft::FFTReal <double> *m_fft;
	QTime time;
};

#endif // SPECTRUMANALYSER_H
