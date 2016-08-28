/**************************************************************************

Author		: Dongming Sheng

Date		: 2016-08-20

File Name	: main.cpp

**************************************************************************/

#include <QApplication>
#include"mainwindow.h"
#include<qsplashscreen.h>
#include<qelapsedtimer.h>

int main(int argc, char *argv[]){
	QApplication app(argc, argv);

	QSplashScreen *splash = new QSplashScreen;
	splash->setPixmap(QPixmap(":/splash/images/splash.png"));
	splash->show();
	
	Qt::Alignment bottomCenter = Qt::AlignCenter | Qt::AlignBottom;
	splash->showMessage(QObject::tr("Setting up the main window..."),
		bottomCenter, Qt::white);
	MainWindow mainWin;
	/*QElapsedTimer et;
	et.start();
	while (et.elapsed() < 500) {}*/
	mainWin.show();
	splash->finish(&mainWin);
	delete splash;

	return app.exec();
}
