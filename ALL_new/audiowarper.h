#ifndef AUDIOWARPER_H
#define AUDIOWARPER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "common.h"
#include <QTcpSocket>
#include <memory>
#include <QVector>
#include <QTimer>
#include <QIODevice>
#include "wavfile.h"
#include "spectrumhelper.h"
/**
 * @brief The AudioWarper class
 * 一个设备对应一个warper对象，支持移动语义不支持拷贝语义
 * 定义数据接收和处理（预测标签）操作，可以放于同一线程之下，用于显示的数据处理位于单独线程
 *
 * 读入数据与处理数据都只在一个线程进行，不需要条件变量同步，即使使用互斥量也是串行进行。
 * 读写模型是只要涉及写操作同一时间只允写线程工作，如果没有写线程多个读线程可以实现并行操作，也就是说并行只有在多个读者时才有意义。
 */
enum class Status {
	started = 0,
	stopped = 1
};

class AudioWarper : public QObject
{
    Q_OBJECT
private:
	bool recordFlag;
	bool paintFlag;
	int curvepos;
	int sptrmpos;
	int recordpos;
	int writepos;
	int curveAvaliable;
	int sptrmAvaliable;

	bool writeoverF; //第一次写满数组
	int length_for_spectrum;
	int accum_time;

	std::unique_ptr<char> buf;
	WavFile *file;
	QString dev_ip;

protected:
	QTimer *timer;
    const int cap;
	QVector<double> xs, ys;
    AudioFormat format; //子类必须显式更新format
	Status state;
	SpectrumHelper *m_spectrum;

signals:
	void audioFormatInit(AudioFormat format);
	void curvePainted(QVector<double>, QVector<double>);
	void labelPredicted(int, bool, QString);
	void alreadyTrained();
	
public:
    explicit AudioWarper(const QString &id, QObject *parent = nullptr);
    ~AudioWarper();//没用virtual析构，因为所有子类要用共同的操作
	void onRecordStart(QString ip, QString name);
	void onRecordStop(QString ip);
	void onPaintStart(QString ip);
	void onTrainModel(QVector<QString> filenames, QVector<int> label, QVector<QString> devip);
	static QString pureAddress(const QString &s)
	{
		auto ret = s.split(":");
		if (ret.size() > 1)	return ret[3];
		else      return ret[0];
	}
	
protected:
	virtual void onReadyRead() = 0;
	void timeout();

	void writeCirclebuf(const char *src, int size)
	{
		if (size <= 0)
			return;
		int len = qMin(size, cap - writepos);
		memcpy(buf.get() + writepos, src, len);

		if (len < size)
			memcpy(buf.get(), src + len, size - len);
		if (writepos + size >= cap)
			writeoverF = true;
		writepos = (writepos + size) % cap;
		curveAvaliable += size;
		sptrmAvaliable += size;
	}

};

#endif // AUDIOWARPER_H
