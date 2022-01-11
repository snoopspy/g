#include <GApp>
#include <GJson>
#include "probewidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	ProbeWidget pw;

	QJsonObject jo = GJson::loadFromFile();
	jo["pw"] >> pw;

	pw.show();

	int res = a.exec();

	jo["pw"] << pw;
	GJson::saveToFile(jo);

	return res;
}
