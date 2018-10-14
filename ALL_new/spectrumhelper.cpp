#include "spectrumhelper.h"
#include <QDir>
QByteArray tensor_to_bytes(const Tensor &obj);
Tensor bytes_to_tensor(const char* ptr, int row, int col);

SpectrumHelper::SpectrumHelper(const QString &id, QObject *parent) :
	QObject(parent),
	dev_id(id),
	m_fft_analyser(new SpectrumAnalyser(this))
{
	loadFromFile();
}

SpectrumHelper::~SpectrumHelper()
{
}
bool SpectrumHelper::isValid() const
{
	return m_featMean.rows() > 0;
}
/**
 * @brief 对一帧数据进行类别预测
 * @param pos：数组中的位置
 * @return -2: 未进行预测  -1：未知类别  其他：具体类
 */
int SpectrumHelper::predict_of_frame(const QByteArray& data, int sample_size, int unitByte)
{
    //一次性使用Audio buffer所有数据,当数据未连续充满一个数组长度时程序阻塞，保证数据的连续性

    Tensor testdata = m_fft_analyser->calcSpectrum_of_frame(data, sample_size, unitByte, SPECTRUM_FEAT_DIM);
    assert(testdata.rows()==1 && testdata.cols()==SPECTRUM_FEAT_DIM
           && "testdata shape is wrong");
    Tensor pred = gausianTest(testdata, m_featMean, m_featStd, m_gate, CDF_THRESHOLD, GROUP);
    Q_ASSERT_X(pred.rows()==1 && pred.cols()==1 ,"","pred shape wrong");

    return pred(0,0);
}
/*
 * 从wav文件获取音频进行训练
 * 必须保证多次添加的数据类别是连贯的， 如0,1,0,2(0,1,2), 下次添加的数据类别不能跳过某个数字，如4,5（跳过了3）
 */
void SpectrumHelper::train_from_file(const QVector<QString> &filenames, const QVector<int> &label)
{
    Q_ASSERT_X(filenames.size()==label.size(),
               "SpectrumThread::train_from_file",
               "size not equal");
    std::vector<Tensor> traindata;
    std::map<int, std::vector<int> > arg_label;
    //sparse audio file to arrays
    WavFile file;
    int valid_indx = 0;
    for(int i = 0; i < filenames.size(); i++)
    {
        if(file.open(filenames[i], true)==false)
        {
            //error signals
			return;
        }
        QByteArray array = file.readAll();
        int unitByte = file.fileFormat().channel.toInt() * file.fileFormat().sampleSizes.toInt() / 8;
        file.close();

		if (array.size() < Min_SpectrumTime_Train* unitByte || unitByte <= 0)
		{
			qcout << "array.size()/unitByte/44100 must larger than 10,";
			continue; //过小文件
		}
        Tensor data = m_fft_analyser->calculateSpectFeature(array, file.fileFormat().sampleSizes.toInt(),
                                                            unitByte, SPECTRUM_FEAT_DIM);
        traindata.push_back(data);
            //先将标签-索引 映射表计算出来
        if(arg_label.find(label[i]) == arg_label.end())
        {
            arg_label[label[i]] = std::vector<int>();
        }

        arg_label[label[i]].push_back(valid_indx);

        valid_indx++;
    }

    if(traindata.size() == 0)
    {
        return;
    }
    std::vector<Tensor> traindata_merged;

    //相同类的数据合并到一个tensor,label是在vector索引
    group_by_label(arg_label, traindata, traindata_merged);

    int cls_num = traindata_merged.size();
    //行号不是类别信息，类别信息是label_args的key，第几行就是第几个key
    Tensor featMean(cls_num, SPECTRUM_FEAT_DIM);
    Tensor featStd(cls_num, SPECTRUM_FEAT_DIM);
    Tensor gate(1, cls_num);

    gausianTrain(traindata_merged, CDF_THRESHOLD, PROB_THRESHOLD, GROUP, featMean, featStd, gate);
    update_paramsters(featMean, featStd, gate);
	saveToFile();
}
/*
* 参数保存到文件
*/
void SpectrumHelper::saveToFile()
{
	QDir qDir;
	if (qDir.exists(AUDIO_PATH) == false)
		qDir.mkdir(AUDIO_PATH);
	qDir.cd(AUDIO_PATH);

	if (qDir.exists(AUDIO_PARAM_PATH) == false)
		qDir.mkdir(AUDIO_PARAM_PATH);
	qDir.cd(AUDIO_PARAM_PATH);
	QFile file(qDir.absolutePath() + "/" + dev_id + ".dat");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	auto size = paramSize();
	out << qint64(std::get<0>(size)) << qint64(std::get<1>(size)) << params();
	file.close();
}
/*
* 参数从文件中加载
*/
void SpectrumHelper::loadFromFile()
{
	QDir qDir;
	if (qDir.exists(AUDIO_PATH) == false)
		return;
	qDir.cd(AUDIO_PATH);
	if (qDir.exists(AUDIO_PARAM_PATH) == false)
		return;
	qDir.cd(AUDIO_PARAM_PATH);
	QFile file(qDir.absolutePath() + "/" +  dev_id + ".dat");
	qcout << file.exists();
	if (false == file.exists() || false == file.open(QIODevice::ReadOnly))
		return;
	QDataStream in(&file);
	qint64 row, col;
	QByteArray data;
	in >> row >> col >> data;

	int unitebyte = sizeof(double);

	m_featMean = bytes_to_tensor(data.constData(), row, col);
	m_featStd = bytes_to_tensor(data.constData() + unitebyte * row*col, row, col);
	m_gate = bytes_to_tensor(data.constData() + 2 * unitebyte*row*col, 1, row);
}
/*
 * 从音频流获取音频进行训练
 * NOTE:
 *      在数据处理线程获得字节流最大长度是AudioBuffer数组，必须保证数组长度(2s)足够训练用
 */
