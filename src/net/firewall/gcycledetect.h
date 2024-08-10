#pragma once

#include <GStateObj>
#include <GTcpFlowMgr>

// ----------------------------------------------------------------------------
// GCycleItemKey
// ----------------------------------------------------------------------------
struct GCycleItemKey {
	GCycleItemKey(GIp clientIp, GIp serverIp, uint16_t serverPort, uint8_t ttl) :
		clientIp_(clientIp), serverIp_(serverIp), serverPort_(serverPort), ttl_(ttl) {}
	GIp clientIp_;
	GIp serverIp_;
	uint16_t serverPort_;
	uint8_t ttl_;

	bool operator < (const GCycleItemKey& r) const {
		if (this->clientIp_ < r.clientIp_) return true;
		if (this->clientIp_ > r.clientIp_) return false;
		if (this->serverIp_ < r.serverIp_) return true;
		if (this->serverIp_ > r.serverIp_) return false;
		if (this->serverPort_ < r.serverPort_) return true;
		if (this->serverPort_ > r.serverPort_) return false;
		if (this->ttl_ < r.ttl_) return true;
		return false;
	}
};

// ----------------------------------------------------------------------------
// GCycleItem
// ----------------------------------------------------------------------------
struct GCycleItem {
	struct TimevalList : QList<struct timeval> {
		double diffAvg_{-1};
		void check(QString prefix);
	} firstTimes_, lastTimes_;

	struct Quint64List : QList<quint64> {
		double diffAvg_{-1};
		void check(QString prefix);
	} txPackets_, txBytes_, rxPackets_, rxBytes_;

	void* user_{nullptr}; // used for QTreeWidgetItem
};

// ----------------------------------------------------------------------------
// GCycleMap
// ----------------------------------------------------------------------------
struct GCycleMap : QMap<GCycleItemKey, GCycleItem> {
};

// ----------------------------------------------------------------------------
// GCycleDetect
// ----------------------------------------------------------------------------
struct G_EXPORT GCycleDetect : GStateObj, GTcpFlowMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(QString prop MEMBER prop_)
	Q_PROPERTY(int minCheckCount MEMBER minCheckCount_)
	Q_PROPERTY(int maxCheckCount MEMBER maxCheckCount_)
	Q_PROPERTY(GObjPtr tcpFlowMgr READ getTcpFlowMgr WRITE setTcpFlowMgr)

	GObjPtr getTcpFlowMgr() { return tcpFlowMgr_; }
	void setTcpFlowMgr(GObjPtr value) { tcpFlowMgr_ = dynamic_cast<GTcpFlowMgr*>(value.data()); }

public:
	QString prop_;
	int minCheckCount_{3};
	int maxCheckCount_{100};
	GTcpFlowMgr* tcpFlowMgr_{nullptr};

public:
	Q_INVOKABLE GCycleDetect(QObject* parent = nullptr) : GStateObj(parent) {}
	~GCycleDetect() override { close(); }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GCycleMap tcpMap_;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		Item(uint8_t ttl) : ttl_(ttl) {}
		uint8_t ttl_;
	};
	typedef Item* PItem;
	// --------------------------------------------------------------------------

	// GTcpFlowMgr::Managable
	size_t itemOffset_{0};
	Item* getItem(GTcpFlowMgr::TcpFlowValue* tcpFlowValue) { return PItem(tcpFlowValue->mem(itemOffset_)); }
	void tcpFlowCreated(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;
	void tcpFlowDeleted(GFlow::TcpFlowKey tcpFlowKey, GTcpFlowMgr::TcpFlowValue* tcpFlowValue) override;

signals:
	void created(GCycleItemKey key, GCycleItem* item);
	void updated(GCycleItemKey key, GCycleItem* item);
	void deleted(GCycleItemKey key, GCycleItem* item);
};
