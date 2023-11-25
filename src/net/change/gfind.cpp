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
			case GFindItem::HexValue :
				item->hexPattern_ = QByteArray::fromHex(item->pattern_.toUtf8());
				break;
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
		res = QByteArray(pchar(packet->tcpData_.data_), packet->tcpData_.size_);
	} else
	if (packet->udpData_.valid()) {
		res = QByteArray(pchar(packet->udpData_.data_), packet->udpData_.size_);
	}
	return res;
}

QString GFind::makeFullPacket(GPacket* packet) {
	return QByteArray(pchar(packet->buf_.data_), packet->buf_.size_);
}

void GFind::find(GPacket* packet) {
	(void)packet;
	QString segment;
	QString fullPacket;
	QString* subject = nullptr;
	for (GObj* obj: items_) {
		GFindItem* item = PFindItem(obj);
		GFindItem::Category category = item->category_;
		switch (category) {
			case GFindItem::Segment:
				if (segment.isEmpty())
					segment = makeSegment(packet);
				if (segment.isEmpty())
					continue;
				subject = &segment;
				break;
			case GFindItem::FullPacket: break;
				if (fullPacket.isEmpty())
					fullPacket = makeFullPacket(packet);
				Q_ASSERT(!fullPacket.isEmpty());
				subject = &fullPacket;
				break;
		}
		GFindItem::Type type = item->type_;

		int index = item->offset_;
		int count = item->count_;
		bool _found = false;
		while (index < subject->size()) {
			int incOffset;
			switch (type) {
				case GFindItem::String :
					index = subject->indexOf(item->pattern_, index);
					if (index != -1) incOffset = item->pattern_.size();
					break;
				case GFindItem::HexValue :
					index = subject->indexOf(item->hexPattern_, index);
					if (index != -1) incOffset = item->hexPattern_.size();
					break;
				case GFindItem::RegularExpression :
					QRegularExpressionMatch m = item->re_.match(*subject, index);
					if (m.hasMatch()) {
						index = m.capturedStart(0);
						incOffset = m.capturedLength(0);
					} else {
						index = -1;
					}
					break;
			}
			if (index == -1) break;
			_found = true;
			if (log_) {
				qInfo() << QString("found(%1 %2 0x%3)").arg(item->pattern_).arg(index).arg(QString::number(index, 16));
			}
			index += incOffset;
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
