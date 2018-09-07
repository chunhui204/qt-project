#include "spectrumhelper.h"

QByteArray tensor_to_bytes(const Tensor &obj);

SpectrumHelper::SpectrumHelper(QObject *parent) : QObject(parent)
{
    m_fft_analyser = new SpectrumAnalyser(this);
    m_prediction_count.clear();
}

SpectrumHelper::~SpectrumHelper()
{
//    delete m_fft_analyser;
}

/**
 * @brief 对一帧数据进行类别预测
 * @param pos：数组中的位置
 * @return -2: 未进行预测  -1：未知类别  其他：具体类
 */
int SpectrumHelper::predict_of_frame(int pos, int sample_size, int unitByte)
{
    if(m_featMean.rows() == 0)//表示训练过模型
        return -2;
    //一次性使用Audio buffer所有数据,当数据未连续充满一个数组长度时程序阻塞，保证数据的连续性
    AudioMutex.lock();
    QByteArray data(AudioBuffer + pos, qMin(SpectrumLengthSamples * unitByte, AudioBufSize-pos));

    if(SpectrumLengthSamples*unitByte > AudioBufSize - pos)
    {
        QByteArray tm(AudioBuffer, SpectrumLengthSamples*unitByte - (AudioBufSize-pos));
        data.append(tm);
    }
    AudioMutex.unlock();

    Tensor testdata = m_fft_analyser->calcSpectrum_of_frame(data, sample_size, unitByte, SPECTRUM_FEAT_DIM);
    assert(testdata.rows()==1 && testdata.cols()==SPECTRUM_FEAT_DIM
           && "testdata shape is wrong");
    Tensor pred = gausianTest(testdata, m_featMean, m_featStd, m_gate, CDF_THRESHOLD, GROUP);
    Q_ASSERT_X(pred.rows()==1 && pred.cols()==1 ,"","pred shape wrong");

//    qcout <<"pred unitbyte "<< unitByte;
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

        }
        QByteArray array = file.readAll();
        int unitByte = file.fileFormat().channelCount() * file.fileFormat().sampleSize() / 8;
        file.close();

        if(array.size() < Min_SpectrumTime_Train* unitByte)
            continue; //过小文件
        Q_ASSERT_X(array.size()/unitByte/44100 > 10,"","file too little");//useless

        Tensor data = m_fft_analyser->calculateSpectFeature(array, file.fileFormat().sampleSize(), unitByte, SPECTRUM_FEAT_DIM);
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

//    Tensor pred = gausianTest(traindata_merged[0].row_vector(3), m_featMean, m_featStd, m_gate, CDF_THRESHOLD, GROUP);
//    print(pred);
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
    m_featMean = featMean;
    m_featStd = featStd;
    m_gate = gate;

//    print(gate);
//    std::cout<<"-----------"<<std::endl;
//    print(featMean);
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

QByteArray SpectrumHelper::featMean() const
{
    return tensor_to_bytes(m_featMean);
}
QByteArray SpectrumHelper::featStd() const
{
    return tensor_to_bytes(m_featStd);
}
QByteArray SpectrumHelper::gate() const
{
    return tensor_to_bytes(m_gate);
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
