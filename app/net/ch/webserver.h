#pragma once

#include <GStateObj>
#include <QAbstractSocket>
#include <QSslServer>
#include <QTcpServer>

struct CookieHijack;
struct G_EXPORT WebServer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int httpPort MEMBER httpPort_)
	Q_PROPERTY(int httpsPort MEMBER httpsPort_)
	Q_PROPERTY(bool hijackSsl MEMBER hijackSsl_)
	Q_PROPERTY(QStringList goodbyeMessage MEMBER goodbyeMessage_)

public:
	int httpPort_{8080};
	int httpsPort_{4433};
	bool hijackSsl_{true};
	QStringList goodbyeMessage_{"HTTP/1.1 302 Redirect", "Location: https://one.one.one.one", "", ""};

public:
	Q_INVOKABLE explicit WebServer(QObject *parent = nullptr);
	~WebServer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	CookieHijack* ch_;
	QTcpServer* tcpServer_{nullptr};
	QSslServer* sslServer_{nullptr};
	void prepareAbstractSocket(QAbstractSocket* socket);
	void prepareSslSocket(QSslSocket* socket);
	void prepareTcpServer(QTcpServer* server);
	void prepareSslServer(QSslServer* server);

public:
	typedef	QMap<QAbstractSocket*,  QString> Map;
	Map segments_;

public slots:
	// QIODevice
	void doReadyRead();

	// QAbstractSocket
	void doConnected();
	void doDisconnected();
	void doErrorOccurred(QAbstractSocket::SocketError socketError);
	void doStateChanged(QAbstractSocket::SocketState socketState);

	// QSslSocket
	void doHandshakeInterruptedOnError(const QSslError &error);
	void doModeChanged(QSslSocket::SslMode mode);
	void doPeerVerifyError(const QSslError &error);
	void doSslErrors(const QList<QSslError> &errors);

	// QTcpServer
	void serverAcceptError(QAbstractSocket::SocketError socketError);
	void serverNewConnection();
	void serverPendingConnectionAvailable();

	// QSslServer
	void serverErrorOccurred(QSslSocket *socket, QAbstractSocket::SocketError socketError);
	void serverSslErrors(QSslSocket *socket, const QList<QSslError> &errors);
	void serverStartedEncryptionHandshake(QSslSocket *socket);
};
