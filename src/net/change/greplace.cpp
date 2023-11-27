#include "greplace.h"

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
GReplace::GReplace(QObject* parent) : GFind(parent) {
}

GReplace::~GReplace() {
	close();
}

bool GReplace::doOpen() {
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
		GFind::findItems_.push_back(*findItem);
	}
	bool res = GFind::doOpen();
	return res;
}

bool GReplace::doClose() {
	return GFind::doClose();
}

void GReplace::processFound(int itemIndex, int foundIndex, QString& foundStr) {
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
	if (foundStr.size() != replaceStr.size()) {
		qWarning() << QString("size is different '%1'(%2) '%3'(%4)").arg(foundStr).arg(foundStr.size()).arg(replaceStr).arg(replaceStr.size());
	}
	heystack_ = heystack_.left(foundIndex) + replaceStr + heystack_.mid(foundIndex + replaceStr.size());
	replaced_ = true;

	if (!log_) return;
	QString logFoundStr = printableStr(foundStr);
	QString logReplaceStr  = printableStr(replaceStr);
	qInfo() << QString("replaced('%1'>'%2' %3 0x%4)").arg(logFoundStr).arg(logReplaceStr).arg(foundIndex).arg(QString::number(foundIndex, 16));
}

void GReplace::replace(GPacket* packet) {
	replaced_ = false;
	GFind::find(packet);
	if (replaced_) {
		QByteArray ba = heystack_.toLatin1();
		// qDebug() << ba.size(); // gilgil temp 2023.11.28
		memcpy(packet->buf_.data_, ba.data(), ba.size());
		packet->ctrl_.changed_ = true;
	}
	if (replaced_)
		emit replaced(packet);
	else
		emit notReplaced(packet);
}
