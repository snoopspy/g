#include "gfind.h"

// ----------------------------------------------------------------------------
// GFind
// ----------------------------------------------------------------------------
GFind::GFind(QObject* parent) : GStateObj(parent) {
	qDebug() << "";
}

GFind::~GFind() {
	qDebug() << "";
}

bool GFind::doOpen() {
	qDebug() << "";
	for (GObj* obj: items_) {
		GFindItem* item = PFindItem(obj);
		if (item->count_ == 0) {
			SET_ERR(GErr::ValueIsZero, "count can not be zero");
			return false;
		}
		if (item->pattern_ == "") {
			SET_ERR(GErr::ValueIsNull, "pattern can not be blank");
			return false;
		}
		GFindItem::Type type = item->type_;
		switch (type) {
			case GFindItem::String :
				break;
			case GFindItem::HexValue : {
				QByteArray ba = QByteArray::fromHex(item->pattern_.toUtf8());
				item->hexPattern_ = QString::fromLatin1(ba.data(), ba.size());
				break;
			}
			case GFindItem::RegularExpression :
				item->re_.setPattern(item->pattern_);
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

void GFind::find(GPacket* packet) {
	QString segment;
	QString fullPacket;
	QString* text = nullptr;
	for (GObj* obj: items_) {
		GFindItem* item = PFindItem(obj);
		GFindItem::Category category = item->category_;
		switch (category) {
			case GFindItem::Segment:
				if (segment.isEmpty())
					segment = makeSegment(packet);
				if (segment.isEmpty())
					continue;
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

		GFindItem::Type type = item->type_;
		int index = item->offset_;
		int count = item->count_;
		bool _found = false;
		while (index < text->size()) {
			int foundIndex;
			QString captured;
			switch (type) {
				case GFindItem::String :
					foundIndex = text->indexOf(item->pattern_, index);
					if (foundIndex != -1) captured = item->pattern_;
					break;
				case GFindItem::HexValue :
					foundIndex = text->indexOf(item->hexPattern_, index);
					if (foundIndex != -1) captured = item->hexPattern_;
					break;
				case GFindItem::RegularExpression :
					QRegularExpressionMatch m = item->re_.match(*text, index);
					if (m.hasMatch()) {
						foundIndex = m.capturedStart(0);
						captured = m.captured(0);
					} else {
						foundIndex = -1;
					}
					break;
			}
			if (foundIndex == -1) break;
			int endOffset = item->endOffset_;
			if (endOffset != -1 && foundIndex >= endOffset) break;

			_found = true;
			if (log_) {
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
			index = foundIndex + captured.size();
			if (count != -1) {
				if (--count <= 0)
					break;
			}
		}
		if (_found)
			emit found(packet);
		else
			emit notFound(packet);
	}
}
