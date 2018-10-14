#include "localaudiowarper.h"

LocalAudioWarper::LocalAudioWarper(const QString &ip, QObject *parent) :
    AudioWarper(ip, parent),
	audioBase(new AudioBase(this))
{
	void (LocalAudioWarper::*onReadyRead)(int) = &LocalAudioWarper::onReadyRead;
	connect(audioBase, &AudioBase::readyRead, this, onReadyRead);
	//AudioWarper::shared_fmtlist->push_back(format);
}
void LocalAudioWarper::onReadyRead()
{
}
void LocalAudioWarper::onReadyRead(int bytes)
{
    QByteArray array = audioIO->read(bytes);
    AudioWarper::writeCirclebuf(array.data(), array.size());
}
void LocalAudioWarper::onStartAudio()
{
	if (state == Status::started)
		return;
	state = Status::started;
	timer->start();
	auto ret = audioBase->startAudio();
	audioIO = std::get<0>(ret);
	format = std::get<1>(ret);
	emit AudioWarper::audioFormatInit(format);
}
void LocalAudioWarper::onStopAudio()
{
	if (state == Status::stopped)
		return;
	state = Status::stopped;
	if (timer->isActive())
		timer->stop();
	audioBase->stopAudio();
}
