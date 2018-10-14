#include "audiowidget.h"
#include "ui_audiowidget.h"
#include "common.h"
#include <iostream>
#include <vector>
#include <string>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QTextStream>
#include <QProcess>
std::vector<std::string> split_stdString(const std::string &str, char ch);

AudioWidget::AudioWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioWidget),
    saveDir(QDir::currentPath()),
	num_trainedDev(0)
{
    ui->setupUi(this);
    initUI();

	ui->progressBar_audio->setValue(0);
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
    /***其他UI**/
    ui->combo_audio_2->setInsertPolicy(QComboBox::InsertAtCurrent);

    checkboxGrp = new QButtonGroup(this);
    checkboxGrp->setExclusive(false);
    void (QButtonGroup::*btnsignal)(QAbstractButton *button) = &QButtonGroup::buttonClicked;
    connect(checkboxGrp, btnsignal, [&](QAbstractButton *button)
    {
		diagnosedIps.append(registeredDevs.dev_to_ip(button->text()));
    });
    radiobtnGrp = new QButtonGroup(this);
    radiobtnGrp->setExclusive(true);
    connect(radiobtnGrp, btnsignal, [&](QAbstractButton *button)
    {
        qcout<<"显示设备"<<button->text();

        curveIp = registeredDevs.dev_to_ip(button->text());
		emit paintStart(curveIp);
    });

    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(true);
    ui->button_audio_sampleEnd_2->setDisabled(true);
}
AudioWidget::~AudioWidget()
{
    delete ui;
}
/**
 * 1.如果保证绘图线程处理速度尽量与接收线程同步，那么曲线和计算的label就是匹配的，（为了实时性应该是同步的）
 * 2.当接受完整个buffer长度才计算label会有些滞后性，如果一般数据就能预测那会实时性强
 */
void AudioWidget::onLabelPredicted(int label, bool savefile, QString id)
{
	QString labelname = "";
	if (registeredLabs.find(label) != registeredLabs.end())
		labelname = registeredLabs[label];
	if (curveIp == id)
	{
		if (label == -1)
			ui->label_audio_class->setText(QString::fromLocal8Bit("未知类别"));
		else if (label == -2)
			ui->label_audio_class->setText(QString::fromLocal8Bit("未进行预测"));
		else
			ui->label_audio_class->setText(QString::fromLocal8Bit("类别为：") + "\t"
				+ labelname  + "\t" + QString::number(label));
	}

	if (savefile)
	{
		qDir.cd(AUDIO_PATH);
		qDir.cd(AUDIO_DIAGNOSE_PATH);
		QDateTime date = QDateTime::currentDateTime();
		QFile f(qDir.absolutePath() + "/" + id + ".txt");
		f.open(QIODevice::Append | QIODevice::Text);
		QTextStream ts(&f);
		QString time;
		QString join[] = { QString::fromLocal8Bit("年"), QString::fromLocal8Bit("月"), QString::fromLocal8Bit("日"),
						 QString::fromLocal8Bit("时"), QString::fromLocal8Bit("分") };
		auto splits = date.toString("yyyy-MM-dd-hh-mm").split("-");
		for (int i = 0; i < splits.size(); ++i)
			time += splits[i] + join[i];

		ts << time << "\t" << QString::fromLocal8Bit("诊断： ") << labelname << "\n";
		f.close();
	}

}

