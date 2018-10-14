#include "setting.h"
#include "ui_setting.h"

Setting::Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromLocal8Bit("配置"));
}

Setting::~Setting()
{
    delete ui;
}

void Setting::onAudioFormatInit(AudioFormat format)
{
	audiofmtList.push_back(format);
}
void Setting::showEvent(QShowEvent *event)
{
	ui->comboBox_device_audio->clear();

	//加载麦克风设备信息
	for (auto it = audiofmtList.begin(); it!= audiofmtList.end(); ++it)
	{
		ui->comboBox_device_audio->addItem((*it).deviceName);
		if (it == audiofmtList.begin())
		{
			ui->label_sampleRate_audio->setText((*it).sampleRates);
			ui->label_sampleSize_audio->setText((*it).sampleSizes);
			ui->label_chns_audio->setText((*it).channel);
			ui->label_codec_audio->setText((*it).codec);
		}
	}
	//当设备名发生改变时，采样率等信息随之变化
	connect(ui->comboBox_device_audio, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
		[&](int index)
	{
		ui->label_sampleRate_audio->setText(audiofmtList[index].sampleRates);
		ui->label_chns_audio->setText(audiofmtList[index].channel);
		ui->label_sampleSize_audio->setText(audiofmtList[index].sampleSizes);
		ui->label_codec_audio->setText(audiofmtList[index].codec);
	});
}
void Setting::on_buttonBox_accepted()
{
    //if(ui->comboBox_device_audio->currentIndex()>0)
    //{
    //    int ind = ui->comboBox_device_audio->currentIndex();
    //}
    close();
}

void Setting::on_buttonBox_rejected()
{
    close();
}
