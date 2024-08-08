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

#include "net/gip.h"
#include "net/gip6.h"
#include "net/gmac.h"

namespace GFlow {
	// ----------------------------------------------------------------------------
	// MacFlowKey
	// ----------------------------------------------------------------------------
	struct MacFlowKey {
		GMac smac_;
		GMac dmac_;

		MacFlowKey() {}
		MacFlowKey(GMac smac, GMac dmac) : smac_(smac), dmac_(dmac) {}

		bool operator < (const MacFlowKey& r) const {
			if (this->smac_ < r.smac_) return true;
			if (this->smac_ > r.smac_) return false;
			if (this->dmac_ < r.dmac_) return true;
			return false;
		}

		MacFlowKey reverse() {
			return MacFlowKey(dmac_, smac_);
		}

		explicit operator QString() const { return QString("%1 %2").arg(QString(smac_)).arg(QString(dmac_)); }
	};

	// ----------------------------------------------------------------------------
	// MacSessionKey
	// ----------------------------------------------------------------------------
	struct MacSessionKey {
		GMac mac1_;
		GMac mac2_;

		MacSessionKey() {}
		MacSessionKey(GMac mac1, GMac mac2) : mac1_(mac1), mac2_(mac2) {}

		bool operator < (const MacSessionKey& r) const {
			if (this->mac1_ < r.mac1_) return true;
			if (this->mac1_ > r.mac1_) return false;
			if (this->mac2_ < r.mac2_) return true;
			return false;
		}

		explicit operator QString() const { return QString("%1 %2").arg(QString(mac1_)).arg(QString(mac2_)); }
	};

	// ----------------------------------------------------------------------------
	// IpFlowKey
	// ----------------------------------------------------------------------------
	struct IpFlowKey {
	public:
		GIp sip_;
		GIp dip_;

		IpFlowKey() {}
		IpFlowKey(GIp sip, GIp dip) : sip_(sip), dip_(dip) {}

		bool operator < (const IpFlowKey& r) const {
			if (this->sip_ < r.sip_) return true;
			if (this->sip_ > r.sip_) return false;
			if (this->dip_ < r.dip_) return true;
			return false;
		}

		IpFlowKey reverse() {
			return IpFlowKey(dip_, sip_);
		}

		explicit operator QString() const { return QString("%1 %2").arg(QString(sip_)).arg(QString(dip_)); }
	};

	// ----------------------------------------------------------------------------
	// IpSessionKey
	// ----------------------------------------------------------------------------
	struct IpSessionKey {
	public:
		GIp ip1_;
		GIp ip2_;

		IpSessionKey() {}
		IpSessionKey(GIp ip1, GIp ip2) : ip1_(ip1), ip2_(ip2) {}

		bool operator < (const IpSessionKey& r) const {
			if (this->ip1_ < r.ip1_) return true;
			if (this->ip1_ > r.ip1_) return false;
			if (this->ip2_ < r.ip2_) return true;
			return false;
		}

		explicit operator QString() const { return QString("%1 %2").arg(QString(ip1_)).arg(QString(ip2_)); }
	};

	// ----------------------------------------------------------------------------
	// PortFlowKey
	// ----------------------------------------------------------------------------
	struct PortFlowKey {
	public:
		uint16_t sport_;
		uint16_t dport_;

		PortFlowKey() {}
		PortFlowKey(uint16_t sport, uint16_t dport) : sport_(sport), dport_(dport) {}

		bool operator < (const PortFlowKey& r) const {
			if (this->sport_ < r.sport_) return true;
			if (this->sport_ > r.sport_) return false;
			if (this->dport_ < r.dport_) return true;
			return false;
		}

		PortFlowKey reverse() {
			return PortFlowKey(dport_, sport_);
		}

		explicit operator QString() const { return QString("%1 %2").arg(sport_).arg(dport_); }
	};

	// ----------------------------------------------------------------------------
	// PortSessionKey
	// ----------------------------------------------------------------------------
	struct PortSessionKey {
	public:
		uint16_t port1_;
		uint16_t port2_;

		PortSessionKey() {}
		PortSessionKey(uint16_t port1, uint16_t port2) : port1_(port1), port2_(port2) {}

		bool operator < (const PortSessionKey& r) const {
			if (this->port1_ < r.port1_) return true;
			if (this->port1_ > r.port1_) return false;
			if (this->port2_ < r.port2_) return true;
			return false;
		}

		explicit operator QString() const { return QString("%1 %2").arg(port1_).arg(port2_); }
	};

