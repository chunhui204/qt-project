#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include <QTime>
#include <QMessageBox>

QSemaphore sema_usedbytes(0);
QSemaphore sema_freebytes(AudioBufferSize);
char AudioBuffer[AudioBufferSize];

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    ui->lineEdit_ip->setText("127.0.0.1");
    //初始化套接字
    commandSocket = new QTcpSocket(this);

    //thread
    videoThread = new QThread(this);
    video_thread = new VideoThread;
    video_thread->moveToThread(videoThread);

    audioThread = new QThread(this);
    audio_thread = new AudioThread;
    audio_thread->moveToThread(audioThread);

    connect(videoThread, &QThread::finished, video_thread, &VideoThread::deleteLater);
    connect(audioThread, &QThread::finished, audio_thread, &AudioThread::deleteLater);

    audioBase = new AudioBase(this);
    videoBase = new VideoBase(this);

    //video
    connect(videoBase, &VideoBase::framePresented, video_thread, &VideoThread::onFramePresented);
    //audio
    connect(audioBase, &AudioBase::audioError, this, &MainWidget::onAudioError);
    connect(audioBase, &AudioBase::audioFormatInfo, audio_thread, &AudioThread::onAudioFormatInfo);

    connect(audio_thread, &AudioThread::labelCalculated, this, &MainWidget::onLabelCalculated);
    connect(this, &MainWidget::audioFormatChanged, audioBase, &AudioBase::onAudioFormatChanged);
    connect(this, &MainWidget::connectHost, audio_thread, &AudioThread::onConnectHosts);
    connect(this, &MainWidget::disconnectHosts, audio_thread, &AudioThread::onDisconnectHosts);
    connect(this, &MainWidget::paramesterUpdated, audio_thread, &AudioThread::onParamesterUpdated);

    connect(commandSocket, &QTcpSocket::connected, this, &MainWidget::dealConnection);
    connect(commandSocket, &QTcpSocket::readyRead, this, &MainWidget::dealCommandResponse);
    connect(commandSocket, &QTcpSocket::disconnected,
            [=]()
            {
                ui->textEdit->setText(QString::fromLocal8Bit("断开连接"));
//                audioBase->stopAudio();
//                videoBase->stopVideo();
            });

    audioThread->start();
    videoThread->start();
}

MainWidget::~MainWidget()
{
    delete ui;

    if(commandSocket->isValid())
    {
        commandSocket->disconnectFromHost();
        commandSocket->close();
    }
    if(audioThread->isRunning())
    {
        audioThread->quit();
        audioThread->wait();
    }
    if(videoThread->isRunning())
    {
        videoThread->quit();
        videoThread->wait();
    }
}

void MainWidget::on_button_connect_clicked()
{
    //连接服务器
    QString ip = ui->lineEdit_ip->text();

    if(ip.isEmpty() == false)
    {
        commandSocket->connectToHost(QHostAddress(ip), COMMAND_PORT);
        emit connectHost(ip);
    }
}
void MainWidget::dealConnection()
{
    ui->textEdit->setText(QString::fromLocal8Bit("连接成功!!!!"));
    //向服务器发送设备信息（麦克风和摄像头）
}

//解析服务器响应
/*
 * resetAudioFormat : 更新audio配置
 * resetVideoFormat : 更新视频配置
 * readyAudioData: 确认接受音频数据
 * readyVideoData: 确认接受视频数据
 * startAudio
 * startVideo
 * stopAudio
 * stopVideo
 */
void MainWidget::dealCommandResponse()
{
    QByteArray response = commandSocket->readAll();
    QDataStream stream(&response, QIODevice::ReadOnly);
    QString head;

    stream >> head;

    qcout <<"head:" <<head;

    if("resetAudioFormat" == head)
    {
        //更新audio配置
        ui->textEdit->append("更新配置成功");

        QString device, rate, chn, byte, codec;
        stream >> device >>rate >> chn >> byte >> codec;
        emit audioFormatChanged(device, rate, chn, byte, codec);
    }
    else if("resetVideoFormat" == head)
    {
        //更新video配置
    }
    else if("startAudio" == head)
    {
        audioBase->startAudio();
        QByteArray audioInfo = audioBase->audioInfoToTcp();
    //    audioInfo.append(videoBase->videoInfoToTcp());

        commandSocket->write(audioInfo);
    }
    else if("stopAudio" == head)
    {
        audioBase->stopAudio();
    }
    else if("startVideo" == head)
    {
        videoBase->startVideo();
    }
    else if("stopVideo" == head)
    {
        videoBase->stopVideo();
    }
    else if("paramester" == head)
    {
        qint64 row, col;
        QByteArray arr;
        stream >> row >> col >> arr;
        qcout <<"paramster "<<arr.size()<<row<<col;
        emit paramesterUpdated(arr, row, col);
    }

}
void MainWidget::onAudioError(QString str)
{
    ui->textEdit->append(str);
}
void MainWidget::onLabelCalculated(int label)
{
//    qcout <<"label: "<<label;
    if(label == -2)
        ui->label_audio->setText(QString::fromLocal8Bit("类别为：未进行预测"));
    else if(label == -1)
        ui->label_audio->setText(QString::fromLocal8Bit("类别为：未知类别"));
    else
        ui->label_audio->setText(QString::fromLocal8Bit("类别为： ")+ QString::number(label));
}


void MainWidget::on_pushButton_clicked()
{
    commandSocket->disconnectFromHost();
    emit disconnectHosts();
}
