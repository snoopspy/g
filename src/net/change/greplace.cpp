#include "greplace.h"

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
bool GReplace::doOpen() {
	if (!enabled_) return true;

	if (!correctChecksum_.open()) {
		err = correctChecksum_.err;
		return false;
	}

	GFind::findItems_.clear();
	for (GObj* obj: replaceItems_) {
		GReplaceItem* replaceItem = PReplaceItem(obj);

		GReplaceItem::Type replaceType = replaceItem->replaceType_;
		if (replaceItem->replacePattern_ == "" && replaceType != GReplaceItem::None) {
			SET_ERR(GErr::ValueIsNull, "replacePattern can not be blank");
			return false;
		}

		switch (replaceType) {
			case GReplaceItem::String :
				break;
			case GReplaceItem::HexValue : {
				QByteArray ba = QByteArray::fromHex(replaceItem->replacePattern_.toUtf8());
				replaceItem->replaceHexPattern_ = QString::fromLatin1(ba.data(), ba.size());
				break;
			}
			case GReplaceItem::None :
				break;
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
	correctChecksum_.close();
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
		case GReplaceItem::HexValue :
			replaceStr = replaceItem->replaceHexPattern_;
			break;
		case GReplaceItem::None :
			if (log_) {
				QString logFoundStr = printableStr(foundStr);
				qInfo() << QString("found('%1' %2 0x%3)").arg(logFoundStr).arg(foundIndex).arg(QString::number(foundIndex, 16));
				return;
			}
	}
	if (foundStr.size() != replaceStr.size()) {
		qWarning() << QString("size is different '%1'(%2) '%3'(%4)").arg(foundStr).arg(foundStr.size()).arg(replaceStr).arg(replaceStr.size());
	}
	heystack_ = heystack_.left(foundIndex) + replaceStr + heystack_.mid(foundIndex + replaceStr.size());
	replaced_ = true;

	if (log_) {
		QString logFoundStr = printableStr(foundStr);
		QString logReplaceStr  = printableStr(replaceStr);
		qInfo() << QString("replaced('%1'>'%2' %3 0x%4)").arg(logFoundStr).arg(logReplaceStr).arg(foundIndex).arg(QString::number(foundIndex, 16));
	}
}

void GReplace::replace(GPacket* packet) {
	if (!enabled_) return;

	replaced_ = false;
	GFind::find(packet);
	if (replaced_) {
		QByteArray ba = heystack_.toLatin1();
		memcpy(packet->buf_.data_, ba.data(), ba.size());
		correctChecksum_.correct(packet);
		packet->ctrl_.changed_ = true;
		emit replaced(packet);
	} else {
		emit notReplaced(packet);
	}
}
