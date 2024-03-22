#pragma once

#include <QSsl>
#include <GObj>

// ----------------------------------------------------------------------------
// Option
// ----------------------------------------------------------------------------
struct NetServer : GObj {
	Q_OBJECT
	Q_PROPERTY(quint16 port MEMBER port_)
	Q_PROPERTY(QString localHost MEMBER localHost_)
	Q_PROPERTY(quint16 localPort MEMBER localPort_)

public:
	quint16 port_{0};
	QString localHost_{""};
	quint16 localPort_{0};

public:
};

struct TcpServer : NetServer {
};

struct UdpServer : NetServer {
};

struct SslServer : NetServer {
	Q_OBJECT
	Q_PROPERTY(SslProtocol protocol MEMBER protocol_)
	Q_ENUMS(SslProtocol)

public:
	enum SslProtocol {
		TlsV1_0 QT_DEPRECATED_VERSION_X_6_3("Use TlsV1_2OrLater instead."),
		TlsV1_1 QT_DEPRECATED_VERSION_X_6_3("Use TlsV1_2OrLater instead."),
		TlsV1_2,
		AnyProtocol,
		SecureProtocols,

		TlsV1_0OrLater QT_DEPRECATED_VERSION_X_6_3("Use TlsV1_2OrLater instead."),
		TlsV1_1OrLater QT_DEPRECATED_VERSION_X_6_3("Use TlsV1_2OrLater instead."),
		TlsV1_2OrLater,

		DtlsV1_0 QT_DEPRECATED_VERSION_X_6_3("Use DtlsV1_2OrLater instead."),
		DtlsV1_0OrLater QT_DEPRECATED_VERSION_X_6_3("Use DtlsV1_2OrLater instead."),
		DtlsV1_2,
		DtlsV1_2OrLater,

		TlsV1_3,
		TlsV1_3OrLater,

		UnknownProtocol = -1
	};

public:
	SslProtocol protocol_{TlsV1_2};
};

struct Option : GObj {
	Q_OBJECT
	Q_PROPERTY(GObjRef tcpServer READ getTcpServer)
	Q_PROPERTY(GObjRef udpServer READ getUdpServer)
	Q_PROPERTY(GObjRef sslServer READ getSslServer)

	GObjRef getTcpServer() { return &tcpServer_; }
	GObjRef getUdpServer() { return &udpServer_; }
	GObjRef getSslServer() { return &sslServer_; }

public:
	TcpServer tcpServer_;
	UdpServer udpServer_;
	SslServer sslServer_;
};