void AudioWidget::onCurvePainted(QVector<double> xs, QVector<double> ys)
{
    if(xs.isEmpty())
        ui->customPlot->graph(0)->setData(xs, xs); // 清空界面
    else
	{
		for (int i = 0; i < xs.size(); i++)
			ui->customPlot->graph(0)->addData(xs[i], ys[i]);
        ui->customPlot->xAxis->setRange(xs[0], 5, Qt::AlignHCenter);
    }
    ui->customPlot->replot();
}
void AudioWidget::onWavFileOpened(const QString &filename)
{
	//各种关闭
	on_button_audio_sampleEnd_2_clicked();
	on_button_audio_recordEnd_clicked();
	on_button_audio_capEnd_clicked();

	std::unique_ptr<WavFile> file(new WavFile(this));
	if (false == file->open(filename, true))
		return;
	QByteArray array = file->readAll();
	quint32 length = file->dataLength();
	length -= length % 2;
	int unitByte = file->fileFormat().channel.toInt() * file->fileFormat().sampleSizes.toInt() / 8;
	int samplesize = file->fileFormat().sampleSizes.toInt();
	file->close();
	QVector<double> xs, ys;
	//清空界面
	onCurvePainted(xs, ys);

	xs.reserve(length / unitByte);
	ys.reserve(length / unitByte);

	double now = 0;
	for (int i = 0; i < length; i += unitByte)
	{//处理完PointNum个点正好RefreshTime，而RefreshTime时间后定时器再次中断继续处理下一个RefreshTime间隔的点。
		now = now + 1.0 / file->fileFormat().sampleRates.toInt();
		xs.push_back(now);
		if (samplesize == 8)
			ys.push_back(array[i] / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
		else if (samplesize == 16)
		{
			qint16 *ptr = reinterpret_cast<qint16 *>(array.data() + i);
			ys.push_back(*ptr / 32768.0);//因为16位的范围-32767~32768
		}
	}

	onCurvePainted(xs, ys);
}
/**
 * @brief 开启终端麦克风
 */
void AudioWidget::on_button_audio_capStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(false);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    ui->button_local_start->setDisabled(true);
    ui->button_local_end->setDisabled(true);

	for (auto &ip : connectedIps)
		emit commandIssued(ip, "startAudio");
}
/**
 * @brief 关闭终端麦克风
 */
void AudioWidget::on_button_audio_capEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(true);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    ui->button_local_start->setDisabled(false);
    ui->button_local_end->setDisabled(false);
    emit recordStop(curveIp);

	for (auto &ip : connectedIps)
		emit commandIssued(ip, "stopAudio");
}
/**
 * @brief 开启录音
 */
void AudioWidget::on_button_audio_recordStart_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(false);
    ui->button_audio_sampleStart_2->setDisabled(true);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    //拼接路径
    qDir.cd(saveDir);
//    cout << saveDir;
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_FILE_PATH) == false)
        qDir.mkdir(AUDIO_FILE_PATH);
    qDir.cd(AUDIO_FILE_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
	
    emit recordStart(curveIp, name);
}
/**
 * @brief 结束录音
 */
void AudioWidget::on_button_audio_recordEnd_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(false);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    emit recordStop(curveIp);
}
/**
 * @brief 开始采集样本,和录音不能同时进行
 */
void AudioWidget::on_button_audio_sampleStart_2_clicked()
{
    if(ui->combo_audio_2->currentText().isEmpty())
        return;
    //get label
	int label = -1;
	for (auto i = registeredLabs.begin(); i != registeredLabs.end(); ++i)
		if (i.value() == ui->combo_audio_2->currentText())
		{
			label = i.key();
			break;
		}
    ui->button_audio_sampleStart_2->setDisabled(true);
    ui->button_audio_sampleEnd_2->setDisabled(false);
    this->ui->button_audio_recordStart->setDisabled(true);
    this->ui->button_audio_recordEnd->setDisabled(true);
    //拼接路径
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == false)
        qDir.mkdir(AUDIO_PATH);
    qDir.cd(AUDIO_PATH);

    if(qDir.exists(AUDIO_TRAIN_PATH) == false)
        qDir.mkdir(AUDIO_TRAIN_PATH);
    qDir.cd(AUDIO_TRAIN_PATH);

    QDateTime date =QDateTime::currentDateTime();
    QString name =qDir.absolutePath() + "/" + QString::number(label) + "_" + curveIp
            + "_" + date.toString("yyyy-MM-dd-hh-mm") + ".wav";
    qcout << name;

    emit recordStop(curveIp);
    emit recordStart(curveIp, name);
}
/**
 * @brief 结束采集
 */
