#ifndef LABELMNGDIALOG_H
#define LABELMNGDIALOG_H

#include <QDialog>
#include <QMap>
#include "common.h"
#include <QTimer>
namespace Ui {
class LabelMngDialog;
}

class LabelMngDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LabelMngDialog(QWidget *parent = 0);
    ~LabelMngDialog();
private:
    bool check_duplicated(const QString &s);
    void repaint_tableWidget(int before_size);
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

signals:
	void labelRegistered(const QMap<int, QString> &);
private:
    Ui::LabelMngDialog *ui;
	QMap<int, QString> labelinfo;
	QFile config;
	int label;
	QTimer *timer;
//    static
};

#endif // LABELMNGDIALOG_H
