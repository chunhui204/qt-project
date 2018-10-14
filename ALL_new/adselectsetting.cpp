#include "adselectsetting.h"
#include "ui_adselectsetting.h"
#include <QTableWidget>
#include <QMessageBox>
#include <QDir>

int AdSelectSetting::currentIndex = 0;

AdSelectSetting::AdSelectSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdSelectSetting),
	timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->tableWidget->setRowCount(100);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget->setWindowTitle(QString::fromLocal8Bit("音频设备注册"));
//    ui->tableWidget->resize(400, 300);  //设置表格
    QStringList header;
    header<<QString::fromLocal8Bit("设备")<<QString::fromLocal8Bit("ip地址");   //表头
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->spinBox_dev_add->setRange(1, 100);
    ui->spinBox_dev_del->setRange(1, 100);
	
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
		QDataStream ss(&config);
		ss >> devinfo;
		repaint_tableWidget(0);
		config.close();
	}
	connect(timer, &QTimer::timeout, [this]() {

		if (devinfo.size())  emit deviceRegistered(devinfo);
		timer->stop();
	});
	timer->start(50);
}

AdSelectSetting::~AdSelectSetting()
{
    delete ui;
}

void AdSelectSetting::on_buttonBox_accepted()
{
	if (devinfo.size() > 0)
	{
		if (config.open(QIODevice::WriteOnly))
		{
			QDataStream ss(&config);
			ss << devinfo;
			config.close();
		}
		emit deviceRegistered(devinfo);
	}
    close();
}

void AdSelectSetting::on_buttonBox_rejected()
{
    close();
}

void AdSelectSetting::on_btn_add_clicked()
{
    QString ip = ui->ip_widget->getIpaddress();
    if(false == ip.isEmpty())
    {
        if(check_duplicated(ip))
        {
            QMessageBox::information(this, QString::fromLocal8Bit("添加失败"),
                                     QString::fromLocal8Bit("ip已经存在!"));
            return;
        }
        devinfo.addItem(ip, ui->spinBox_dev_add->value());
        ui->spinBox_dev_add->setValue(ui->spinBox_dev_add->value() + 1);
		repaint_tableWidget(devinfo.size() - 1);
    }
}
void AdSelectSetting::on_btn_delete_clicked()
{
    QString s = QString::fromLocal8Bit("设备") + QString::number(
                ui->spinBox_dev_del->value());
	auto  ret = devinfo.deleteItem(ui->spinBox_dev_del->value());

	if (ret == -1)
		QMessageBox::information(this, QString::fromLocal8Bit("删除失败"),
			QString::fromLocal8Bit("设备号不存在!"));
	else
		repaint_tableWidget(devinfo.size() + 1);
}

bool AdSelectSetting::check_duplicated(const QString &s)
{
	return devinfo.find(s) != devinfo.end();
}

void AdSelectSetting::repaint_tableWidget(int before_size)
{
	for (int i = 0; i < before_size; i++)
		ui->tableWidget->removeRow(i);
	int i = 0;
	for (auto it = devinfo.begin(); it != devinfo.end(); it++, i++)
	{
		QString dev = QString::fromLocal8Bit("设备") + QString::number(it.value());
		QTableWidgetItem *item1 = new QTableWidgetItem(dev);
		QTableWidgetItem *item2 = new QTableWidgetItem(it.key());
		item1->setFlags(Qt::NoItemFlags);
		item2->setFlags(Qt::NoItemFlags);
		ui->tableWidget->setItem(i, 0, item1);
		ui->tableWidget->setItem(i, 1, item2);
	}

}