	// ----------------------------------------------------------------------------
	// TransportKey
	// ----------------------------------------------------------------------------
	struct TransportKey {
	public:
		GIp ip_;
		uint16_t port_;

		TransportKey() {}
		TransportKey(GIp ip, uint16_t port) : ip_(ip), port_(port) {}

		bool operator < (const TransportKey& r) const {
			if (this->ip_ < r.ip_) return true;
			if (this->ip_ > r.ip_) return false;
			if (this->port_ < r.port_) return true;
			return false;
		}

		explicit operator QString() const { return QString("%1:%2").arg(QString(ip_)).arg(port_); }
	};

	typedef TransportKey TcpKey;
	typedef TransportKey UdpKey;

	// ----------------------------------------------------------------------------
	// TransportFlowKey
	// ----------------------------------------------------------------------------
	struct TransportFlowKey {
	public:
		GIp sip_;
		uint16_t sport_;
		GIp dip_;
		uint16_t dport_;

		TransportFlowKey() {}
		TransportFlowKey(GIp sip, uint16_t sport, GIp dip, uint16_t dport) : sip_(sip), sport_(sport), dip_(dip), dport_(dport) {}

		bool operator < (const TransportFlowKey& r) const {
			if (this->sip_ < r.sip_) return true;
			if (this->sip_ > r.sip_) return false;
			if (this->sport_ < r.sport_) return true;
			if (this->sport_ > r.sport_)return false;
			if (this->dip_ < r.dip_) return true;
			if (this->dip_ > r.dip_) return false;
			if (this->dport_ < r.dport_) return true;
			return false;
		}

		bool operator == (const TransportFlowKey& r) const {
			if (this->sip_ != r.sip_) return false;
			if (this->sip_ != r.sip_) return false;
			if (this->sport_ != r.sport_) return false;
			if (this->sport_ != r.sport_) return false;
			if (this->dip_ != r.dip_) return false;
			if (this->dip_ != r.dip_) return false;
			if (this->dport_ != r.dport_) return false;
			return true;
		}

		TransportFlowKey reverse() {
			return TransportFlowKey(dip_, dport_, sip_, sport_);
		}

		explicit operator QString() const { return QString("%1:%2>%3%4").arg(QString(sip_)).arg(sport_).arg(QString(dip_)).arg(dport_); }
	};

	typedef TransportFlowKey TcpFlowKey;
	typedef TransportFlowKey UdpFlowKey;

	// ----------------------------------------------------------------------------
	// TransportSessionKey
	// ----------------------------------------------------------------------------
	struct TransportSessionKey {
	public:
		GIp ip1_;
		uint16_t port1_;
		GIp ip2_;
		uint16_t port2_;

		TransportSessionKey() {}
		TransportSessionKey(GIp ip1, uint16_t port1, GIp ip2, uint16_t port2) : ip1_(ip1), port1_(port1), ip2_(ip2), port2_(port2) {}

		bool operator < (const TransportSessionKey& r) const {
			if (this->ip1_ < r.ip1_) return true;
			if (this->ip1_ > r.ip1_) return false;
			if (this->port1_ < r.port1_) return true;
			if (this->port1_ > r.port1_) return false;
			if (this->ip2_ < r.ip2_) return true;
			if (this->ip2_ > r.ip2_) return false;
			if (this->port2_ < r.port2_) return true;
			return false;
		}

		explicit operator QString() const { return QString("%1:%2 %3%4").arg(QString(ip1_)).arg(port1_).arg(QString(ip2_)).arg(port2_); }
	};

	typedef TransportSessionKey TcpSessionKey;
	typedef TransportSessionKey UdpSessionKey;

	// ----------------------------------------------------------------------------
	// TupleFlowKey
	// ----------------------------------------------------------------------------
	struct TupleFlowKey {
	public:
		uint8_t proto_;
		TransportFlowKey flow_;

		TupleFlowKey() {}
		TupleFlowKey(uint8_t proto, TransportFlowKey flow) : proto_(proto), flow_(flow) {}

		bool operator < (const TupleFlowKey& r) const {
			if (this->proto_ < r.proto_) return true;
			if (this->proto_ > r.proto_) return false;
			return this->flow_ < r.flow_;
		}

		TupleFlowKey reverse() {
			return TupleFlowKey(proto_, flow_.reverse());
		}
		explicit operator QString() const { return QString("%1 %2").arg(proto_).arg(QString(flow_)); }
	};
}
