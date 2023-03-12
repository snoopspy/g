#include "glogfile.h"
#include <QDate>

// ----------------------------------------------------------------------------
// GLogFile
// ----------------------------------------------------------------------------
GLogFile::GLogFile(QObject* parent) : GLog(parent) {
}

GLogFile::~GLogFile() {
	thread_.quit();
	thread_.wait();
	file_.close();
}

void GLogFile::write(QString& msg) {
	if (!configured_) {
		configure();
		configured_ = true;
	}
	QMutexLocker ml(&m_);
	file_.write(qPrintable(msg));
	file_.flush();
}

void GLogFile::configure() {
	if (!folder_.endsWith(QDir::separator()))
		folder_ += QDir::separator();
	QDir dir;
	if (!dir.mkpath(folder_)) {
		fprintf(stderr, "can not create directory(%s)\n", qPrintable(folder_));
	}
	changeFileName();
	thread_.start();
}

void GLogFile::changeFileName(){
	QDate now = QDate::currentDate();
	QString newFileName = now.toString("yyMMdd") + ".log";
	if (newFileName != fileName_) {
		QMutexLocker ml(&m_);
		fileName_ = newFileName;
		QString filePath = folder_ + fileName_;
		file_.close();
		file_.setFileName(filePath);
		if (!file_.open(QIODevice::Append | QIODevice::Text)) {
			fprintf(stderr, "error in file open(%s) error=%s\n", qPrintable(filePath), qPrintable(file_.errorString()));
		}
	}
}

void GLogFile::ChangeFileNameThread::quit(){
	swe_.wakeAll();
	QThread::quit();
}

void GLogFile::ChangeFileNameThread::run() {
	while (true) {
		if (swe_.wait(1000)) break; // signaled
		logFile_->changeFileName();
	}
}
