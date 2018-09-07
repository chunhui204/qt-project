#ifndef SPECTRUMHELPER_H
#define SPECTRUMHELPER_H

#include <QObject>
#include "spectrumanalyser.h"
#include "wavfile.h"
#include "gaussmodel/tensor.h"
#include "gaussmodel/gaussianmg.h"
#include <qglobal.h>

class SpectrumHelper : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumHelper(QObject *parent = nullptr);
    ~SpectrumHelper();
    void train_from_file(const QVector<QString> &filenames, const QVector<int> &label);
    void train_from_bytes(const QByteArray &stream, int label, int sample_size, int unitByte);
    int predict_of_frame(int pos,int sample_size, int unitByte);

    QByteArray featMean() const;
    QByteArray featStd() const;
    QByteArray gate() const;

private :
    //更新模型参数
    void update_paramsters(const Tensor &, const Tensor &,const Tensor &);
    /*map: key:label, val: indexs */
    void group_by_label(const std::map< int, std::vector<int> > &label, const std::vector<Tensor> &src, std::vector<Tensor> &dst);

private:
    //data, 行号是类别
    Tensor m_featMean;
    Tensor m_featStd;
    Tensor m_gate;
//    QMap<int, int> m_labelIdx; //key为类别，值为在上述m_featMean中的行索引

    //fft
    SpectrumAnalyser *m_fft_analyser;
    //prediction
    std::map<int, int> m_prediction_count;//key:label   val:count
};

#endif // SPECTRUMHELPER_H
