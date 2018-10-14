#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>

QSemaphore VideoBufUsed;
QSemaphore VideoBufFree;
QImage VideoBuffer[VideoBufferSize];

MainWidget::MainWidget(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWidget),
	setting(new Setting(this)),
	adsltSetting(new AdSelectSetting(this)),
	lbmngDialog(new LabelMngDialog(this)),
	audioWidget(new AudioWidget(this)),
	audioBase(new AudioBase(this)),
	server(new AudioServer(this)),
	videoWidget(new VideoWidget(this)),
	labelConnection(new QLabel(this)),
	localAudio(new LocalAudioWarper("localhost"))
{
    ui->setupUi(this);
	initLocalAudio();

	server->listen(QHostAddress::Any, 8888);
	
    //初始化UI
	connect(server, &QTcpServer::newConnection, [&]()
	{
		std::unique_ptr<QThread> thread(server->getSocketThread());
		
		std::unique_ptr<QTcpSocket> socket(server->nextPendingConnection());
		QString ip = socket->peerAddress().toString();
		emit socketConnected(ip);
		labelConnection->setText(ip);

        TcpAudioWarper *warper = new TcpAudioWarper(ip, std::move(socket));
		
		qRegisterMetaType<AudioFormat>("AudioFormat");
		connect(warper, &TcpAudioWarper::audioFormatInit, setting, &Setting::onAudioFormatInit);
		qRegisterMetaType<QVector<double>>("QVector<double>");
		connect(warper, &TcpAudioWarper::curvePainted, audioWidget, &AudioWidget::onCurvePainted);
		connect(warper, &TcpAudioWarper::labelPredicted, audioWidget, &AudioWidget::onLabelPredicted);
		connect(audioWidget, &AudioWidget::commandIssued, warper, &TcpAudioWarper::onCommandIssued);
		connect(warper, &TcpAudioWarper::alreadyTrained, audioWidget, &AudioWidget::onAlreadyTrained);

		connect(audioWidget, &AudioWidget::paintStart, warper, &TcpAudioWarper::onPaintStart);
		connect(audioWidget, &AudioWidget::recordStart, warper, &TcpAudioWarper::onRecordStart);
		connect(audioWidget, &AudioWidget::recordStop, warper, &TcpAudioWarper::onRecordStop);
        qRegisterMetaType<QVector<QString>>("QVector<QString>");
        qRegisterMetaType<QVector<int>>("QVector<int>");
        connect(audioWidget, &AudioWidget::trainModel, warper, &TcpAudioWarper::onTrainModel);

		warper->moveToThread(thread.get());
		connect(thread.get(), &QThread::finished, thread.get(), &QThread::deleteLater);
		threads.push_back(std::move(thread));
	});
    ui->stackedWidget->insertWidget(AUDIO_WIDGET_INDEX, audioWidget);
    ui->stackedWidget->insertWidget(VIDEO_WIDGET_INDEX, videoWidget);
    ui->stackedWidget->setCurrentIndex(AUDIO_WIDGET_INDEX);
    setCentralWidget(ui->stackedWidget);

    //设置窗口菜单栏，标题，图标等
    designMenu();
    setWindowTitle(tr("CVPR"));

    connectUI();

}

MainWidget::~MainWidget()
{
    delete ui;
	for (auto &thread : threads)
	{
		thread->quit();
		thread->wait();
	}
}

