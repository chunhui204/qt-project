#ifndef AUDIOPLOTTHREAD_H
#define AUDIOPLOTTHREAD_H

#include <QObject>
#include "common.h"
#include <QTimer>
#include <QTime>
#include "wavfile.h"
#include <QDir>
#include "spectrumhelper.h"

const int RefreshTime = 30; //ms

/**
 * 在绘图处理的事件循环中，
 *      1.频繁分配/释放内存引起 bad_alloc  2.在信号槽函数拷贝大数组会造成UI线程来不及反应（一直在copy...）
 * 利用了vector将元素清理和空间清理分离，事件循环中只清理元素，由于显示容量变化次数不多，所以内存重新分配的次数也不多
 * 不会引起bad_alloc，拷贝的数组是显示数据实际长度，不会阻塞UI。
 */
class AudioPlotThread : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlotThread(QObject *parent = 0);
    ~AudioPlotThread();

public slots:
    void onAudioFormatInit(const QString &rate, const QString &chns, const QString &size);
    void onRecordStart(const QString &);
    void onRecordStop();
    void onWavFileOpened(const QString &name);

    void onTrainModel( QVector<QString> filenames,  QVector<int> label);
//    void
signals:
    void dataProcessed( QVector<double> ,  QVector<double> );
    void labelPredicted(int label);
    void alreadyTrained(QByteArray, int , int);

private slots:
    void timeout();
private:
    void clearCurves();
    void clearCurveMemory();
private:
    int bufferpos;
    QTimer *timer;
    QTime timeCount;
    bool isPlotBefore; //指定是否为绘图之前，一旦开始绘图此标志位false

    QAudioFormat audioFormat;
    bool isFormatReady;
    //保存录音
    WavFile *wavFile;
    bool isSaveRecording; //保存录音？
    bool isSaveTraining;  //保存训练样本？
    QString recordFile;
    int recordpos;

    QVector<double> xs, ys;
    //spectrum
    SpectrumHelper *m_spectrum;
    int length_for_spectrum;//已处理过的AudioBuffer数组长度
    int pos_for_spectrum; //处理数据在AudioBuffer数组中的位置
};

#endif // AUDIOPLOTTHREAD_H
