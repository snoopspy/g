#include "webserver.h"

#include <QToolButton>

#include "hostanalyzer.h"

WebServer::WebServer(QObject* parent) : GStateObj(parent) {
	tcpServer_ = new QTcpServer(this);

	QObject::connect(tcpServer_, &QTcpServer::newConnection, this, &WebServer::tcpServer_newConnection);
}

WebServer::~WebServer()
{
	close();
}

bool WebServer::doOpen() {
	if (!tcpServer_->listen(QHostAddress::Any, port_)) {
		SET_ERR(GErr::Fail, QString("can not listen port %1").arg(port_));
		return false;
	}
	return true;
}

bool WebServer::doClose() {
	tcpServer_->close();
	return true;
}

WebServer::Session::Session() {
	created_ = QDateTime::currentSecsSinceEpoch();
	bytes_.resize(SessionSize);
	for (int i = 0; i < SessionSize; i++)
		bytes_[i] = std::rand() % 10 + '0';
}

QString WebServer::Session::bytesToString() {
	QString res;
	for (int i = 0; i < SessionSize; i++)
		res += bytes_[i];
	return res;
}

void WebServer::SessionList::deleteOldSession() {
	time_t now = QDateTime::currentSecsSinceEpoch();
	for (int i = 0; i < count();) {
		const Session& session = at(i);
		time_t created = session.created_;
		if (created + webServer_->sessionTimeOutSec_ < now) {
			this->removeAt(i);
		} else
			i++;
	}
}

void WebServer::tcpServer_newConnection() {
	QTcpSocket *socket = tcpServer_->nextPendingConnection();
	QObject::connect(socket, SIGNAL(readyRead()), SLOT(tcpServer_ReadyRead()));
}

void WebServer::tcpServer_ReadyRead() {
	QTcpSocket* socket = dynamic_cast<QTcpSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	QByteArray ba = socket->readAll();
	if (ba.startsWith("GET /favicon.ico")) {
		socket->close();
		return;
	}

	QByteArray bytes;
	int count = ba.length();
	for (int i = 4; i < count; i++) { // 4 is sizeof "GET "
		char c = ba.at(i);
		if (std::isdigit(c)) {
			bytes += c;
			if (bytes.length() >= Session::SessionSize)
				break;
			continue;
		}
		if (c == ' ') break;
	}

	if (bytes.length() != Session::SessionSize) {
		socket->write("HTTP/1.1 200 OK\r\n\r\n<br><br><br><br><h1><center>Invalid session</center><h1>");
		socket->close();
		return;
	}

	sessionList_.deleteOldSession();
	bool sessionFound = false;
	for (Session& session: sessionList_) {
		if (session.bytes_ == bytes) {
			qDebug() << "matched" << bytes;
			sessionFound = true;
			break;
		}
	}
	if (!sessionFound) {
		socket->write("HTTP/1.1 200 OK\r\n\r\n<br><br><br><br><h1><center>Session expired</center><h1>");
		socket->close();
		return;
	}

	GIp peerIp = socket->peerAddress().toIPv4Address();

	HostAnalyzer* hostAnalyzer = dynamic_cast<HostAnalyzer*>(parent());
	Q_ASSERT(hostAnalyzer != nullptr);
	GTreeWidget* treeWidget = hostAnalyzer->treeWidget_;
	int count2 = treeWidget->topLevelItemCount();

	GHostMgr::HostMap* hostMap = &hostAnalyzer->hostMgr_.hostMap_;
	QMutexLocker ml(hostMap);
	bool found = false;
	for (int i = 0; i < count2; i++) {
		GTreeWidgetItem* twi = dynamic_cast<GTreeWidgetItem*>(treeWidget->topLevelItem(i));
		Q_ASSERT(twi != nullptr);
		QToolButton* toolButton = dynamic_cast<QToolButton*>(treeWidget->itemWidget(twi, HostAnalyzer::ColumnAttack));
		Q_ASSERT(toolButton != nullptr);

		GMac mac = toolButton->property("mac").toString();
		GHostMgr::HostMap::iterator it = hostMap->find(mac);
		if (it == hostMap->end()) continue;
		GHostMgr::HostValue* hostValue = it.value();
		Q_ASSERT(hostValue != nullptr);
		if (hostValue->ip_ == peerIp) {
			found = true;
			qDebug() << QString("found toolButton for %1").arg(QString(peerIp));
			if (toolButton->text() == "B") // block state
				toolButton->click();
			break;
		}
	}

	if (found) {
		socket->write("HTTP/1.1 200 OK\r\n\r\n<br><br><br><br><h1><center>Internet is connected</center><h1>");
	} else {
		qWarning() << QString("can not find %1").arg(QString(peerIp));
		socket->write("HTTP/1.1 503 Service Unavailable\r\n\r\n<br><br><br><br><h1><center>Can not find session</center><h1>");
	}
	socket->close();
}
