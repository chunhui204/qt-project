#ifndef SPECTRUMHELPER_H
#define SPECTRUMHELPER_H

#include <QObject>
#include "gaussmodel/gaussianmg.h"
#include "gaussmodel/tensor.h"
#include "ffft/FFTReal.h"
#include <QDebug>
#include "common.h"

class SpectrumHelper : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumHelper(QObject *parent = nullptr);
    ~SpectrumHelper();
    int predict_of_frame(const QByteArray &data, int sample_size, int unitByte);
    void updateParamester(const QByteArray &arr, int row, int col);
private:
    Tensor calcSpectrum_of_frame(const QByteArray &, int, int, int);
    void postProcess(const Tensor &inputs, Tensor &outputs);

private:
    ffft::FFTReal <double> *m_fft;
    //spectrum
    Tensor m_featMean;
    Tensor m_featStd;
    Tensor m_gate;
};


#endif // SPECTRUMHELPER_H
