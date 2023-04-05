#include "glogudp.h"

// ----------------------------------------------------------------------------
// GLogUdp
// ----------------------------------------------------------------------------
GLogUdp::GLogUdp(QObject* parent) : GLog(parent) {
}

GLogUdp::~GLogUdp() {
	if (sock_ != 0) {
		::close(sock_ );
		sock_ = 0;
	}
}

void GLogUdp::write(QString& msg) {
	if (!configured_) {
		configure();
		configured_ = true;
	}
    ::sendto(sock_, qPrintable(msg), msg.size(), 0, (struct sockaddr*)&addr_in_, sizeof(struct sockaddr_in ));
}

void GLogUdp::configure() {
#ifdef WIN32
	static bool first = true;
	if (first) {
		WSADATA wsaData;
		WSAStartup(0x0202, &wsaData);
		first = false;
	}
#endif // WIN32

	sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_ == -1) {
		fprintf(stderr, "socket return null\n");
		return;
	} else {
        addr_in_.sin_family = AF_INET;
        addr_in_.sin_port = htons(port_);
        addr_in_.sin_addr.s_addr = inet_addr(qPrintable(ip_));
        memset(addr_in_.sin_zero, 0, sizeof(addr_in_.sin_zero));
	}
}
