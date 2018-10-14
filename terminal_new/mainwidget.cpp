#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QHostAddress>
#include <QComboBox>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget),
    audioWidget(new AudioWidget(this)),
    videoWidget(new VideoWidget(this)),
    audioWarper(new AudioWarper())
{
    ui->setupUi(this);
    ui->stackedWidget->insertWidget(0, audioWidget);
    ui->stackedWidget->insertWidget(1, videoWidget);
    ui->stackedWidget->setCurrentIndex(0);

    QThread *thread1 = new QThread(this);
    connect(thread1, &QThread::finished, thread1, &QThread::deleteLater);
    threads.push_back(thread1);
    audioWarper->moveToThread(thread1);
    thread1->start();

    qRegisterMetaType<QVector<double>>("QVector<double>");
    connect(audioWarper, &AudioWarper::curvePainted, audioWidget, &AudioWidget::onCurvePainted);
    connect(audioWarper, &AudioWarper::labelPredicted, audioWidget, &AudioWidget::onLabelPredicted);
    connect(this, &MainWidget::connectSocket,audioWarper, &AudioWarper::onConnectSocket);
    connect(this, &MainWidget::disconnectSocket, audioWarper, &AudioWarper::onDisconnectSocket);
}

MainWidget::~MainWidget()
{
    delete ui;
    for(auto t : threads)
    {
        t->quit();
        t->wait();
    }
}

void MainWidget::on_button_connect_clicked()
{
    QString ip = ui->ip_widget->getIpaddress();
    emit connectSocket(ip, 8888);
    ui->label_connection->setText(QString::fromLocal8Bit("已连接"));
}
void MainWidget::on_button_disconnect_clicked()
{
    emit disconnectSocket();
    ui->label_connection->setText(QString::fromLocal8Bit("未连接"));
}

void MainWidget::on_button_switch_clicked()
{
    if(ui->stackedWidget->currentIndex() == 0)
        ui->stackedWidget->setCurrentIndex(1);
    else
        ui->stackedWidget->setCurrentIndex(0);
}
