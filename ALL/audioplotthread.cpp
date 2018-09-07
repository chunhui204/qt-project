#include "audioplotthread.h"

AudioPlotThread::AudioPlotThread(QObject *parent) : QObject(parent)
{
    bufferpos = 0;
    recordpos = 0;
    length_for_spectrum = 0;
    pos_for_spectrum= 0;
    isPlotBefore = true;
    isSaveRecording = false;
    isSaveTraining = false;

    timer = new QTimer(this);
    wavFile = new WavFile(this);

    m_spectrum = new SpectrumHelper(this);
    connect(timer, &QTimer::timeout, this, &AudioPlotThread::timeout);
    //30ms刷新一次
    timer->start(RefreshTime);

}

AudioPlotThread::~AudioPlotThread()
{
    xs.clear();
    ys.clear();
    QVector<double>().swap(xs);
    QVector<double>().swap(ys);

    if(timer->isActive())
        timer->stop();
}

void AudioPlotThread::timeout()
{
    //还未设置format,说明未连上设备
    if(audioFormat.channelCount() == -1)
        return;
    static double now = 0;//time.elapsed() /1000.0;
    //显示时出现每段音频间有连线或不同时间段内分辨率不同都是因为横轴时间量有误
    //这里显示RefreshTime+50是尽快的显示做到实时效果，如果不加的话每段length取整会损失精度
    //，就是说每段显示的数据长度都比这个时间段该显示的少一点点时间长了会造成累计误差滞后严重。
    int length = audioFormat.channelCount() * audioFormat.sampleRate()
            *audioFormat.sampleSize() / 8 * (RefreshTime+5)/ 1000 ;

    length -= length % 2;
    Q_ASSERT(AudioBufUsed.available() % 2 == 0);

    length = qMin(length, AudioBufUsed.available());

    int unitByte = audioFormat.channelCount() *audioFormat.sampleSize() / 8;
    int pointNum = length / unitByte;

    if(isPlotBefore && length > 0)
    {
        clearCurves();
        isPlotBefore = false;
    }
    //写文件,在处理到数组最后的时将内容统一写入文件
    if(isSaveRecording && bufferpos + length >= AudioBufSize)
    {
        wavFile->writeWave(recordFile, audioFormat, AudioBuffer + recordpos, AudioBufSize - recordpos);
        recordpos = 0;
    }
    clearCurveMemory();

//    qcout << AudioBufUsed.available();
    AudioBufUsed.acquire(length);
    AudioMutex.lock();
    for(int i=0; i< pointNum; i++)
    {//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
        now = now + 1.0 / audioFormat.sampleRate();
        xs.push_back(now);

        if(audioFormat.sampleSize() == 8)
        {
            ys.push_back(AudioBuffer[bufferpos] / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        }
        else if(audioFormat.sampleSize() == 16)
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(AudioBuffer + bufferpos);
            ys.push_back(*ptr / 32768.0);
        }
        bufferpos = (unitByte + bufferpos) % AudioBufSize;
     }

    AudioMutex.unlock();
    AudioBufFree.release(length);
    /****************spectrum process****************************/
//    timeCount = QTime::currentTime();

    int frame_len = unitByte* SpectrumTimeForCalc ;//一帧的字节长度

    length_for_spectrum += length;      //积累的数据长度
    if(length_for_spectrum >= frame_len)//要处理的时间长度
    {
        int label = m_spectrum->predict_of_frame(pos_for_spectrum, audioFormat.sampleSize(), unitByte);

        length_for_spectrum = length_for_spectrum - frame_len;
        pos_for_spectrum = (pos_for_spectrum + SpectrumLengthSamples* unitByte) % AudioBufSize;
        emit labelPredicted(label);
    }
    /*************************************************************************/

    Q_ASSERT_X(xs.size()==ys.size(),"AudioPlotThread::timeout()",
               "different channel and samplesize out of 1 8,1 16,2 16");
    if(pointNum > 0)
    {//只在有值的情况下发送，防止之前的曲线被清空
        emit dataProcessed(xs, ys);
    }

}

void AudioPlotThread::onRecordStart(const QString &name)
{
    recordFile = name;
    isSaveRecording = true;
    recordpos = bufferpos;
}

void AudioPlotThread::onRecordStop()
{
    isSaveRecording = false;
    Q_ASSERT(bufferpos >= recordpos);

    if(recordFile.isEmpty() == false)
        wavFile->writeWave(recordFile, audioFormat, AudioBuffer + recordpos, bufferpos - recordpos);
    recordFile.clear();
}
//显示wav file的音频信号
void AudioPlotThread::onWavFileOpened(const QString &name)
{   //关闭录音
    onRecordStop();
    //声音采集已关闭，所以又处于绘制实时曲线之前
    isPlotBefore = true;

    WavFile file;
    if(false == file.open(name, true))
        return;

    QByteArray array = file.readAll();
    file.close();
	
//清空界面
    clearCurves();
    clearCurveMemory();

    double now = 0;
    quint32 length = file.dataLength();
    length -= length % 2;

    int unitByte = file.fileFormat().channelCount() * file.fileFormat().sampleSize() / 8;

    for(int i=0; i < length; i+=unitByte)
    {//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
        now = now + 1.0 / file.fileFormat().sampleRate();
        xs.push_back(now);
        if(unitByte == 1)
            ys.push_back(array[i] / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
        else if(unitByte == 2)
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(array.data() + i);
            ys.push_back(*ptr / 32768.0);//因为16位的范围-32767~32768
        }else
        {
            qint16 *ptr = reinterpret_cast<qint16 *>(array.data() + i);
            ys.push_back(*ptr / 32768.0);
        }
    }
    emit dataProcessed(xs, ys);
}

void AudioPlotThread::onAudioFormatInit(const QString &rate, const QString &chns, const QString &size)
{
    audioFormat.setSampleRate(rate.toInt());
    audioFormat.setSampleSize(size.toInt());
    audioFormat.setChannelCount(chns.toInt());
    qcout<< "onAudioFormatInit" <<rate<<size<<chns;
}
void AudioPlotThread::clearCurves()
{
    QVector<double> t;
    emit dataProcessed(t, t);
}
void AudioPlotThread::clearCurveMemory()
{
    //clear all elements, size()==0, capcity not change
    xs.clear();
    ys.clear();
}

void AudioPlotThread::onTrainModel(QVector<QString> filenames, QVector<int> label)
{
    m_spectrum->train_from_file(filenames, label);

    int col = SPECTRUM_FEAT_DIM;
    int row = m_spectrum->featMean().size() / sizeof(double) / col;

    QByteArray mean = m_spectrum->featMean();
    QByteArray stan = m_spectrum->featStd();
    QByteArray gate = m_spectrum->gate();
    mean = mean.append(stan).append(gate);
//    Tensor snd(2,3);

//    for(int i=0;i<2;i++)
//        for(int j=0;j<3;j++)
//            snd(i,j) = i*3+j;
//    QByteArray arr = tensor_to_bytes(snd);

//    row=2;
//    col=3;
    qcout <<"param "<< mean.size() << stan.size() << gate.size();
    emit alreadyTrained(mean, row, col);

}
