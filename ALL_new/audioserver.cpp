#include "audioserver.h"

AudioServer::AudioServer(QObject *parent) : QTcpServer(parent)
{

}
void AudioServer::onCommandIssued(const QString &ip, const QByteArray &command)
{
    (ipsockets[ip])->write(command);
}
