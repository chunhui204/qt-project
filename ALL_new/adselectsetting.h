#ifndef ADSELECTSETTING_H
#define ADSELECTSETTING_H

#include <QDialog>
#include <QMap>
#include "common.h"
#include <memory>
#include <QTimer>
namespace Ui {
class AdSelectSetting;
}

class AdSelectSetting : public QDialog
{
    Q_OBJECT

public:
    explicit AdSelectSetting(QWidget *parent = 0);
    ~AdSelectSetting();

private:
    bool check_duplicated(const QString &);
	void repaint_tableWidget(int);
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_btn_add_clicked();

    void on_btn_delete_clicked();

signals:
    void deviceRegistered(const AudioDevice& devs);
private:
    Ui::AdSelectSetting *ui;
    static int currentIndex;
	AudioDevice devinfo;
	QFile config;
	QTimer *timer;
};

#endif // ADSELECTSETTING_H