void MainWidget::designMenu(void)
{
    QMenuBar *m_menuBar = menuBar();
    setMenuBar(m_menuBar);

    QMenu *menuFile = m_menuBar->addMenu(QString::fromLocal8Bit("文件"));
    QMenu *menuDevice = m_menuBar->addMenu(QString::fromLocal8Bit("设置"));

    QAction *actionOpen = menuFile->addAction(QString::fromLocal8Bit("打开文件"));

    //菜单栏，”文件“下”打开“
    connect(actionOpen, &QAction::triggered,
            [=]()
    {
        //打开文件，将音频或视频且文件信息显示到音频曲线或视频窗口
        QString name = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("请选择wav文件"), QDir::currentPath(),
                                                    tr("AUDIO files (*.wav);; VIDEO files (*.avi)"));
//        cout << "name " <<name;
//        if(tcpSocket != NULL)
//        {
//            //关闭声音采集
//            QByteArray array;
//            QDataStream stream(&array, QIODevice::WriteOnly);
//            stream << QString("stopAudio");
//            tcpSocket->write(array);
//        }
        //显示曲线
        if(name.isEmpty() == false)
            emit wavFileOpened(name);
    });
    

    QAction *md1 = menuDevice->addAction(QString::fromLocal8Bit("设备信息"));
    connect(md1, &QAction::triggered,
            [=]()
    {

        //音频设备设置对话框
//        setting->designWidget(audioformats);
        setting->show();
    });
    QAction *md2 = menuDevice->addAction(QString::fromLocal8Bit("音频设备注册"));
    connect(md2, &QAction::triggered,
            [=]()
    {
        adsltSetting->show();
    });
    QAction *md3 = menuDevice->addAction(QString::fromLocal8Bit("音频标签注册"));
    connect(md3, &QAction::triggered,
            [=]()
    {
        lbmngDialog->show();
    });

    //显示终端连接信息，显示ip，port
    labelConnection->setText(QString(QString::fromLocal8Bit("未连接")));
    ui->statusBar->addWidget(labelConnection);

}
void MainWidget::connectUI()
{
    //切换界面
    connect(audioWidget, &AudioWidget::widgetSwitched, this, &MainWidget::onWidgetSwitched);
    connect(videoWidget, &VideoWidget::widgetSwitched, this, &MainWidget::onWidgetSwitched);
    //各种设置widget与audiowidget交互
    connect(adsltSetting, &AdSelectSetting::deviceRegistered, audioWidget, &AudioWidget::onDeviceRegistered);
	connect(lbmngDialog, &LabelMngDialog::labelRegistered, audioWidget, &AudioWidget::onLabelRegistered);
    connect(audioWidget, &AudioWidget::startAudioBase, localAudio, &LocalAudioWarper::onStartAudio);
    connect(audioWidget, &AudioWidget::stopAudioBase, localAudio, &LocalAudioWarper::onStopAudio);
	connect(this, &MainWidget::socketConnected, audioWidget, &AudioWidget::onSocketConnected);

	connect(this, &MainWidget::wavFileOpened, audioWidget, &AudioWidget::onWavFileOpened);
    // 更新保存路径后通知需要保存文件的类
//    connect(videoWidget, &VideoWidget::imageSaved, vplot_thread, &VideoPlotThread::onImageSaved);
    //摄像头显示
//    connect(vplot_thread, &VideoPlotThread::framePresented, videoWidget, &VideoWidget::onFramePresented);
}
/**
 * @brief tcp warper的start audio, stop audio, params
 */

void MainWidget::onWidgetSwitched(int widget_id)
{
    ui->stackedWidget->setCurrentIndex(widget_id);

}
void MainWidget::initLocalAudio()
{
    std::unique_ptr<QThread> thread(new QThread(this));
    localAudio->moveToThread(thread.get());
 
    qRegisterMetaType<AudioFormat>("AudioFormat");
    connect(localAudio, &LocalAudioWarper::audioFormatInit, setting, &Setting::onAudioFormatInit);
    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(localAudio, &LocalAudioWarper::curvePainted, audioWidget, &AudioWidget::onCurvePainted);
    connect(localAudio, &LocalAudioWarper::labelPredicted, audioWidget, &AudioWidget::onLabelPredicted);
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
	connect(audioWidget, &AudioWidget::trainModel, localAudio, &LocalAudioWarper::onTrainModel);
	connect(localAudio, &LocalAudioWarper::alreadyTrained, audioWidget, &AudioWidget::onAlreadyTrained);

    connect(audioWidget, &AudioWidget::paintStart, localAudio, &LocalAudioWarper::onPaintStart);
    connect(audioWidget, &AudioWidget::recordStart, localAudio, &LocalAudioWarper::onRecordStart);
    connect(audioWidget, &AudioWidget::recordStop, localAudio, &LocalAudioWarper::onRecordStop);
    thread->start();
	threads.push_back(std::move(thread));
}
