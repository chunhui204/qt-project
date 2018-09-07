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

void Setting::designWidget(const QVector<AudioSettingFormat> &formats)
{
    if(formats.size() == 0)
        return;

    m_formats.clear();
    ui->comboBox_device_audio->clear();

    //加载麦克风设备信息
    for(int i=0; i < formats.size(); i++)
    {
        m_formats.push_back(formats[i]);
        ui->comboBox_device_audio->addItem(formats[i].deviceName);
        if(i == 0)
        {
            ui->label_sampleRate_audio->setText(formats[i].sampleRates);
            ui->label_sampleSize_audio->setText(formats[i].sampleSizes);
            ui->label_chns_audio->setText(formats[i].channel);
            ui->label_codec_audio->setText(formats[i].codec);
        }
    }
    //当设备名发生改变时，采样率等信息随之变化
    connect(ui->comboBox_device_audio, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            [=](int index)
    {
        ui->label_sampleRate_audio->setText(m_formats[index].sampleRates);
        ui->label_chns_audio->setText(m_formats[index].channel);
        ui->label_sampleSize_audio->setText(m_formats[index].sampleSizes);
        ui->label_codec_audio->setText(m_formats[index].codec);
    });
}

void Setting::on_buttonBox_accepted()
{
    qcout <<"ok" ;
    //如果改变了配置，重置音频设备，并停止终端采样工作。
    //如果在录音过程或采集训练样本过程中打开dialog，破坏数据，待处理。
    if(ui->comboBox_device_audio->currentIndex()>0)
    {
        int ind = ui->comboBox_device_audio->currentIndex();

        qcout <<"ok" << ind << " " << m_formats.size();

        currentFormat.deviceName = m_formats[ind].deviceName;
        currentFormat.channel = m_formats[ind].channel;
        currentFormat.sampleRates= m_formats[ind].sampleRates;
        currentFormat.sampleSizes = m_formats[ind].sampleSizes;
        currentFormat.codec = m_formats[ind].codec;

        emit audioFormatChanged(currentFormat);
    }
    close();
}

void Setting::on_buttonBox_rejected()
{
    close();
}
