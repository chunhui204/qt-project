#include "mainwidget.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>

QByteArray tensor_to_bytes(const Tensor &obj);

bool IsBig_Endian()
{
    unsigned short test = 0x1234;
    if(*( (unsigned char*) &test ) == 0x12)
       return true;
   else
       return false;
}//IsBig_Endian()

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    w.show();

    return a.exec();
}
/**
 * @brief tensor_to_bytes,小端模式：低字节-低地址
 * @param arr
 * @param len
 * @return
 */

QByteArray tensor_to_bytes(const Tensor &obj)
{
    const int unitbyte = sizeof(double);
    const int size = obj.rows() * obj.cols() * unitbyte;
    Tensor::const_iterator ptr = obj.begin();
    QByteArray ret(size, 0);

    for(int i=0; i< obj.rows()*obj.cols(); i++)
    {
        for(int j=0; j< unitbyte; j++)
        {
            ret[i*unitbyte + j]=((char*)(ptr + i))[j];
        }
    }
    return ret;
}
