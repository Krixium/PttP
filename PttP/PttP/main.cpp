#include "PttP.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	PttP w;
	w.show();
	return a.exec();
}
