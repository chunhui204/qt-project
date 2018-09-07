#include "audiowidget.h"
#include "ui_audiowidget.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <string>
#include <QComboBox>

std::vector<std::string> split_stdString(const std::string &str, char ch);

AudioWidget::AudioWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioWidget)
{
    ui->setupUi(this);
    initUI();

    //其他初始化
    saveDir = QDir::currentPath();

    audioBase = new AudioBase;
    captureThread = new QThread(this);
    audioBase->moveToThread(captureThread);
    connect(captureThread, &QThread::finished, audioBase, &AudioBase::deleteLater);

    connect(this, &AudioWidget::audioFormatChanged,audioBase, &AudioBase::onAudioFormatChanged);
//    connect()
    captureThread->start();
}
void AudioWidget::initUI()
{
    // 初始化绘图组件customPlot
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
//    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    //y周范围
    ui->customPlot->yAxis->setRange(-1,1);//(-32768, 32768);
    ui->customPlot->xAxis->setRange(0, 5, Qt::AlignLeft);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->axisRect()->setupFullAxesBox();
    // 上方和右边坐标显示
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
//    ui->customPlot->graph(0)->rescaleAxes();
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
    ui->customPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->combo_audio->setInsertPolicy(QComboBox::InsertAtCurrent);
//    connect(ui->combo_audio, &QComboBox::currentIndexChanged, [=]()
//    {
//        qcout<< ui->combo_audio->currentIndex() << ui->combo_audio->currentText();
//    });
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
}
AudioWidget::~AudioWidget()
{
    delete ui;
    captureThread->quit();
    captureThread->wait();
}
/**
 * 1.如果保证绘图线程处理速度尽量与接收线程同步，那么曲线和计算的label就是匹配的，（为了实时性应该是同步的）
 * 2.当接受完整个buffer长度才计算label会有些滞后性，如果一般数据就能预测那会实时性强
 */
void AudioWidget::onLabelPredicted(int label)
{
    if(label == -1)
        ui->label_audio_class->setText(QString::fromLocal8Bit("未知类别"));
    else if(label == -2)
        ui->label_audio_class->setText(QString::fromLocal8Bit("未进行预测"));
    else
        ui->label_audio_class->setText(QString::fromLocal8Bit("类别")+QString::number(label, 10));
}

void AudioWidget::onDataProcessed(QVector<double> xs, QVector<double> ys)
{
        if(xs.isEmpty())
        {
            ui->customPlot->graph(0)->setData(xs, xs); // 清空界面
        }
        else
        {
            for(int i=0; i< xs.size() ; i++)
            {
                ui->customPlot->graph(0)->addData(xs[i], ys[i]);
            }
            ui->customPlot->xAxis->setRange(xs[0], 5, Qt::AlignHCenter);
        }

    ui->customPlot->replot();
}
/**
 * @brief 开启终端麦克风
 */
void AudioWidget::on_button_audio_capStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    ui->button_local_start->setDisabled(true);
    ui->button_local_end->setDisabled(true);

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString("startAudio");
    emit commandIssued(array);
}
/**
 * @brief 关闭终端麦克风
 */
void AudioWidget::on_button_audio_capEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
    ui->button_local_start->setDisabled(false);
    ui->button_local_end->setDisabled(false);
    emit recordStop();

    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);
    stream << QString("stopAudio");
    emit commandIssued(array);
}
/**
 * @brief 开启录音
 */
void AudioWidget::on_button_audio_recordStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(false);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
    //拼接路径
    qDir.cd(saveDir);
//    cout << saveDir;
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_FILE_PATH) == false)
        qDir.mkdir(AUDIO_FILE_PATH);
    qDir.cd(AUDIO_FILE_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
    emit recordStart(name);
}
/**
 * @brief 结束录音
 */
void AudioWidget::on_button_audio_recordEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    emit recordStop();
}
/**
 * @brief 保存路径改变
 * @param dir
 */
void AudioWidget::onSavePathChanged(const QString &dir)
{
    saveDir = dir;
}
/**
 * @brief 开始采集样本,和录音不能同时进行
 */
