#include "tcpaudiowarper.h"
#include <QHostAddress>
TcpAudioWarper::TcpAudioWarper(const QString &ip, std::unique_ptr<QTcpSocket> so, QObject *parent) :
   AudioWarper(ip, parent),
	socket(std::move(so)),
	readBytes(0),
	totalSize(0)
{
	connect(socket.get(), &QTcpSocket::readyRead, this, &TcpAudioWarper::onReadyRead);
}
void TcpAudioWarper::onReadyRead()
{
	if (readBytes == 0)
	{//读包的开始部分
		QDataStream socketStream(socket.get());
		QString headtype;
		socketStream >> headtype;
		if (headtype == "format")
		{
			socketStream >> AudioWarper::format.deviceName >> AudioWarper::format.sampleRates >> AudioWarper::format.channel
				>> AudioWarper::format.sampleSizes >> AudioWarper::format.codec >> AudioWarper::format.ip;
			emit AudioWarper::audioFormatInit(format);
		}
		else if (headtype == "data")
		{
			socketStream >> totalSize;
			totalSize += 4;
			//socket.reset(static_cast<QTcpSocket*>(socketStream.device()));
			QByteArray array = socket->read(totalSize);
			readBytes += array.size();

			AudioWarper::writeCirclebuf(array.data() + 4, array.size() - 4);
		}
	}
	else if (readBytes < totalSize)
	{
		QByteArray array = socket->read(totalSize - readBytes);
		readBytes += array.size();
		AudioWarper::writeCirclebuf(array.data(), array.size());
	}
	if (readBytes == totalSize)
		readBytes = 0;
}
void TcpAudioWarper::onCommandIssued(QString ip, QString head)
{
	QByteArray command;
	QDataStream ss(&command, QIODevice::WriteOnly);
	ss << head;
	if (head == "startAudio" && state == Status::stopped
		&& ip == AudioWarper::pureAddress(socket->peerAddress().toString()))
	{
		timer->start(30);
		state = Status::started;
		socket->write(command);
	}
	else if (head == "stopAudio" && state == Status::started && 
		ip == AudioWarper::pureAddress(socket->peerAddress().toString()))
	{
		timer->stop();
		state = Status::stopped;
		socket->write(command);
	}
	else if (head == "paramester" && ip == AudioWarper::pureAddress(socket->peerAddress().toString()))
	{
		auto size = m_spectrum->paramSize();
		ss << qint64(std::get<0>(size)) << qint64(std::get<1>(size)) << m_spectrum->params();
		socket->write(command);
	}
}