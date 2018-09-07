#ifndef AUDIOSETTING_H
#define AUDIOSETTING_H

#include <QDialog>
#include "common.h"
#include <QVector>

namespace Ui {
class Setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QWidget *parent = 0);
    ~Setting();
    void designWidget(const QVector<AudioSettingFormat> &) ;

signals:
    void audioFormatChanged(AudioSettingFormat);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::Setting *ui;
    AudioSettingFormat currentFormat;
    QVector<AudioSettingFormat> m_formats;
};

#endif // AUDIOSETTING_H