void AudioWidget::on_button_audio_sampleStart_clicked()
{
    //get label
    std::string str = ui->combo_audio->currentText().toStdString();
    if(str.empty())
    {
        return;
    }

    std::vector<std::string> vec = split_stdString(str, ' ');

    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(false);
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    //拼接路径
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_TRAIN_PATH) == false)
        qDir.mkdir(AUDIO_TRAIN_PATH);
    qDir.cd(AUDIO_TRAIN_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + QString::fromStdString(vec[0])
            + "_" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
    qcout << name;

    emit recordStart(name);
}
/**
 * @brief 结束采集
 */
void AudioWidget::on_button_audio_sampleEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    emit recordStop();
}
/**
 * @brief 添加label信息
 */
void AudioWidget::on_button_audio_labelAdd_clicked()
{
    QString text = ui->lineEdit_audio->text().simplified();//delete duplicate spaces
    ui->lineEdit_audio->clear();

    if(text.isEmpty())
        return;
    if(ui->combo_audio->findText(text) == -1)
        ui->combo_audio->addItem(text);

}

void AudioWidget::on_button_audio_train_clicked()
{
    ui->button_audio_train->setDisabled(true);
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == true)
        qDir.cd(AUDIO_PATH);
    if(qDir.exists(AUDIO_TRAIN_PATH) == true)
        qDir.cd(AUDIO_TRAIN_PATH);

    QStringList files = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择训练文件"),
                            qDir.absolutePath(), tr("AUDIO files (*.wav)"));
    if(false == files.isEmpty())
    {
        QVector<QString> filenames;
        QVector<int> labels;
        for(QStringList::iterator i=files.begin();i!=files.end();i++)
        {
            filenames.push_back(*i);

            std::vector<std::string> name = split_stdString((*i).toStdString(), '/');
            std::vector<std::string> label_t = split_stdString(name[name.size()-1], '_');
            labels.push_back( std::stoi(label_t[0]));
        }
        emit trainModel(filenames, labels);
    }
}
void AudioWidget::onAlreadyTrained(QByteArray arr, int row, int col)
{
    ui->button_audio_train->setDisabled(false);
    QByteArray snd;
    QDataStream ss(&snd, QIODevice::WriteOnly);
    ss << QString("paramester")
       << qint64(row) << qint64(col) << arr;
    qcout << row<< col<< arr.size();
    emit commandIssued(snd);
}
/**
 * @brief AudioWidget::onAudioFormatChanged
 * @param format
 */
void AudioWidget::onAudioFormatChanged(AudioSettingFormat format)
{
    ui->button_audio_capStart->setDisabled(false);
    ui->button_audio_capEnd->setDisabled(false);
    ui->button_local_start->setDisabled(false);
    ui->button_local_end->setDisabled(false);
    ui->button_audio_recordStart->setDisabled(true);
    ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);

    audioBase->stopAudio();
    emit recordStop();
    emit audioFormatChanged(format);
}
/**
 * @brief split_stdString by charactor, util function
 * @param str
 * @param vec
 */
std::vector<std::string> split_stdString(const std::string &str, char ch)
{
    std::vector<std::string> vec;
    std::string::const_iterator start = str.begin(), end = str.begin();

    while(end != str.end())
    {
        while(end != str.end() && *end == ch )
            end++;
        start = end;

        while(end != str.end() && *end != ch)
            end++;

        if(end != start)
        {
            //valid substr
            vec.push_back(std::string(start, end));
        }
    }
    return vec;
}
/**
 * @brief 本地采集
 */

void AudioWidget::on_button_local_start_clicked()
{
    ui->button_audio_capStart->setDisabled(true);
    ui->button_audio_capEnd->setDisabled(true);
    ui->button_audio_recordStart->setDisabled(false);
    ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(false);
    ui->button_audio_sampleEnd->setDisabled(true);
    audioBase->startAudio();
    QByteArray info = audioBase->audioInfo();
    emit localAudioInfo(info);
}

void AudioWidget::on_button_local_end_clicked()
{
    ui->button_audio_capStart->setDisabled(false);
    ui->button_audio_capEnd->setDisabled(false);
    ui->button_audio_recordStart->setDisabled(true);
    ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart->setDisabled(true);
    ui->button_audio_sampleEnd->setDisabled(true);
    emit recordStop();

    audioBase->stopAudio();
}
