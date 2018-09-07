#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include "audiobase.h"
#include <QThread>
#include "videobase.h"
#include "videothread.h"
#include "audiothread.h"
#include "common.h"

const int COMMAND_PORT = 8888;
const int AUDIO_PORT = 8889;
const int VIDEO_PORT = 8890;

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void dealConnection();
    void dealCommandResponse();
    void on_button_connect_clicked();
    void onAudioError(QString str);
    void onLabelCalculated(int label);

    void on_pushButton_clicked();

private:
    Ui::MainWidget *ui;
    QTcpSocket *commandSocket;
    AudioBase *audioBase;
    VideoBase *videoBase;

    //thread
    QThread *videoThread, *audioThread;
    VideoThread *video_thread;
    AudioThread *audio_thread;

signals:
    void paramesterUpdated(QByteArray, int, int);
    void connectHost(QString ip);
    void disconnectHosts();
    void audioFormatChanged(const QString &,const QString &,const QString &,const QString &,const QString &);
};

#endif // MAINWIDGET_H
