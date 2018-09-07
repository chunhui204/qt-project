#ifndef VIDEOBASE_H
#define VIDEOBASE_H

#include <QObject>
#include <QThread>
#include <QCamera>
#include <QDataStream>
#include <QCameraInfo>
#include <QAbstractVideoSurface>
#include <QVideoFrame>
#include <QImage>

class VideoBase : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit VideoBase( QObject *parent = 0);
    ~VideoBase();
    QByteArray videoInfoToTcp();
    void startVideo();
    void stopVideo();

protected:

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);


signals:
    void framePresented( QImage frame);

private:
    QCamera *m_cam;
};

#endif // VIDEOBASE_H
