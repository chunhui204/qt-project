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

public slots:
    void onAudioFormatInit(AudioFormat format);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::Setting *ui;
	AudioFmtList audiofmtList;

	virtual void showEvent(QShowEvent *event) override;
};

#endif // AUDIOSETTING_H
