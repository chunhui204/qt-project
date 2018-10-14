#ifndef _IPWIDGETS_H
#define _IPWIDGETS_H

#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QHBoxLayout>
#include <QSpinBox>

class IpWidgets : public QWidget
{

    Q_OBJECT
private:
    QSpinBox *m_SpinBox[4];
    QLabel *m_Label[3];
    QString m_Separator;        //四个数字之间的分隔符
protected:
    bool eventFilter(QObject *object, QEvent *event);
public:
    IpWidgets(QWidget *parent = 0);
    ~IpWidgets();
    QString getIpaddress();
};

#endif
