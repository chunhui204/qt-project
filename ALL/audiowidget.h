#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>
#include <QDir>
#include "gaussmodel/tensor.h"
#include "audiobase.h"
#include <QThread>

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
private:
    Ui::AudioWidget *ui;
    AudioBase *audioBase;
    QThread *captureThread;
    //文件保存
    QString saveDir;        //ROOT目录路径
    QDir qDir;

public slots:
    void onDataProcessed( QVector<double> xs, QVector<double> ys);
    void onSavePathChanged(const QString &dir);
    void onLabelPredicted(int albel);
    void onAlreadyTrained(QByteArray ,  int, int);
    void onAudioFormatChanged(AudioSettingFormat format);
private slots:
    void on_button_audio_capStart_clicked();
    void on_button_audio_capEnd_clicked();
    void on_button_audio_recordStart_clicked();

    void on_button_audio_recordEnd_clicked();

    void on_button_audio_sampleStart_clicked();

    void on_button_audio_sampleEnd_clicked();

    void on_button_audio_labelAdd_clicked();

    void on_button_audio_train_clicked();

    void on_button_local_start_clicked();

    void on_button_local_end_clicked();

signals:
    void commandIssued(const QByteArray &command);
    void recordStart(const QString &filename);
    void recordStop();
    void trainModel( QVector<QString> filenames, QVector<int> label);
    void localAudioInfo(QByteArray response);
    void audioFormatChanged(AudioSettingFormat);
};

#endif // AUDIOWIDGET_H
