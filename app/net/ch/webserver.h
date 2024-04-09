#pragma once

#include <GStateObj>
#include <QAbstractSocket>
#include <QSslServer>
#include <QTcpServer>
#include <GMac>
#include <GIp>

struct CookieHijack;
struct G_EXPORT WebServer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int httpPort MEMBER httpPort_)
	Q_PROPERTY(int httpsPort MEMBER httpsPort_)
	Q_PROPERTY(QStringList goodbyeMessage MEMBER goodbyeMessage_)
	Q_PROPERTY(QString keyFileName MEMBER keyFileName_)
	Q_PROPERTY(QString crtFileName MEMBER crtFileName_)

public:
	int httpPort_{8080};
	int httpsPort_{4433};
	QString keyFileName_{"cert/default.key"};
	QString crtFileName_{"cert/default.crt"};
	QStringList goodbyeMessage_{"HTTP/1.1 302 Goodbye ", "Location: https://one.one.one.one", "Connection: close", "", ""};


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
	QRegularExpression reSiteNo_;

public:
	void prepareAbstractSocket(QAbstractSocket* socket);
	void prepareSslSocket(QSslSocket* socket);
	void prepareTcpServer(QTcpServer* server);
	void prepareSslServer(QSslServer* server);

public:
	typedef	QMap<QAbstractSocket* /*socket*/,  QString /*segment*/> Map;
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

public slots:
	void doHijacked(time_t created, GMac mac, GIp ip, QString host, QString cookie);

#ifdef QT_GUI_LIB
public:
	GPropItem* propCreateItem(GPropItemParam* param) override;
#endif // QT_GUI_LIB
};