void AudioWidget::on_button_audio_sampleEnd_2_clicked()
{
    this->ui->button_audio_recordStart->setDisabled(false);
    this->ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(false);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    emit recordStop(curveIp);
}
/**
* 训练
*/
void AudioWidget::on_button_audio_train_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择训练文件"),
                            qDir.absolutePath(), tr("AUDIO files (*.wav)"));
    if(files.isEmpty())//验证有效性
        return;

//    ui->button_audio_train->setDisabled(true);
    qDir.cd(saveDir);
    if(qDir.exists(AUDIO_PATH) == true)
        qDir.cd(AUDIO_PATH);
    if(qDir.exists(AUDIO_TRAIN_PATH) == true)
        qDir.cd(AUDIO_TRAIN_PATH);

    QVector<QString> filenames;
	QVector<QString> ips;
    QVector<int> labels;
	QSet<QString> tmp;

    for(QStringList::iterator i=files.begin();i!=files.end();i++)
    {
        filenames.push_back(*i);

        std::vector<std::string> name = split_stdString((*i).toStdString(), '/');
        std::vector<std::string> label_t = split_stdString(name[name.size()-1], '_');
        labels.push_back( std::stoi(label_t[0]));
		ips.push_back(QString::fromStdString(label_t[1]));
		tmp.insert(QString::fromStdString(label_t[1]));
    }
	num_trainedDev = tmp.size();

    emit trainModel(filenames, labels, ips);
}
/**
* 训练完成
*/
void AudioWidget::onAlreadyTrained()
{
	static int cnt = 0;
    ui->button_audio_train->setDisabled(false);
	++cnt;
    ui->progressBar_audio->setValue(cnt * 100 / num_trainedDev);
	if (cnt == num_trainedDev)
	{
		cnt = 0;
		num_trainedDev = 0;
	}
	
}
/**
 * @brief split_stdString by charactor, util function
 * @param str
 * @param vec
 */
std::vector<std::string> split_stdString(const std::string &str, char ch)
{
    std::vector<std::string> vec;
    std::string::const_iterator start = str.begin(), end = str.begin();

    while(end != str.end())
    {
        while(end != str.end() && *end == ch )
            end++;
        start = end;

        while(end != str.end() && *end != ch)
            end++;

        if(end != start)
        {
            //valid substr
            vec.push_back(std::string(start, end));
        }
    }
    return vec;
}
/**
 * @brief 本地采集
 */

void AudioWidget::on_button_local_start_clicked()
{
    ui->button_audio_capStart->setDisabled(true);
    ui->button_audio_capEnd->setDisabled(true);
    ui->button_audio_recordStart->setDisabled(false);
    ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(false);
    ui->button_audio_sampleEnd_2->setDisabled(true);
	emit startAudioBase();
    curveIp = "localhost";
    emit paintStart(curveIp);
}

void AudioWidget::on_button_local_end_clicked()
{
    ui->button_audio_capStart->setDisabled(false);
    ui->button_audio_capEnd->setDisabled(false);
    ui->button_audio_recordStart->setDisabled(true);
    ui->button_audio_recordEnd->setDisabled(true);
    ui->button_audio_sampleStart_2->setDisabled(true);
    ui->button_audio_sampleEnd_2->setDisabled(true);
    emit recordStop("localhost");
	emit stopAudioBase();
}
/**
 * @brief 发送参数
 */
void AudioWidget::on_btn_audio_param_clicked()
{
	ui->progressBar_audio->setValue(0);
	for (auto &ip : connectedIps)
		emit commandIssued(ip, "paramester");
}

