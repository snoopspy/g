#include <GApp>
#include "probewidget.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	ProbeWidget pw;
	pw.show();
	return a.exec();
}
