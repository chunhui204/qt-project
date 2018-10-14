#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>
#include <QDir>
#include "gaussmodel/tensor.h"
#include "audiobase.h"
#include <QThread>
#include <QSet>
#include <QButtonGroup>
#include <QRadioButton>
#include <memory>
#include "audiowarper.h"

namespace Ui {
class AudioWidget;
}

class AudioWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioWidget(QWidget *parent = 0);
    ~AudioWidget();

private:
    void initUI();
    void paintDgnDevs(const AudioDevice& devs);
    void paintShowDevs(const AudioDevice& devs);
private:
    Ui::AudioWidget *ui;
    //文件保存
    QString saveDir;        //ROOT目录路径
    QDir qDir;
    int num_trainedDev;
    QButtonGroup *checkboxGrp, *radiobtnGrp;
	AudioDevice registeredDevs;
	QSet<QString> connectedIps;
	QStringList diagnosedIps;
	QString curveIp;
    QMap<int, QString> registeredLabs;
    //data

public slots:
	void onWavFileOpened(const QString &filename);
    void onCurvePainted(QVector<double> xs, QVector<double> ys);
	void onSocketConnected(const QString &); //设备连接时
    void onLabelPredicted(int , bool, QString );
    void onAlreadyTrained();
    void onDeviceRegistered(const AudioDevice& devs); //注册设备时
    void onLabelRegistered(const QMap<int, QString> &dev); //注册标签时

private slots:
    void on_button_audio_capStart_clicked();
    void on_button_audio_capEnd_clicked();
    void on_button_audio_recordStart_clicked();

    void on_button_audio_recordEnd_clicked();

    void on_button_audio_sampleEnd_2_clicked();

    void on_button_audio_train_clicked();

    void on_button_local_start_clicked();

    void on_button_local_end_clicked();

    void on_btn_audio_param_clicked();

    void on_button_audio_sampleStart_2_clicked();

    void on_btn_audio_diagnose_clicked();

    void on_btn_ad_switch_clicked();

signals:
    void commandIssued(QString ip, QString command);
    void recordStart(const QString &ip, const QString &filename);
    void recordStop(const QString &ip);
	void paintStart(const QString &ip);
    void trainModel(QVector<QString> filenames, QVector<int> label, QVector<QString> devip);
    void widgetSwitched(int id);
	void startAudioBase();
	void stopAudioBase();
	void paintWavFile();
};

#endif // AUDIOWIDGET_H
