#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "audiowidget.h"
#include "videowidget.h"
#include <QThread>
#include <QTcpSocket>
#include "audiobase.h"
#include "audiowarper.h"

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
    void on_button_connect_clicked();

    void on_button_disconnect_clicked();

    void on_button_switch_clicked();

private:
    Ui::MainWidget *ui;
    AudioWidget *audioWidget;
    VideoWidget *videoWidget;
    AudioWarper *audioWarper;
    QVector<QThread*> threads;
signals:
    void connectSocket(QString ip, int port);
    void disconnectSocket();
};

#endif // MAINWIDGET_H
