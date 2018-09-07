#include "videobase.h"

VideoBase::VideoBase(QObject *parent) : QAbstractVideoSurface(parent)
{
    m_cam = new QCamera(QCameraInfo::defaultCamera());

    m_cam->setViewfinder(this);

//    connect(videoSurface, &VideoSurface::framePresented, &thread, &VideoThread::onFramePresented);
//    videoThread->start();
}
VideoBase::~VideoBase()
{
    delete m_cam;
}
QByteArray VideoBase::videoInfoToTcp()
{
    QByteArray arr;
    QDataStream stream(&arr, QIODevice::WriteOnly);

    stream << QString("videoDevice");
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        stream << cameraInfo.deviceName();
    }
    return arr;
}
void VideoBase::startVideo()
{
    m_cam->start();
}
void VideoBase::stopVideo()
{
    m_cam->stop();
}
QList<QVideoFrame::PixelFormat> VideoBase::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
//    Q_UNUSED(handleType);
        QList<QVideoFrame::PixelFormat> list;
        list << QVideoFrame::Format_RGB32;
        list << QVideoFrame::Format_ARGB32;
        list << QVideoFrame::Format_RGB24;
        list << QVideoFrame::Format_UYVY;
        list << QVideoFrame::Format_Y8;
        list << QVideoFrame::Format_YUYV;
        return list;
}

bool VideoBase::present(const QVideoFrame &frame)
{
    QVideoFrame frame_copy(frame);
    //已经映射到内存中了
    if(!frame_copy.isMapped())
        frame_copy.map(QAbstractVideoBuffer::ReadOnly);

        QImage image(
                    frame.bits(),
                    frame.width(),
                    frame.height(),
                    frame.bytesPerLine(),
                    QImage::Format_RGB32);
        frame_copy.unmap();
        emit framePresented(image);

        return true;
}
