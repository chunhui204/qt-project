#include "mainwidget.h"
#include <QApplication>
#include <QTimer>
#include "gaussmodel/tensor.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	
	//char *p = new char();
    MainWidget w;
    w.show();
	return a.exec();
}