/*
*诊断，打开相应的诊断文件
*/
void AudioWidget::on_btn_audio_diagnose_clicked()
{
	QProcess * po = new QProcess(this);
	qDir.cd(AUDIO_PATH);
	if (qDir.exists(AUDIO_DIAGNOSE_PATH) == false)
		return;
	qDir.cd(AUDIO_DIAGNOSE_PATH);
	for (auto i = diagnosedIps.begin(); i != diagnosedIps.end(); ++i)
	{
		qcout << qDir.absolutePath() + "/" + *i + ".txt";
		QStringList sl(qDir.absolutePath() + "/" + *i + ".txt");
		po->start("notepad.exe", sl);
	}
}
void AudioWidget::onDeviceRegistered(const AudioDevice& names)
{
	registeredDevs = names;
    paintDgnDevs(names);
    paintShowDevs(names);
}
void AudioWidget::paintDgnDevs(const AudioDevice& devs)
{
    if(checkboxGrp->buttons().size() > 0)
    {
        for(QAbstractButton *bn : checkboxGrp->buttons())
        {
            checkboxGrp->removeButton(bn);
            delete bn;
        }
        delete ui->scrollwidget_ad_diag->layout();
    }
    QVBoxLayout *layout = new QVBoxLayout(ui->scrollwidget_ad_diag);
    layout->setSpacing(10);
    for(auto it= devs.begin();it!=devs.end();it++)
    {
        QCheckBox *pbox = new QCheckBox(ui->scrollwidget_ad_diag);
        pbox->setText(QString::fromLocal8Bit("设备")+QString::number(it.value()));
		pbox->setDisabled(true);
        checkboxGrp->addButton(pbox);
        layout->addWidget(pbox);
    }
    ui->scrollwidget_ad_diag->setLayout(layout);
}
void AudioWidget::paintShowDevs(const AudioDevice& devs)
{
    if(radiobtnGrp->buttons().size() > 0)
    {
        for(QAbstractButton *bn : radiobtnGrp->buttons())
        {
            checkboxGrp->removeButton(bn);
            delete bn;
        }
        delete ui->scrollWidget_ad_curve->layout();
    }
    QVBoxLayout *layout = new QVBoxLayout(ui->scrollWidget_ad_curve);
    layout->setSpacing(10);
	for (auto it = devs.begin(); it != devs.end(); it++)
    {
        QRadioButton *pradio = new QRadioButton(ui->scrollWidget_ad_curve);
        pradio->setText(QString::fromLocal8Bit("设备") + QString::number(it.value()));
		pradio->setDisabled(true);
        radiobtnGrp->addButton(pradio);
        layout->addWidget(pradio);
    }
    ui->scrollWidget_ad_curve->setLayout(layout);
}
void AudioWidget::onLabelRegistered(const QMap<int, QString> &dev)
{
    registeredLabs = dev;
	for (int i = 0; i < ui->combo_audio_2->count(); i++)
	{
		ui->combo_audio_2->removeItem(i);
	}
	for (auto it = dev.begin(); it != dev.end(); it++)
	{
        ui->combo_audio_2->addItem(it.value());
	}
}
void AudioWidget::on_btn_ad_switch_clicked()
{
    emit widgetSwitched(VIDEO_WIDGET_INDEX);
}

void AudioWidget::onSocketConnected(const QString &ipaddr)
{
	QString ip = AudioWarper::pureAddress(ipaddr);
	connectedIps.insert(ip);
	if (registeredDevs.find(ip) == registeredDevs.end())
	{
		QMessageBox::information(this, QString::fromLocal8Bit("错误"),
			QString::fromLocal8Bit("该设备未注册!请为该ip注册音频设备。"));
		return;
	}
	/*为诊断文件创建目录**/
	if (qDir.exists(AUDIO_PATH) == false)
		qDir.mkdir(AUDIO_PATH);
	qDir.cd(AUDIO_PATH);
	if (qDir.exists(AUDIO_DIAGNOSE_PATH) == false)
		qDir.mkdir(AUDIO_DIAGNOSE_PATH);
	qDir.cd(AUDIO_DIAGNOSE_PATH);

	int i = registeredDevs[ip];
	//connectedIps.insert(ip);
	for (QAbstractButton *bn : checkboxGrp->buttons())
		if (QString::fromLocal8Bit("设备") + QString::number(i) == bn->text())
		{
			bn->setDisabled(false);
			break;
		}
	for (QAbstractButton *bn : radiobtnGrp->buttons())
		if (QString::fromLocal8Bit("设备") + QString::number(i) == bn->text())
		{
			bn->setDisabled(false);
			break;
		}
}
