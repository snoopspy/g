#include "gfind.h"

// ----------------------------------------------------------------------------
// GFind
// ----------------------------------------------------------------------------
GFind::GFind(QObject* parent) : GStateObj(parent) {
	qDebug() << "";
}

GFind::~GFind() {
	qDebug() << "";
	close();
}

bool GFind::doOpen() {
	qDebug() << "";
	for (GObj* obj: findItems_) {
		GFindItem* findItem = PFindItem(obj);
		qDebug() << findItem->pattern_; // gilgil temp 2023.11.27
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
	qDebug() << "";
	return true;
}

QString GFind::makeSegment(GPacket* packet) {
	QString res;
	if (packet->tcpData_.valid()) {
		res = QString::fromLatin1(pchar(packet->tcpData_.data_), packet->tcpData_.size_);
	} else
	if (packet->udpData_.valid()) {
		res = QString::fromLatin1(pchar(packet->udpData_.data_), packet->udpData_.size_);
	}
	return res;
}

QString GFind::makeFullPacket(GPacket* packet) {
	return QString::fromLatin1(pchar(packet->buf_.data_), packet->buf_.size_);
}

void GFind::processFound(int itemIndex, int foundIndex, QString& captured, QString& text) {
	(void)itemIndex;
	(void)text;

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
	qInfo() << QString("found('%1' %2 0x%3)").arg(logCaptured).arg(foundIndex).arg(QString::number(foundIndex, 16));
}

bool GFind::find(GPacket* packet) {
	bool _found = false;
	QString segment;
	QString fullPacket;
	QString* text = nullptr;
	int itemIndex = 0;
	for (GObj* obj: findItems_) {
		GFindItem* findItem = PFindItem(obj);
		GFindItem::Category category = findItem->category_;
		switch (category) {
			case GFindItem::Segment:
				if (segment.isEmpty())
					segment = makeSegment(packet);
				text = &segment;
				break;
			case GFindItem::FullPacket:
				if (fullPacket.isEmpty())
					fullPacket = makeFullPacket(packet);
				Q_ASSERT(!fullPacket.isEmpty());
				text = &fullPacket;
				break;
		}
		Q_ASSERT(text != nullptr);

		GFindItem::Type type = findItem->type_;
		int index = findItem->offset_;
		int count = findItem->count_;
		while (index < text->size()) {
			int foundIndex;
			QString captured;
			switch (type) {
				case GFindItem::String :
					foundIndex = text->indexOf(findItem->pattern_, index);
					if (foundIndex != -1) captured = findItem->pattern_;
					break;
				case GFindItem::HexValue :
					foundIndex = text->indexOf(findItem->findHexPattern_, index);
					if (foundIndex != -1) captured = findItem->findHexPattern_;
					break;
				case GFindItem::RegularExpression :
					QRegularExpressionMatch m = findItem->findRe_.match(*text, index);
					if (m.hasMatch()) {
						foundIndex = m.capturedStart(0);
						captured = m.captured(0);
					} else {
						foundIndex = -1;
					}
					break;
			}
			if (foundIndex == -1) break;
			int endOffset = findItem->endOffset_;
			if (endOffset != -1 && foundIndex >= endOffset) break;

			_found = true;
			processFound(itemIndex, foundIndex, captured, *text);
			index = foundIndex + captured.size();
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
	return _found;
}
