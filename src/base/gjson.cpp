#include "gjson.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>

// ----------------------------------------------------------------------------
// GJson
// ----------------------------------------------------------------------------
QJsonObject GJson::loadFromFile(QString fileName) {
	if (fileName == "")
		fileName = defaultFileName();

	if (!QFile::exists(fileName)) {
		return QJsonObject();
	}
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly)) {
		qWarning() << QString("file open(%1) failed(%2)").arg(fileName, file.errorString());
		return QJsonObject();
	}
	QByteArray ba = file.readAll();
	QJsonDocument doc = QJsonDocument::fromJson(ba);
	QJsonObject res = doc.object();
	return res;
}

bool GJson::saveToFile(QJsonObject jo, QString fileName) {
	if (fileName == "")
		fileName = defaultFileName();

	QFile file(fileName);
	if (!file.open(QFile::WriteOnly)) {
		qWarning() << QString("file open(%1) failed(%2)").arg(fileName, file.errorString());
		return false;
	}
	QJsonDocument doc(jo);
	QByteArray ba = doc.toJson();
	qint64 writeLen = file.write(ba);
	if (writeLen != ba.size()) {
		qWarning() << QString("file write(%1) return %2(%3)").arg(fileName, QString::number(writeLen), file.errorString());
		return false;
	}
	return true;
}

#include <QCoreApplication>
QString GJson::defaultFileName() {
	QFileInfo fileInfo(QCoreApplication::applicationFilePath());
	QString res = fileInfo.fileName();
	if (res.endsWith(".exe")) res = res.left(res.length() - 4);
	if (res.endsWith(".so")) res = res.left(res.length() - 3);
	res += ".json";
	return res;
}

struct GJsonObject {
	GJsonObject() {
		fileName_ = GJson::defaultFileName();
		jo_ = GJson::loadFromFile(fileName_);
	}

	virtual ~GJsonObject() {
		GJson::saveToFile(jo_, fileName_);
	}

	QString fileName_;
	QJsonObject jo_;
};

QJsonObject& G_EXPORT GJson::instance() {
	static GJsonObject gjo;
	return gjo.jo_;
}

// ----------------------------------------------------------------------------
// QList<int>
// ----------------------------------------------------------------------------
void operator << (QJsonValueRef ref, const QList<int>& intList) {
	QStringList strList;
	for (int i: intList) {
		QString s = QString::number(i);
		strList.append(s);
	}
	ref = strList.join(',');
}

void operator >> (const QJsonValue val, QList<int>& intList) {
	if (val.isNull()) return;
	QStringList strList = val.toString().split(',');
	for (QString& s: strList) {
		intList.append(s.toInt());
	}
}

#include "prop/gprop.h"
// ----------------------------------------------------------------------------
// GProp
// ----------------------------------------------------------------------------
void operator << (QJsonValueRef ref, const GProp& prop) {
	QJsonObject jo;
	((GProp&)prop).propSave(jo);
	ref = jo;
}

void operator >> (const QJsonValue val, GProp& prop) {
	if (val.isNull()) return;
	QJsonObject jo = val.toObject();
	prop.propLoad(jo);
}

#ifdef QT_GUI_LIB

// ----------------------------------------------------------------------------
// GWidgetRect
// ----------------------------------------------------------------------------
GJson::GWidgetRect GJson::rect(QWidget* widget) {
	return GJson::GWidgetRect(widget);
}

#include <QPoint>
#include <QSize>
void operator << (QJsonValueRef ref, const GJson::GWidgetRect&& rect) {
	QJsonObject jo;

	QPoint pos = rect.widget_->pos();
	jo["left"] = pos.x();
	jo["top"] = pos.y();

	QSize size = rect.widget_->size();
	jo["width"] = size.width();
	jo["height"] = size.height();

	ref = jo;
}

void operator >> (const QJsonValue val, GJson::GWidgetRect&& rect) {
	if (val.isNull()) return;
	QJsonObject jo = val.toObject();

	QPoint pos;
	pos.setX(jo["left"].toInt());
	pos.setY(jo["top"].toInt());
	rect.widget_->move(pos);

	QSize size;
	size.setWidth(jo["width"].toInt());
	size.setHeight(jo["height"].toInt());
	rect.widget_->resize(size);
}

// ----------------------------------------------------------------------------
// GSplitterSizes
// ----------------------------------------------------------------------------
GJson::GSplitterSizes GJson::splitterSizes(QSplitter* splitter) {
	return GJson::GSplitterSizes(splitter);
}

void operator << (QJsonValueRef ref, const GJson::GSplitterSizes&& sizes) {
	ref << sizes.splitter_->sizes();
}

void operator >> (const QJsonValue val, GJson::GSplitterSizes&& sizes) {
	if (val.isNull()) return;
	QList<int> intSizes;
	val >> intSizes;
	sizes.splitter_->setSizes(intSizes);
}

// ----------------------------------------------------------------------------
// GTableWidgetColumnSizes
// ----------------------------------------------------------------------------
GJson::GTableWidgetColumnSizes GJson::columnSizes(QTableWidget* tableWidget) {
	return GJson::GTableWidgetColumnSizes(tableWidget);
}

void operator << (QJsonValueRef ref, const GJson::GTableWidgetColumnSizes&& columnSizes) {
	int count = columnSizes.tableWidget_->columnCount();
	QList<int> intSizes;
	for (int i = 0; i < count - 1; i++) {
		intSizes << columnSizes.tableWidget_->columnWidth(i);
	}
	ref << intSizes;
}

void operator >> (const QJsonValue val, GJson::GTableWidgetColumnSizes&& columnSizes) {
	if (val.isNull()) return;
	QList<int> intSizes;
	val >> intSizes;
	int count = columnSizes.tableWidget_->columnCount();
	for (int i = 0; i < count - 1; i++) {
		columnSizes.tableWidget_->setColumnWidth(i, intSizes.at(i));
	}
}

// ----------------------------------------------------------------------------
// GTreeViewColumnSizes
// ----------------------------------------------------------------------------
GJson::GTreeViewColumnSizes GJson::columnSizes(QTreeView* treeView) {
	return GJson::GTreeViewColumnSizes(treeView);
}

void operator << (QJsonValueRef ref, const GJson::GTreeViewColumnSizes&& columnSizes) {
	int count = columnSizes.treeView_->header()->count();
	QList<int> intSizes;
	for (int i = 0; i < count - 1; i++) {
		intSizes << columnSizes.treeView_->columnWidth(i);
	}
	ref << intSizes;
}

void operator >> (const QJsonValue val, GJson::GTreeViewColumnSizes&& columnSizes) {
	if (val.isNull()) return;
	QList<int> intSizes;
	val >> intSizes;
	int count = columnSizes.treeView_->header()->count();
	for (int i = 0; i < count - 1; i++) {
		columnSizes.treeView_->setColumnWidth(i, intSizes.at(i));
	}
}

#endif // QT_GUI_LIB
