#include "labelmngdialog.h"
#include "ui_labelmngdialog.h"
#include <QMessageBox>
#include <QDir>

LabelMngDialog::LabelMngDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LabelMngDialog),
	label(0),
	timer(new QTimer(this))
{
    ui->setupUi(this);
	ui->tableWidget->setRowCount(100);
	ui->tableWidget->setColumnCount(2);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableWidget->setWindowTitle(QString::fromLocal8Bit("音频标签注册"));
	//    ui->tableWidget->resize(400, 300);  //设置表格
	QStringList header;
	header << QString::fromLocal8Bit("标签索引") << QString::fromLocal8Bit("标签名称");   //表头
	ui->tableWidget->setHorizontalHeaderLabels(header);
	/*从配置文件中加载，格式AudioDevice对象，QMap<int,QString>对象*/
	QDir dir;
	dir.cd(AUDIO_PATH);
	if (!dir.exists(AUDIO_PARAM_PATH))
	{
		dir.mkdir(AUDIO_PARAM_PATH);
	}
	dir.cd(AUDIO_PARAM_PATH);
	config.setFileName(dir.absolutePath() + "/" + "configure.dat");
	if (config.exists() && config.open(QIODevice::ReadOnly))
	{
		AudioDevice ad;
		QDataStream ss(&config);
		ss >> ad >> labelinfo;
		repaint_tableWidget(0);
		config.close();
	}
	connect(timer, &QTimer::timeout, [this]() {

		if (labelinfo.size())  emit labelRegistered(labelinfo);
		timer->stop();
	});
	timer->start(50);
}

LabelMngDialog::~LabelMngDialog()
{
    delete ui;
}

void LabelMngDialog::on_buttonBox_accepted()
{
	if (labelinfo.size() > 0)
	{
		if (config.open(QIODevice::ReadWrite))
		{
			AudioDevice ad;
			QDataStream ss(&config);
			ss >> ad;
			config.seek(0);
			ss << ad << labelinfo;
			config.close();
		}
		emit labelRegistered(labelinfo);
	}
    close();
}

void LabelMngDialog::on_buttonBox_rejected()
{
    close();
}
/**
 * @brief 添加
 */
void LabelMngDialog::on_pushButton_clicked()
{
    QString text = ui->lineEdit->text().simplified();//delete duplicate spaces
//    ui->lineEdit->clear();

    if(false == text.isEmpty() && false == check_duplicated(text))
    {
		while (labelinfo.find(label) != labelinfo.end())
			++label;
		labelinfo[label] = text;
        ++label;
    }
    repaint_tableWidget(labelinfo.size() - 1);

}
/**
 * @brief 删除
 */
void LabelMngDialog::on_pushButton_2_clicked()
{
    QString text = ui->lineEdit->text().simplified();
	if (false == text.isEmpty())
	{
		for (auto it = labelinfo.begin(); it != labelinfo.end(); ++it)
			if (it.value() == text)
			{
				label = qMin(label, it.key());
				labelinfo.erase(it);
				repaint_tableWidget(labelinfo.size() + 1);
				break;
			}
	}else
        QMessageBox::information(this, QString::fromLocal8Bit("删除失败"),
            QString::fromLocal8Bit("标签不存在!"));
}

bool LabelMngDialog::check_duplicated(const QString &s)
{
	for (auto it = labelinfo.begin(); it != labelinfo.end(); ++it)
		if (it.value() == s)
			return true;
	return false;
}
void LabelMngDialog::repaint_tableWidget(int before_size)
{
    for (int i = 0; i < before_size; i++)
        ui->tableWidget->removeRow(i);
    int i = 0;
    for (auto it = labelinfo.begin(); it != labelinfo.end(); it++, i++)
    {
        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(it.key()));
        QTableWidgetItem *item2 = new QTableWidgetItem(it.value());
        item1->setFlags(Qt::NoItemFlags);
        item2->setFlags(Qt::NoItemFlags);
        ui->tableWidget->setItem(i, 0, item1);
        ui->tableWidget->setItem(i, 1, item2);
    }

}
