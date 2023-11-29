#include "gfind.h"

// ----------------------------------------------------------------------------
// GFind
// ----------------------------------------------------------------------------
GFind::GFind(QObject* parent) : GStateObj(parent) {
}

GFind::~GFind() {
	close();
}

bool GFind::doOpen() {
	if (!enabled_) return true;

	for (GObj* obj: findItems_) {
		GFindItem* findItem = PFindItem(obj);
		if (findItem->count_ == 0) {
			SET_ERR(GErr::ValueIsZero, "count can not be zero");
			return false;
		}
		if (findItem->pattern_ == "") {
			SET_ERR(GErr::ValueIsNull, "pattern can not be blank");
			return false;
		}
		GFindItem::Type type = findItem->type_;
		switch (type) {
			case GFindItem::String :
				break;
			case GFindItem::HexValue : {
				QByteArray ba = QByteArray::fromHex(findItem->pattern_.toUtf8());
				findItem->findHexPattern_ = QString::fromLatin1(ba.data(), ba.size());
				break;
			}
			case GFindItem::RegularExpression :
				findItem->findRe_.setPattern(findItem->pattern_);
				break;
		}
	}
	return true;
}

bool GFind::doClose() {
	return true;
}

QString GFind::printableStr(QString s) {
	bool isPrintable = true;
	for (QChar ch: s) {
		if (!ch.isPrint()) {
			isPrintable = false;
			break;
		}
	}
	if (isPrintable) return s;

	QByteArray ba;
	for (QChar ch: s) {
		ba += ch.toLatin1();
	}
	s = "0x" + ba.toHex().toUpper();
	return s;
}

void GFind::processFound(int itemIndex, int foundIndex, QString& foundStr) {
	(void)itemIndex;

	if (!log_) return;
	QString logFoundStr = printableStr(foundStr);
	qInfo() << QString("found('%1' %2 0x%3)").arg(logFoundStr).arg(foundIndex).arg(QString::number(foundIndex, 16));
}

void GFind::find(GPacket* packet) {
	if (!enabled_) return;

	bool _found = false;
	heystack_ = QString::fromLatin1(pchar(packet->buf_.data_), packet->buf_.size_);
	int itemIndex = 0;

	for (GObj* obj: findItems_) {
		GFindItem* findItem = PFindItem(obj);
		GFindItem::Type type = findItem->type_;
		int index = findItem->offset_;
		int count = findItem->count_;
		while (index < heystack_.size()) {
			int foundIndex = -1;
			QString foundStr;
			switch (type) {
				case GFindItem::String :
					foundIndex = heystack_.indexOf(findItem->pattern_, index);
					if (foundIndex != -1) foundStr = findItem->pattern_;
					break;
				case GFindItem::HexValue :
					foundIndex = heystack_.indexOf(findItem->findHexPattern_, index);
					if (foundIndex != -1) foundStr = findItem->findHexPattern_;
					break;
				case GFindItem::RegularExpression :
					QRegularExpressionMatch m = findItem->findRe_.match(heystack_, index);
					if (m.hasMatch()) {
						foundIndex = m.capturedStart(0);
						foundStr = m.captured(0);
					} else {
						foundIndex = -1;
					}
					break;
			}
			if (foundIndex == -1) break;
			int endOffset = findItem->endOffset_;
			if (endOffset != -1 && foundIndex > endOffset) break;

			_found = true;
			processFound(itemIndex, foundIndex, foundStr);
			index = foundIndex + foundStr.size();
			if (count != -1) {
				if (--count <= 0)
					break;
			}
		}
		itemIndex++;
	}
	if (_found)
		emit found(packet);
	else
		emit notFound(packet);
}
