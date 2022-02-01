// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include "base/gstateobj.h"
#include "base/sys/gthread.h"
#include "base/sys/gwaitevent.h"
#include "net/packet/gpacket.h"
#include "net/write/gwrite.h"

// ----------------------------------------------------------------------------
// GDot11Block
// ----------------------------------------------------------------------------
struct G_EXPORT GDot11Block : GStateObj {
	Q_OBJECT
	Q_PROPERTY(bool enabled MEMBER enabled_)
	Q_PROPERTY(bool attackOnPacket MEMBER attackOnPacket_)
	Q_PROPERTY(ulong attackInterval MEMBER attackInterval_)
	Q_PROPERTY(ulong sendInterval MEMBER sendInterval_)
	Q_PROPERTY(ulong deleteTimeout MEMBER deleteTimeout_)
	Q_PROPERTY(bool debugLog MEMBER debugLog_)
	Q_PROPERTY(bool authStaAp MEMBER authStaAp_);
	Q_PROPERTY(bool deauthApBroadcast MEMBER deauthApBroadcast_);
	Q_PROPERTY(bool deauthStaAp MEMBER deauthStaAp_);
	Q_PROPERTY(bool disassociateStaAp MEMBER disassociateStaAp_);
	Q_PROPERTY(bool timAttack MEMBER timAttack_);
	Q_PROPERTY(GObjPtr writer READ getWriter WRITE setWriter)

public:
	GObjPtr getWriter() { return writer_; }
	void setWriter(GObjPtr value) { writer_ = dynamic_cast<GWrite*>(value.data()); }

public:
	bool enabled_{true};
	bool attackOnPacket_{true};
	GDuration attackInterval_{100}; // 10 msecs
	GDuration sendInterval_{1}; // 1 msecs
	GDuration deleteTimeout_{60000}; // 1 minute
	bool debugLog_{false};
	bool authStaAp_{false};
	bool deauthApBroadcast_{true};
	bool deauthStaAp_{false};
	bool disassociateStaAp_{false};
	bool timAttack_{false};
	GWrite* writer_{nullptr};

public:
	Q_INVOKABLE GDot11Block(QObject* parent = nullptr);
	~GDot11Block() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QAtomicInt currentChannel_{0};

protected:
	struct DeauthFrame {
		GRadioHdr radioHdr_;
		GDeauthHdr deauthHdr_;
	};

	struct Ap {
		int channel_;
		GMac mac_;
		qint64 lastAccess_{0};
		DeauthFrame deauthFrame_;
		QByteArray timFrame_;
		QList<GMac> staList_;
	};

	typedef class QMap<GMac, Ap> Aps;

	struct ApMap : QMap<int, Aps> {
		QMutex m_;
	} apMap_;

	void processBeacon(GPacket* packet);

	void attack(Ap& ap);
	QByteArray extractBeaconTimFrame(GPacket *packet);
	void attackRun();
	void deleteRun();

	struct AttackThread : GThread {
		AttackThread(QObject *parent) : GThread(parent) {}
		~AttackThread() override {}
		GWaitEvent we_;
		void run() override {
			GDot11Block* d11b = dynamic_cast<GDot11Block*>(parent());
			Q_ASSERT(d11b != nullptr);
			d11b->attackRun();
		}
	} attackThread_{this};

	struct DeleteThread : GThread {
		DeleteThread(QObject *parent) : GThread(parent) {}
		~DeleteThread() override {}
		GWaitEvent we_;
		void run() override {
			GDot11Block* d11b = dynamic_cast<GDot11Block*>(parent());
			Q_ASSERT(d11b != nullptr);
			d11b->deleteRun();
		}
	} deleteThread_{this};

public slots:
	void block(GPacket* packet);
	void processChannelChanged(int channel);

signals:
	void blocked(GPacket* packet);
};
