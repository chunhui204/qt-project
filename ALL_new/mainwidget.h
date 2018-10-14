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
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>

#include <QStackedWidget>
#include <QTimer>
#include <memory>
#include <QMap>
#include "setting.h"
#include "videodatathread.h"
#include "spectrumthread.h"
#include "videowidget.h"
#include "videoplotthread.h"
#include "common.h"
#include "adselectsetting.h"
#include "labelmngdialog.h"
#include "audioserver.h"
#include "audiowidget.h"
#include "audiobase.h"
#include "localaudiowarper.h"
#include <vector>

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
    void onWidgetSwitched(int widget_id);

signals:
	void socketConnected(const QString &ip);
    void savePathChanged(const QString & dir);
    void wavFileOpened(const QString &name);
private:
    //ui
    Ui::MainWidget *ui;
    QLabel *labelConnection;
    Setting *setting;
    AudioWidget *audioWidget;
    AdSelectSetting *adsltSetting;
    LabelMngDialog *lbmngDialog;
    VideoWidget *videoWidget;
	AudioBase *audioBase;
    LocalAudioWarper *localAudio;
	AudioServer *server;
    //
    QString saveRootPath;
    QString saveAudioPath;
    QString saveVideoPath;
	std::vector<std::unique_ptr<QThread> > threads;


private:
    void designMenu(void);
    void connectUI();
	void initLocalAudio();

//private signals:
};

#endif // MAINWIDGET_H
