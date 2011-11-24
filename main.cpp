#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(raytracer);
	QApplication app(argc, argv);

	MainWindow win;
	win.show();

	return app.exec();
}