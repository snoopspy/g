#include <iostream>
#include <GApp>
#include <GNetInfo>

using namespace std;

void showNetIntf() {
	GAllInterface& allInterface = GNetInfo::instance().allInterface();
	GRtmEntry* entry = GNetInfo::instance().rtm().getBestEntry(QString("8.8.8.8"));

	int index = 1;
	for(GInterface& intf: allInterface) {
		bool best = entry != nullptr && entry->intf()->name() == intf.name();
		QString msg = QString("index %1 %2").arg(index).arg(best ? "(Best)" : "");
		cout << qPrintable(msg) << endl;

		msg = QString("  %1").arg(intf.name());
		if (intf.desc() != "") msg += QString(" / %1").arg(intf.desc());
		cout << qPrintable(msg) << endl;

		msg = "";
		if (!intf.mac().isNull()) msg += QString("mac:%1 ").arg(QString(intf.mac()));
		if (intf.ip() != 0) msg += QString("ip:%1 ").arg(QString(intf.ip()));
		if (intf.mask() != 0) msg += QString("mask:%1 ").arg(QString(intf.mask()));
		if (intf.gateway() != 0) msg += QString("gateway:%1 ").arg(QString(intf.gateway()));
		if (msg != "") {
			msg = QString("  %1").arg(msg);
			cout << qPrintable(msg) << endl;
		}

		cout << endl;
		index++;
	}
}

void showRtm() {
	GRtm& rtm = GNetInfo::instance().rtm();

	cout << "dst             mask            gateway         metric intf" << endl;
	for (GRtmEntry& entry: rtm) {
		QString dstStr = QString(entry.dst()); dstStr += QString(" ").repeated(16 - dstStr.length());
		QString maskStr = QString(entry.mask()); maskStr += QString(" ").repeated(16 - maskStr.length());
		QString gatewayStr = QString(entry.gateway()); gatewayStr += QString(" ").repeated(16 - gatewayStr.length());
		QString metricStr = QString::number(entry.metric()); metricStr += QString(" ").repeated(7 - metricStr.length());
		QString intfStr = QString("%1 %2").arg(entry.intf()->index()).arg(entry.intf()->desc());
		QString msg = dstStr + maskStr + gatewayStr + metricStr + intfStr;
		cout << qPrintable(msg) << endl;
	}
}

int main() {
	GApp::init();
	showNetIntf();
	showRtm();
}
