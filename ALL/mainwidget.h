#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QStatusBar>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "audiowidget.h"
#include "setting.h"
#include <QThread>
#include "audiodatathread.h"
#include "audioplotthread.h"
#include "videodatathread.h"
#include "spectrumthread.h"
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include "videowidget.h"
#include "videoplotthread.h"
#include <QStackedWidget>
#include <QTimer>

//widget在stackedwidget中的索引
const int AUDIO_WIDGET_INDEX = 0;
const int VIDEO_WIDGET_INDEX =1;

namespace Ui {
class MainWidget;
}

class MainWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void dealResponseFromClient();
    void parseAudioInfo(QByteArray response);//不能使用引用
    void onAudioFormatChanged( AudioSettingFormat);
    void onCommandIssued(const QByteArray &);

signals:
    void audioFormatInit(const QString &rate, const QString &chns, const QString &size);
    void savePathChanged(const QString & dir);
    void wavFileOpened(const QString &name);

    void spectrumTimeout();
private:
    //ui
    Ui::MainWidget *ui;
    QLabel *labelConnection;
    Setting *setting;
    AudioWidget *audioWidget;
    VideoWidget *videoWidget;

    //
    QString saveRootPath;
    QString saveAudioPath;
    QString saveVideoPath;

    //网络
    QTcpServer *tcpServer;
    QTcpSocket *tcpSocket;
    //线程
    QThread *audioThread, *videoThread, *aPlotThread, *vPlotThread;
    AudioDataThread *adata_thread;//recv audio data
    AudioPlotThread *aplot_thread;//plot audio
    VideoDataThread *vdata_thread;//recv vedio data
    VideoPlotThread *vplot_thread;//plot video

    //设备参数
    QVector<AudioSettingFormat> audioformats;

private:
    void designMenu(void);
    void connectUI();
    void initThread();
    void timeout();


//private signals:
};

#endif // MAINWIDGET_H
