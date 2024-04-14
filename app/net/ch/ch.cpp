#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "chwidget.h"

#ifdef Q_OS_ANDROID
void installCerts() {
	QDir dir;
	QString dirPath = "cert/";
	if (!dir.mkpath(dirPath))
		qWarning() << QString("can not create directory(%1)").arg(dirPath);
	dirPath = "cert/root/";
	if (!dir.mkpath(dirPath))
		qWarning() << QString("can not create directory(%1)").arg(dirPath);

	GApp::copyFileFromAssets("ffce", "", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	GApp::copyFileFromAssets("root.crt", "cert/root/", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	GApp::copyFileFromAssets("root.key", "cert/root/", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	GApp::copyFileFromAssets("default.crt", "cert/", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	GApp::copyFileFromAssets("default.key", "cert/", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
}
#endif // Q_OS_ANDROID

int main(int argc, char *argv[])
{
	GApp a(argc, argv, true, true);
#ifdef Q_OS_ANDROID
	installCerts();
#endif // Q_OS_ANDROID
	ChWidget cw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("pw") == jo.end();
	jo["cw"] >> cw;

	cw.show();
	if (isFirst) {
		int width = cw.width();
		cw.treeWidget_->setColumnWidth(ChWidget::ColumnHost, width / 2);
	}

	GCommand* command = &cw.cookieHijack_.command_;
	if (!command->open()) {
		QMessageBox::warning(&cw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["cw"] << cw;

	return res;
}
