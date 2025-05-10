#include "gprop.h"
#include "base/gobj.h"

// ----------------------------------------------------------------------------
// GProp
// ----------------------------------------------------------------------------
void GProp::propLoad(QJsonObject jo) {
	QObject* object = dynamic_cast<QObject*>(this);
	if (object == nullptr) {
		qWarning() << "object is null";
		return;
	}
	const QMetaObject* mobj = object->metaObject();
	int count = mobj->propertyCount();
	for (int i = 0; i < count; i++) {
		QMetaProperty mpro = mobj->property(i);
		propLoad(jo, mpro);
	}
}

void GProp::propSave(QJsonObject& jo) {
	QObject* object = dynamic_cast<QObject*>(this);
	if (object == nullptr) {
		qWarning() << "object is null";
		return;
	}
	const QMetaObject* mobj = object->metaObject();
	int count = mobj->propertyCount();
	for (int i = 0; i < count; i++) {
		QMetaProperty mpro = mobj->property(i);
		propSave(jo, mpro);
	}
}

bool GProp::propLoad(QJsonObject jo, QMetaProperty mpro) {
	QObject* object = dynamic_cast<QObject*>(this);
	if (object == nullptr) {
		qWarning() << "object is null";
		return false;
	}
	const char* propName = mpro.name();
	int userType = mpro.userType();
	QJsonObject::iterator it = jo.find(propName);
	if (it == jo.end()) return false;

	QVariant variant = (static_cast<const QJsonObject>(jo))[propName];
	bool res = false;

	if (mpro.isEnumType()) {
		QMetaEnum menum = mpro.enumerator();
		QString key = variant.toString();
		if (key == "") return true;
		int value = menum.keyToValue(qPrintable(key));
		res = object->setProperty(propName, value);
	} else
		switch (userType) {
			case QMetaType::Bool:
				res = object->setProperty(propName, variant.toBool());
				break;

			case QMetaType::QChar:
				res = object->setProperty(propName, variant.toString());
				break;

			case QMetaType::Char:
			case QMetaType::Double:
			case QMetaType::Float:
			case QMetaType::Int:
			case QMetaType::Long:
			case QMetaType::LongLong:
			case QMetaType::Short:
			case QMetaType::UChar:
			case QMetaType::UInt:
			case QMetaType::ULong:
			case QMetaType::ULongLong:
			case QMetaType::UShort: {
				QString s = variant.toString();
				if (s == "") return true;
				res = object->setProperty(propName, QVariant::fromValue<QString>(s));
				break;
			}

			case QMetaType::QString: {
				QString s = variant.toString();
				res = object->setProperty(propName, QVariant::fromValue<QString>(s));
				break;
			}

			case QMetaType::QStringList: {
				QJsonArray array = jo[propName].toArray();
				QStringList sl;
				for (int i = 0; i < array.size(); i++)
					sl.append(array.at(i).toString());
				res = object->setProperty(propName, sl);
				break;
			}
		}

	if (userType == qMetaTypeId<GObjPtr>()) {
		GObj* parent = dynamic_cast<GObj*>(object->parent());
		if (parent != nullptr) {
			QString s = variant.toString();
			if (s != "") {
				QObject* qobj = parent->findChild<QObject*>(s);
				if (qobj != nullptr) {
					GObj* node = dynamic_cast<GObj*>(qobj);
					if (node != nullptr) {
						GObjPtr objPtr = node;
						res = object->setProperty(propName, QVariant::fromValue<GObjPtr>(objPtr));
					}
				}
			} else
				res = true;
		}
	}

	if (userType == qMetaTypeId<GObjRef>()) {
		GObj* obj = qvariant_cast<GObjRef>(object->property(propName));
		Q_ASSERT(obj != nullptr);
		obj->propLoad(jo[propName].toObject());
		return true;
	}

	if (userType == qMetaTypeId<GObjRefArrayPtr>()) {
		GObjRefArrayPtr array = qvariant_cast<GObjRefArrayPtr>(object->property(propName));
		Q_ASSERT(array != nullptr);
		array->clear();
		QJsonArray ja = jo[propName].toArray();
		for (QJsonValue value: ja) {
			QJsonObject childJo = value.toObject();
			GObj* childObj = array->addObj();
			childObj->propLoad(childJo);
		}
		return true;
	}

	if (!res) {
		qWarning() << QString("%1::propLoad(%2, %3) return false").arg(object->metaObject()->className(), propName, variant.toString());
	}
	return res;
}

