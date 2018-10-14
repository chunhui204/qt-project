#include "audiowarper.h"
#include <QHostAddress>

AudioWarper::AudioWarper(const QString &id, QObject *parent) :
    QObject(parent),
	buf(new char[AudioBufSize]),
	cap(AudioBufSize),
	curvepos(0),
	sptrmpos(0),
	writepos(0),
	recordpos(0),
	curveAvaliable(0),
	sptrmAvaliable(0),
	length_for_spectrum(0),
	recordFlag(false),
	paintFlag(false),
	writeoverF(false),
	state(Status::stopped),
	dev_ip(pureAddress(id)),
	m_spectrum(new SpectrumHelper(dev_ip, this)),
	timer(new QTimer(this)),
	file(nullptr),
	accum_time(0)
{
	connect(timer, &QTimer::timeout, this, &AudioWarper::timeout);
}
AudioWarper::~AudioWarper()
{
	if (timer->isActive())
		timer->stop();
	xs.clear();
	ys.clear();
	QVector<double>().swap(xs);
	QVector<double>().swap(ys);
}

void AudioWarper::timeout()
{
	if (format.ip.isEmpty())
		return;

	int length = format.channel.toInt() * format.sampleRates.toInt()
		*format.sampleSizes.toInt() / 8 * (50) / 1000;
	length -= length % 2;
	int unitByte = format.channel.toInt() * format.sampleSizes.toInt() / 8;
	int frame_len = unitByte * SpectrumTimeForCalc;//一帧的字节长度
	int samplesize = format.sampleSizes.toInt();
    static double now = 0;	//注意static是类对象共享的

	if (recordFlag && writeoverF)//record
	{
		file->writeWave(format, buf.get() + recordpos, cap - recordpos);
		file->writeWave(format, buf.get(), writepos);
		recordpos = writepos;
		writeoverF = false;
	}
	
	if (paintFlag)//curve
	{
		curveAvaliable -= length;
		xs.clear();
		ys.clear();
		if (curveAvaliable > 0)
		{
			int num = length / unitByte;
			for (int i = 0; i < num; i++)
			{
				now = now + 1.0 / (format.sampleRates.toInt());
				xs.push_back(now);

				if (samplesize == 8)
				{
					ys.push_back(*(buf.get() + curvepos) / 128.0); //因为8位的范围-127~128，使曲线纵轴在[-1,1]
				}
				else if (samplesize == 16)
				{
					qint16 *ptr = reinterpret_cast<qint16 *>(buf.get() + curvepos);
					double t = *ptr;
					ys.push_back(*ptr / 32768.0);
				}
				curvepos = (unitByte + curvepos) % cap;
			}

			if (num) emit  curvePainted(xs, ys);
		}else
			curveAvaliable += length;

	}
	/*spectrum*/

	++accum_time;
	if (m_spectrum->isValid())
	{
		length_for_spectrum += length;      //积累的数据长度
		if (length_for_spectrum >= frame_len)//要处理的时间长度
		{
			length_for_spectrum = length_for_spectrum - frame_len;
			sptrmAvaliable -= frame_len;
			if (sptrmAvaliable <= 0)
			{
				sptrmAvaliable += frame_len;
				return;
			}
			QByteArray data(buf.get() + sptrmpos, qMin(SpectrumLengthSamples * unitByte, cap - sptrmpos));

			if (SpectrumLengthSamples*unitByte > cap - sptrmpos)
			{
				QByteArray tm(buf.get(), SpectrumLengthSamples*unitByte - (cap - sptrmpos));
				data.append(tm);
			}
			int label = m_spectrum->predict_of_frame(data, samplesize, unitByte);


			sptrmpos = (sptrmpos + SpectrumLengthSamples * unitByte) % cap;
			
			if (300 <= accum_time)//1min 1200
			{
				emit labelPredicted(label, true, dev_ip);
				accum_time = 0;
			}else
				emit labelPredicted(label, false, dev_ip);
		}
	}

}
void AudioWarper::onPaintStart(QString ip)
{
	if (ip != dev_ip)
	{
		paintFlag = false;
		xs.clear();
		ys.clear();
		QVector<double>().swap(xs);
		QVector<double>().swap(ys);
	}else
	{
		paintFlag = true;
		int length = format.channel.toInt() * format.sampleRates.toInt()
			*format.sampleSizes.toInt() / 8 * (30) / 1000;
		length -= length % 2;
		int unitByte = format.channel.toInt() * format.sampleSizes.toInt() / 8;
		int num = length / unitByte;

		xs.reserve(num);
		ys.reserve(num);
		//clear curve
		QVector<double> t;
		emit curvePainted(t, t);
	}
}

void AudioWarper::onRecordStart(QString ip, QString name)
{
    if(ip != format.ip)
        return;
	recordFlag = true;
	recordpos = writepos;
	file = new WavFile(this);
	file->open(name, false);
}
void AudioWarper::onRecordStop(QString ip)
{
    if(ip != format.ip)
        return;
	if (file != nullptr)
	{
		file->close();
		delete file;
		file = nullptr;
	}
	recordFlag = false;
}
void AudioWarper::onTrainModel(QVector<QString> filenames, QVector<int> label, QVector<QString> devip)
{
	for (int i = 0; i < devip.size(); ++i)
		if (devip[i] != dev_ip)
		{
			filenames.remove(i);
			label.remove(i);
			devip.remove(i);
			--i;
		}
	if (filenames.size() == 0)
		return;
	m_spectrum->train_from_file(filenames, label);
	emit alreadyTrained();
}
