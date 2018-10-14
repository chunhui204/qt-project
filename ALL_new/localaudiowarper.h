#ifndef LOCALAUDIOWARPER_H
#define LOCALAUDIOWARPER_H

#include <QObject>
#include "audiowarper.h"
#include <QIODevice>
#include "audiobase.h"

class LocalAudioWarper : public AudioWarper
{
    Q_OBJECT
private:
	QIODevice* audioIO;
	AudioBase *audioBase;
public:
    explicit LocalAudioWarper(const QString &ip, QObject* parent=nullptr);

	virtual void onReadyRead();
    virtual void onReadyRead(int bytes);

public slots:
	void onStartAudio();
	void onStopAudio();
};

#endif // LOCALAUDIOWARPER_H
