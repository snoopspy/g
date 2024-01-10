#pragma once

#include <QTcpServer>
#include <QTcpSocket>
#include <GStateObj>

struct WebServer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int port MEMBER port_)
	Q_PROPERTY(int sessionTimeOutSec MEMBER sessionTimeOutSec_)

public:
	int port_{1234};
	int sessionTimeOutSec_{600}; // 10 minutes

public:
	WebServer(QObject* parent);
	~WebServer() override;

public:
	QTcpServer* tcpServer_;

protected:
	bool doOpen() override;
	bool doClose() override;

public slots:
	void tcpServer_newConnection();
	void tcpServer_ReadyRead();

public:
	struct Session {
		static const int SessionSize = 8;
		Session();
		QString bytesToString();
		time_t created_;
		QByteArray bytes_;
	};

	struct SessionList : QList<Session> {
		SessionList(WebServer* webServer) : webServer_(webServer) {}
		WebServer* webServer_;
		void deleteOldSession();
	} sessionList_{this};
};
