#pragma once

#include <QSsl>
#include <GObj>

// ----------------------------------------------------------------------------
// Option
// ----------------------------------------------------------------------------
struct NetClient : GObj {
	Q_OBJECT
	Q_PROPERTY(QString host MEMBER host_)
	Q_PROPERTY(quint16 port MEMBER port_)
	Q_PROPERTY(QString localHost MEMBER localHost_)
	Q_PROPERTY(quint16 localPort MEMBER localPort_)

public:
	QString host_{""};
	quint16 port_{0};
	QString localHost_{""};
	quint16 localPort_{0};

public:
};

struct TcpClient : NetClient {
};

struct UdpClient : NetClient {
};

struct SslClient : NetClient {
	Q_OBJECT
	Q_PROPERTY(int protocol READ protocol() WRITE setProtocol)

protected:
	int protocol() { return (int) protocol_; }
	void setProtocol(int protocol) { protocol_ = (QSsl::SslProtocol)protocol; }

public:
	QSsl::SslProtocol protocol_;
};

struct Option : GObj {
	Q_OBJECT
	Q_PROPERTY(GObjRef tcpClient READ getTcpClient)
	Q_PROPERTY(GObjRef udpClient READ getUdpClient)
	Q_PROPERTY(GObjRef sslClient READ getSslClient)

	GObjRef getTcpClient() { return &tcpClient_; }
	GObjRef getUdpClient() { return &udpClient_; }
	GObjRef getSslClient() { return &sslClient_; }

public:
	TcpClient tcpClient_;
	UdpClient udpClient_;
	SslClient sslClient_;
};
