#include "greplace.h"

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
GReplace::GReplace(QObject* parent) : GFind(parent) {
	qDebug() << "";
}

GReplace::~GReplace() {
	qDebug() << "";
	close();
}

bool GReplace::doOpen() {
	qDebug() << "";

	GFind::findItems_.clear();
	for (GObj* obj: replaceItems_) {
		GReplaceItem* replaceItem = PReplaceItem(obj);

		if (replaceItem->replacePattern_ == "") {
			SET_ERR(GErr::ValueIsNull, "replacePattern can not be blank");
			return false;
		}
		GReplaceItem::Type replaceType_ = replaceItem->replaceType_;
		switch (replaceType_) {
			case GReplaceItem::String :
				break;
			case GReplaceItem::HexValue : {
				QByteArray ba = QByteArray::fromHex(replaceItem->replacePattern_.toUtf8());
				replaceItem->replaceHexPattern_ = QString::fromLatin1(ba.data(), ba.size());
				break;
			}
		}

		GFindItem* findItem = new GFindItem;
		findItem->offset_ = replaceItem->offset_;
		findItem->endOffset_ = replaceItem->endOffset_;
		findItem->count_ = replaceItem->count_;
		findItem->pattern_ = replaceItem->pattern_;
		findItem->type_ = replaceItem->type_;
		findItem->category_ = replaceItem->category_;
		GFind::findItems_.push_back(*findItem);
	}
	bool res = GFind::doOpen();
	return res;
}

bool GReplace::doClose() {
	return GFind::doClose();
}

void GReplace::processFound(int itemIndex, int foundIndex, QString& captured, QString& text) {
	Q_ASSERT(itemIndex < replaceItems_.count());
	GObj* obj = replaceItems_.at(itemIndex);
	GReplaceItem* replaceItem = reinterpret_cast<GReplaceItem*>(obj);
	Q_ASSERT(replaceItem != nullptr);
	QString replaceStr;
	GReplaceItem::Type replaceType = replaceItem->replaceType_;
	switch (replaceType) {
		case GReplaceItem::String :
			replaceStr = replaceItem->replacePattern_;
			break;
		case GReplaceItem::HexValue : {
			replaceStr = replaceItem->replaceHexPattern_;
			break;
		}
	}
	if (captured.size() != replaceStr.size()) {
		qWarning() << QString("size is different '%1'(%2) '%3'(%4)").arg(captured).arg(captured.size()).arg(replaceStr).arg(replaceStr.size());
	}
	text = text.left(foundIndex) + replaceStr + text.mid(foundIndex + replaceStr.size());

	if (!log_) return;
	QString logCaptured = captured;
	bool isPrintable = true;
	for (QChar ch: logCaptured) {
		if (!ch.isPrint()) {
			isPrintable = false;
			break;
		}
	}
	if (!isPrintable) {
		QByteArray ba = logCaptured.toUtf8();
		logCaptured = "0x" + ba.toHex();
	}
	qInfo() << QString("replaced('%1'>'%2 '%3 0x%4)").arg(logCaptured).arg(replaceStr).arg(foundIndex).arg(QString::number(foundIndex, 16));
}

void GReplace::replace(GPacket* packet) {
	GFind::find(packet);
}
