#ifndef SPECTRUMHELPER_H
#define SPECTRUMHELPER_H

#include <QObject>
#include "spectrumanalyser.h"
#include "wavfile.h"
#include "gaussmodel/tensor.h"
#include "gaussmodel/gaussianmg.h"
#include <qglobal.h>
#include <tuple>

class SpectrumHelper : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumHelper(const QString &id, QObject *parent = nullptr);
    ~SpectrumHelper();
    void train_from_file(const QVector<QString> &filenames, const QVector<int> &label);
    void train_from_bytes(const QByteArray &stream, int label, int sample_size, int unitByte);
    int predict_of_frame(const QByteArray& pos,int sample_size, int unitByte);

    const QByteArray params() const;
	const std::tuple<int, int> paramSize() const
	{
		return std::make_tuple<int, int>(m_featMean.rows(), m_featMean.cols());
	}
	bool isValid() const;

private :
    //更新模型参数
    void update_paramsters(const Tensor &, const Tensor &,const Tensor &);
    /*map: key:label, val: indexs */
    void group_by_label(const std::map< int, std::vector<int> > &label, const std::vector<Tensor> &src, std::vector<Tensor> &dst);
	void saveToFile();
	void loadFromFile();
private:
    //data, 行号是类别
    Tensor m_featMean;
    Tensor m_featStd;
    Tensor m_gate;
	QString dev_id;
    //fft
    SpectrumAnalyser *m_fft_analyser;
};

#endif // SPECTRUMHELPER_H
