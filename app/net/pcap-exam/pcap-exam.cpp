#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <GApp>
#include <GJson>
#include <GPcapDevice>

using namespace std;

struct Obj : QObject {
	Q_OBJECT

public slots:
	void captured(GPacket* packet) {
		QString smac;
		QString dmac;
		GEthHdr* ethHdr = packet->ethHdr_;
		if (ethHdr != nullptr) {
			smac = QString(ethHdr->smac());
			dmac = QString(ethHdr->dmac());
		}

		GIpHdr* ipHdr = packet->ipHdr_;
		if (ipHdr == nullptr) return;
		QString sip = QString(ipHdr->sip());
		QString dip = QString(ipHdr->dip());

		QString proto, sport, dport;
		GTcpHdr* tcpHdr = packet->tcpHdr_;
		if (tcpHdr != nullptr) {
			proto = "tcp";
			sport = QString::number(tcpHdr->sport());
			dport = QString::number(tcpHdr->dport());
		} else {
			GUdpHdr* udpHdr = packet->udpHdr_;
			if (udpHdr != nullptr) {
				proto = "udp";
				sport = QString::number(udpHdr->sport());
				dport = QString::number(udpHdr->dport());
			} else {
				return;
			}
		}

		static int count = 0;
		QString msg = QString("%1 %2 %3 > %4 %5:%6 > %7:%8\n")
			.arg(++count).arg(proto, smac, dmac, sip, sport, dip, dport);
		cout << qPrintable(msg);
	}

	void processClose() {
		qDebug() << "processClose";
		QCoreApplication::quit();
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv);

	GPcapDevice device;
	QJsonObject jo = GJson::loadFromFile();
	jo["device"] >> device;
	jo["device"] << device;
	GJson::saveToFile(jo);

	Obj obj;
	QObject::connect(&device, &GPcapDevice::captured, &obj, &Obj::captured, Qt::DirectConnection);
	QObject::connect(&device, &GPcapDevice::closed, &obj, &Obj::processClose);

	if (!device.open()) {
		cout << device.err;
		return EXIT_FAILURE;
	}

	return a.exec();
}

#include "pcap-exam.moc"
