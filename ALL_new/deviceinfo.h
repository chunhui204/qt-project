#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include<QList>
#include "common.h"
#include <QMap>

class DeviceInfo
{
private:
    QMap<QString,int> iplist;

public:
    int curvedev;
    QList<AudioSettingFormat> audiofmts;

    DeviceInfo() : curvedev(0){}
    DeviceInfo(const DeviceInfo&)  =delete;
    DeviceInfo& operator= (const DeviceInfo& ) = delete;

    /**connected  devs*/
    int getConnecteddevCount() const
    {
        return audiofmts.size();
    }

    /*registered devs***/
    int getRegistdevCount() const
    {
        return iplist.size();
    }
    void addItem(const QString& ip, int dev)
    {
        iplist[ip] = dev;
    }
    void deleteItem(int dev)
    {
        auto it = iplist.begin();
        while(it != iplist.end())
        {
            if(it.value() == dev)
            {
                iplist.erase(it);
                break;
            }
            ++it;
        }

    }
    QList<QString> getDevNames()
    {
        QList<QString> ret;
        for(auto k= iplist.begin(); k!=iplist.end(); k++)
            ret.push_back(QString::fromLocal8Bit("设备") + QString::number(k.value()));
        return ret;
    }
    QString ip_to_devname(const QString &ip)
    {
        return QString::fromLocal8Bit("设备") + QString::number(iplist[ip]);
    }
};

#endif // DEVICEINFO_H
