#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QWidget>

namespace Ui {
class AudioWidget;
}

class AudioWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioWidget(QWidget *parent = 0);
    ~AudioWidget();
public slots:
    void onCurvePainted(QVector<double> xs, QVector<double> ys);
    void onLabelPredicted(int);

private:
    Ui::AudioWidget *ui;
    void initUI();
};

#endif // AUDIOWIDGET_H