bool GProp::propSave(QJsonObject& jo, QMetaProperty mpro) {
	QObject* object = dynamic_cast<QObject*>(this);
	if (object == nullptr) {
		qWarning() << "object is null";
		return false;
	}
	const char* propName = mpro.name();
	int userType = mpro.userType();
	QVariant variant = object->property(propName);

	if (mpro.isEnumType()) {
		QMetaEnum menum = mpro.enumerator();
		int value = variant.toInt();
		QString key = menum.valueToKey(value);
		jo[propName] = key;
		return true;
	}

	switch (userType) {
		case QMetaType::Bool:
			jo[propName] = variant.toBool();
			return true;

		case QMetaType::QChar:
			jo[propName] = object->property(propName).toString();
			return true;

		case QMetaType::Char:
		case QMetaType::Double:
		case QMetaType::Float:
		case QMetaType::Int:
		case QMetaType::Long:
		case QMetaType::LongLong:
		case QMetaType::Short:
		case QMetaType::UChar:
		case QMetaType::UInt:
		case QMetaType::ULong:
		case QMetaType::ULongLong:
		case QMetaType::UShort: {
			QString s = variant.toString();
			jo[propName] = s;
			return true;
		}

		case QMetaType::QString: {
			QString s = variant.toString();
			jo[propName] = s;
			return true;
		}

		case QMetaType::QStringList: {
			QStringList sl = variant.toStringList();
			QJsonArray array;
			for (int i = 0; i < sl.size(); i++)
				array.append(QJsonValue(sl.at(i)));
			jo[propName] = array;
			return true;
		}
	}

	if (userType == qMetaTypeId<GObjPtr>()) {
		GObj* obj = qvariant_cast<GObjPtr>(variant);
		QString s = obj != nullptr ? obj->objectName() : "";
		jo[propName] = s;
		return true;
	}

	if (userType == qMetaTypeId<GObjRef>()) {
		GObj* obj = qvariant_cast<GObjRef>(variant);
		Q_ASSERT(obj != nullptr);
		QJsonObject childJo;
		obj->propSave(childJo);
		jo[propName] = childJo;
		return true;
	}

	if (userType == qMetaTypeId<GObjRefArrayPtr>()) {
		GObjRefArrayPtr array = qvariant_cast<GObjRefArrayPtr>(variant);
		Q_ASSERT(array != nullptr);
		QJsonArray ja;
		for (_GObjRefArray::iterator it = array->begin(); it != array->end(); it++) {
			QJsonObject childJo;
			GObj* childObj = *it;
			childObj->propSave(childJo);
			ja.append(QJsonValue(childJo));
		}
		jo[propName] = ja;
		return true;
	}

	qWarning() << QString("%1::propSave(%2, %3) return false").arg(object->metaObject()->className(), propName, variant.toString());
	return false;
}

#ifdef QT_GUI_LIB
#include "gpropitem-bool.h"
#include "gpropitem-char.h"
#include "gpropitem-enum.h"
#include "gpropitem-objptr.h"
#include "gpropitem-objref.h"
#include "gpropitem-objrefarrayptr.h"
#include "gpropitem-stringlist.h"
#include "gpropitem-unknowntype.h"
#include "gpropitem-variant.h"