void SpectrumHelper::train_from_bytes(const QByteArray &stream, int label, int sample_size, int unitByte)
{
    if(stream.size() == 0)
    {
        return;
    }
    std::vector<Tensor> traindata;
    Tensor data = m_fft_analyser->calculateSpectFeature(stream, sample_size, unitByte, SPECTRUM_FEAT_DIM);
    traindata.push_back(data);

    std::map<int, std::vector<int> > arg_label;
    //先将标签-索引 映射表计算出来
    arg_label[label] = std::vector<int>();
    arg_label[label].push_back(0);

    int cls_num = traindata.size();
    //行号不是类别信息，类别信息是label_args的key，第几行就是第几个key
    Tensor featMean(cls_num, SPECTRUM_FEAT_DIM);
    Tensor featStd(cls_num, SPECTRUM_FEAT_DIM);
    Tensor gate(1, cls_num);
    //training...
    gausianTrain(traindata, CDF_THRESHOLD, PROB_THRESHOLD, GROUP, featMean, featStd, gate);
    update_paramsters(featMean, featStd, gate);
	saveToFile();
}
/*
 * 从音频流获取音频进行测试
 * NOTE:
 *      在数据处理线程获得字节流最大长度是AudioBuffer数组，必须保证数组长度(2s)足够测试用
 */

/*
 * 不需要以此提供全部类别的数据，可以增加新类数据的模式进行训练
 * 为模型参数提供更新操作
 * 实质上是动态数组释放再分配的过程，所以尽量得到所有类别的样本后在训练，
 * 而且产生内存碎片
 */

void SpectrumHelper::update_paramsters(const Tensor &featMean,
                       const Tensor &featStd,
                       const Tensor &gate)
{
	m_featMean = std::move(featMean);
    m_featStd = std::move(featStd);
    m_gate = std::move(gate);
}

void SpectrumHelper::group_by_label(const std::map< int, std::vector<int> > &arg_label,
                                    const std::vector<Tensor> &src,
                                    std::vector<Tensor> &dst)
{
    std::map< int, std::vector<int> >::const_iterator map_it = arg_label.begin();
    for(;map_it!=arg_label.end();map_it++)
    {
        std::vector<int>::const_iterator vec_it=map_it->second.begin();
        std::vector<Tensor> grp;
        for(;vec_it!=map_it->second.end();vec_it++)
            grp.push_back(src[*vec_it]);

        dst.push_back(stack_tensors(grp));
    }

}

const QByteArray SpectrumHelper::params() const
{
	QByteArray ret = tensor_to_bytes(m_featMean);
	ret.append(tensor_to_bytes(m_featStd));
	ret.append(tensor_to_bytes(m_gate));
    return ret;
}
/**
 * @brief tensor_to_bytes,小端模式：低字节-低地址
 * @param arr
 * @param len
 * @return
 */
QByteArray tensor_to_bytes(const Tensor &obj)
{
    const int unitbyte = sizeof(double);
    const int size = obj.rows() * obj.cols() * unitbyte;
    Tensor::const_iterator ptr = obj.begin();
    QByteArray ret(size, 0);

    for(int i=0; i< obj.rows()*obj.cols(); i++)
    {
        for(int j=0; j< unitbyte; j++)
        {
            ret[i*unitbyte + j]=((char*)(ptr + i))[j];
        }
    }
    return ret;
}
/**
 * @brief bytes_to_tensor,小端模式：低字节-低地址,必须保证ptr:ptr + sizeof(double)*row*col区间内信息有效
 * @param arr
 * @param len
 * @return
 */
Tensor bytes_to_tensor(const char* ptr, int row, int col)
{
	const int unitbyte = sizeof(double);
	Tensor ret(row, col);

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
			ret(i, j) = *reinterpret_cast<const double *>(ptr + unitbyte * (i*col + j));
	}
	return ret;
}
