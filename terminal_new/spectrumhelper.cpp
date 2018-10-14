#include "spectrumhelper.h"
#include <qmath.h>

Tensor bytes_to_tensor(const char* ptr, int row, int col);

SpectrumHelper::SpectrumHelper(QObject *parent) : QObject(parent)
  , m_fft(new ffft::FFTReal <double>(SpectrumLengthSamples))
{

}
SpectrumHelper::~SpectrumHelper()
{
    delete m_fft;
}
/**
 * @brief 对一帧数据进行类别预测
 * @param pos：数组中的位置
 * @return -2: 未进行预测  -1：未知类别  其他：具体类
 */
int SpectrumHelper::predict_of_frame(const QByteArray &data, int sample_size, int unitByte)
{

    if(m_featMean.rows() == 0)//表示训练过模型
        return -2;
    Tensor testdata = calcSpectrum_of_frame(data, sample_size, unitByte, SPECTRUM_FEAT_DIM);

    Tensor pred = gausianTest(testdata, m_featMean, m_featStd, m_gate, CDF_THRESHOLD, GROUP);

    return pred(0,0);
}
/**
 * @brief 只计算一帧数据（SpectrumLengthSamples）的fft，将总的音频分开处理
 * @param audioStream
 * @param unitBytes
 * @param featDim
 * @return
 */
Tensor SpectrumHelper::calcSpectrum_of_frame(const QByteArray &audioStream,
                                         int sample_size,
                                         int unitBytes,
                                         int featDim)
{
    Q_ASSERT_X(audioStream.size() == SpectrumLengthSamples*unitBytes, "" ,"array len is wrong");

    const int size = SpectrumLengthSamples; //去掉冗余长度后流的数据点长度
    /**
      统一对所有样本进行处理计算，减少运算量
      **/
    double *inputs = new double[size];
    Tensor outputs(1, size);
    Tensor specTensor(1, featDim);
    for(int i=0;i< size; i++)
    {
        if(sample_size == 16)
        {
             const qint16 d16 = *reinterpret_cast<const qint16*>(audioStream.data() + i*unitBytes);
            //scale data to [-1,1]
            inputs[i] = d16/ 32768.0;
        }
        else if(sample_size == 8)
        {
            const qint8 d8 = *(audioStream.data() + i*unitBytes);
            //scale data to [-1,1]
            inputs[i] = d8/ 128.0;
        }
    }
    //对每个样本fft

    m_fft->do_fft(outputs.begin(), inputs);
    //post process
    postProcess(outputs, specTensor);
    delete[] inputs;

    return specTensor;
}
/*
 * 对FFT结果求频率对应的幅值，并将[0,fs/2]分割成20个小频率区间求平均幅值，也就是20维特征，
 * @param:
 *  输入tensor ， 输出tensor
 */
void SpectrumHelper::postProcess(const Tensor &inputs, Tensor &outputs)
{
    /**
    输入len长度得到也是输出长度（偶数），FFT返回结果是对称的，第一个元素为直流量，后面（1，（len-2）/2）,((len-2)/2+2,len-1)
    是实部一样，虚部相反数的复数，中间那个数也是直流量。ffft库考虑到这一点，
    （1，（len-2）/2）为存储实数部分，((len-2)/2+2,len-1)存储虚数部分，中间那个值不变。
    输出处理：求模后乘2除以len得到真实幅度。
    由乃奎斯特定理，频域N/2两部分对称，所以只用到前面部分表示的频率范围是[0, fs/2],fs: 采样频率
        **/
    double real=0;
    double imag=0;

    Q_ASSERT_X(inputs.rows()==outputs.rows() && inputs.cols()== SpectrumLengthSamples,
               "SpectrumAnalyser::postProcess",
               "inputs shape is different from outputs or inputs cols is wrong!");
    int bin_len = int((SpectrumLengthSamples/2) / outputs.cols());
    int sample_num = inputs.rows();
    double sum =0;//每个bin的和

    for(int i=0; i< sample_num;i++)
    {
        sum = qAbs(inputs(i, 0));
        //求摸
        int j = 1;
        for(; j< bin_len * outputs.cols(); j++)//去除冗余后的长度
        {
            real = inputs(i, j);
            imag = inputs(i, j + SpectrumLengthSamples / 2);
            sum += qSqrt(real * real + imag * imag);

            if((j+1) % bin_len == 0) //如果遍历到区间最后一个元素，对sum求均值
            {
                outputs(i, j / bin_len) = sum / bin_len;
                sum = 0;
            }
        }
        //剩余元素扔进最后一个bin
        sum = 0;
        for(; j< SpectrumLengthSamples / 2; j++)
        {
            real = inputs(i, j);
            imag = inputs(i, j + SpectrumLengthSamples / 2);
            sum += qSqrt(real * real + imag * imag);
        }
        sum += qAbs(inputs(i, SpectrumLengthSamples/2));
        outputs(i, outputs.cols()-1) = (outputs(i, outputs.cols()-1)*bin_len + sum)
                                                / (bin_len + (SpectrumLengthSamples/2) % outputs.cols() + 1);

    }
}
void SpectrumHelper::updateParamester(const QByteArray &arr, int row, int col)
{
    int unitebyte = sizeof(double);
    if(!(arr.size() == 2*unitebyte*row*col + unitebyte*row))
    {
        qDebug() << arr.size() <<" "<< row  <<" " <<col;
    }
    Q_ASSERT(arr.size() == 2*unitebyte*row*col + unitebyte*row);
    m_featMean  =bytes_to_tensor(arr.constData(), row, col);
    m_featStd  =bytes_to_tensor(arr.constData() + unitebyte*row*col, row, col);
    m_gate  =bytes_to_tensor(arr.constData() + 2*unitebyte*row*col, 1, row);

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

    for(int i=0; i<row; i++)
    {
        for(int j=0;j < col; j++)
            ret(i,j) = *reinterpret_cast<const double *>(ptr + unitbyte*(i*col + j));
    }
    return ret;
}