GPropItem* GProp::propCreateItem(GPropItemParam* param) {
	const char* propName = param->mpro_.name();
	int userType = param->mpro_.userType();

	if (param->mpro_.isEnumType()) {
		return new GPropItemEnum(param);
	}

	switch (userType) {
		case QMetaType::Bool:
			return new GPropItemBool(param);

		case QMetaType::QChar:
			return new GPropItemChar(param);

		case QMetaType::Char:
		case QMetaType::Double:
		case QMetaType::Float:
		case QMetaType::Int:
		case QMetaType::Long:
		case QMetaType::LongLong:
		case QMetaType::QString:
		case QMetaType::Short:
		case QMetaType::UChar:
		case QMetaType::UInt:
		case QMetaType::ULong:
		case QMetaType::ULongLong:
		case QMetaType::UShort:
			return new GPropItemVariant(param);
	}

	if (userType == QMetaType::QStringList)
		return new GPropItemStringList(param);

	if (userType == qMetaTypeId<GObjPtr>())
		return new GPropItemObjPtr(param);

	if (userType == qMetaTypeId<GObjRef>())
		return new GPropItemObjRef(param);

	if (userType == qMetaTypeId<GObjRefArrayPtr>())
		return new GPropItemObjRefArrayPtr(param);

	qWarning() << QString("can not create GPropItem(object=%1 propName=%2)").arg(param->object_->metaObject()->className(), propName);
	return nullptr;
}

void GProp::propCreateItems(QTreeWidget* treeWidget, QTreeWidgetItem* parent, QObject* object) {
	const QMetaObject* mobj = object->metaObject();
	int propCount = mobj->propertyCount();
	for (int i = 1; i < propCount; i++) { // skip objectName
		GPropItemParam param(treeWidget, parent, object, mobj->property(i));
		GPropItem* item = propCreateItem(&param);
		if (item == nullptr) {
			qWarning() << QString("item is nullptr typeName='%1' name='%2'").arg(param.mpro_.typeName(), param.mpro_.name());
			item = new GPropItemUnknownType(&param);
		}
		item->update();
	}
}
#else
void* GProp::propCreateItem(void* param) {
	QString msg = QString("GProp::propCreateItem invalid function call param = %1").arg(quintptr(param));
	qCritical() << "****************************************************************";
	qCritical() << msg;
	qCritical() << "****************************************************************";
	return nullptr;
}

void GProp::propCreateItems(void* treeWidget, void* parent, QObject* object) {
	QString className = object == nullptr ? "" : object->metaObject()->className();
	QString msg = QString("GProp::propCreateItems invalid function call treeWidget = %1 parent= %2 className = %3")
		.arg(quintptr(treeWidget)).arg(quintptr(parent)).arg(className);
	qCritical() << "****************************************************************";
	qCritical() << msg;
	qCritical() << "****************************************************************";
}
#endif // QT_GUI_LIB

#ifdef QT_GUI_LIB
#include <QDialog>
#include "base/gjson.h"
#include "base/prop/gpropdialog.h"
void GProp::showDialog(QObject* object, QString jsonKey) {
	GPropDialog dialog;

	QJsonObject& jo = GJson::instance();
	if (jsonKey != "") jo[jsonKey] >> dialog;

	dialog.widget_.setObject(object);
	dialog.setModal(true);
	dialog.exec();

	if (jsonKey != "") jo[jsonKey] << dialog;
}
#endif // QT_GUI_LIB

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

struct TestPerson : QObject, GProp {
	Q_OBJECT
	Q_PROPERTY(QString name MEMBER name_)
	Q_PROPERTY(int age MEMBER age_)

public:
	QString name_{""};
	int age_{0};
};

TEST(GProp, loadSaveTest) {
	TestPerson person;
	person.name_ = "gilgil";
	person.age_ = 20;

	QJsonObject jo;
	person.propSave(jo);

	TestPerson person2;
	person2.name_ = "marsaj";
	person2.age_ = 17;

	person2.propLoad(jo);

	EXPECT_EQ(person2.name_, "gilgil");
	EXPECT_EQ(person2.age_, 20);
}

#include "gprop.moc"

#endif // GTEST
