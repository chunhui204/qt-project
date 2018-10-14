#include "audiowidget.h"
#include "ui_audiowidget.h"


AudioWidget::AudioWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioWidget)
{
    ui->setupUi(this);
    initUI();

}
void AudioWidget::initUI()
{
    // 初始化绘图组件customPlot
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
//    ui->customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
    //y周范围
    ui->customPlot->yAxis->setRange(-1,1);//(-32768, 32768);
    ui->customPlot->xAxis->setRange(0, 5, Qt::AlignLeft);
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->axisRect()->setupFullAxesBox();
    // 上方和右边坐标显示
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTickLabels(false);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->yAxis2->setTickLabels(false);
    // make left and bottom axes always transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));
    // let the ranges scale themselves so graph 0 fits perfectly in the visible area:
//    ui->customPlot->graph(0)->rescaleAxes();
    // Allow user to drag axis ranges with mouse, zoom with mouse wheel and select graphs by clicking:
    ui->customPlot->axisRect(0)->setRangeDrag(Qt::Horizontal);
    ui->customPlot->axisRect(0)->setRangeZoom(Qt::Horizontal);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}
AudioWidget::~AudioWidget()
{
    delete ui;
}

void AudioWidget::onLabelPredicted(int label)
{
    if(label == -1)
        ui->label->setText(QString::fromLocal8Bit("未知类别"));
    else if(label == -2)
        ui->label->setText(QString::fromLocal8Bit("未进行预测"));
    else
        ui->label->setText(QString::fromLocal8Bit("类别")+QString::number(label, 10));
}

void AudioWidget::onCurvePainted(QVector<double> xs, QVector<double> ys)
{
    if(xs.isEmpty())
        ui->customPlot->graph(0)->setData(xs, xs); // 清空界面
    else
    {
        for(int i=0; i< xs.size() ; i++)
            ui->customPlot->graph(0)->addData(xs[i], ys[i]);
        ui->customPlot->xAxis->setRange(xs[0], 5, Qt::AlignHCenter);
    }
    ui->customPlot->replot();
}
