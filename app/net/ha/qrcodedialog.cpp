#include "qrcodedialog.h"

#include <GJson>

QrCodeDialog::QrCodeDialog(QWidget* parent) : QDialog(parent) {
	resize(QSize(640, 480));
	setWindowTitle("QR Code");

	lblQrCode_ = new QLabel(this);
	pbGenerate_ = new QPushButton(this);
	tcpServer_ = new QTcpServer(this);
	generator_ = new QrCodeGenerator(this);

	lblQrCode_->setAlignment(Qt::AlignCenter);
	pbGenerate_->setText("Generate QR Code");

	QObject::connect(tcpServer_, &QTcpServer::newConnection, this, &QrCodeDialog::tcpServer_newConnection, Qt::DirectConnection);
	if (!tcpServer_->listen(QHostAddress::Any, port_)) {
		QMessageBox mb;
		mb.warning(this, "Error", QString("Can not open port %1").arg(port_));
	}

	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->addWidget(lblQrCode_);
	vLayout->addWidget(pbGenerate_);
	this->setLayout(vLayout);

	QObject::connect(pbGenerate_, &QPushButton::clicked, this, &QrCodeDialog::pbGenerate_clicked);

	std::srand(QDateTime::currentSecsSinceEpoch());
}

QrCodeDialog::~QrCodeDialog() {
	tcpServer_->close();
}

QrCodeDialog::Session::Session() {
	created_ = QDateTime::currentSecsSinceEpoch();
	bytes_.resize(SessionSize);
	for (int i = 0; i < SessionSize; i++)
		bytes_[i] = std::rand() % 10 + '0';
}

QString QrCodeDialog::Session::bytesToString() {
	QString res;
	for (int i = 0; i < SessionSize; i++)
		res += bytes_[i];
	return res;
}

void QrCodeDialog::SessionList::deleteOldSession() {
	time_t now = QDateTime::currentSecsSinceEpoch();
	int i = this->count();
	for (i = 0; i < count();) {
		const Session& session = at(i);
		time_t created = session.created_;
		if (created + qrCodeDialog_->sessionTimeOutSec_ < now) {
			this->removeAt(i);
		} else
			i++;
	}
}

#include "hawidget.h"
void QrCodeDialog::pbGenerate_clicked(bool checked) {
	(void)checked;

	Session session;
	qDebug() << session.bytesToString(); // gilgil temp 2024.01.10
	sessionList_.push_back(session);

	HaWidget* haWidget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	GIntf* intf = haWidget->hostAnalyzer_.pcapDevice_.intf();
	Q_ASSERT(intf != nullptr);
	GIp myIp = intf->ip();

	QString text = QString("http://%1:%2/%3").arg(QString(myIp)).arg(port_).arg(session.bytesToString());
	int size = std::min(lblQrCode_->width(), lblQrCode_->height());
	QImage image = generator_->generateQr(text, size);
	lblQrCode_->setPixmap(QPixmap::fromImage(image));
}

void QrCodeDialog::tcpServer_newConnection() {
	QTcpSocket *socket = tcpServer_->nextPendingConnection();
	QObject::connect(socket, SIGNAL(readyRead()), SLOT(tcpServer_ReadyRead()));
}

#include "hostanalyzer.h"
void QrCodeDialog::tcpServer_ReadyRead() {
	QTcpSocket* socket = dynamic_cast<QTcpSocket*>(sender());
	Q_ASSERT(socket != nullptr);

	QByteArray ba = socket->readAll();
	if (ba.startsWith("GET /favicon.ico")) {
		socket->close();
		return;
	}

	QByteArray bytes;
	int count = ba.count();
	for (int i = 4; i < count; i++) { // 4 is sizeof "GET "
		char c = ba.at(i);
		if (std::isdigit(c)) {
			bytes += c;
			continue;
		}
		if (c == ' ') break;
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
		qWarning() << QString("can not find session %1").arg(QString(bytes));
		socket->write("HTTP/1.1 200 OK\r\n\r\n<br><br><br><br><h1><center>Can not find session</center><h1>");
		socket->close();
		return;
	}

	GIp peerIp = socket->peerAddress().toIPv4Address();
	qDebug() << QString("peerIp = %1").arg(QString(peerIp));

	HaWidget* haWidget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	HostAnalyzer* hostAnalyzer = &haWidget->hostAnalyzer_;
	GTreeWidget* treeWidget = hostAnalyzer->treeWidget_;
	int count2 = treeWidget->topLevelItemCount();

	bool found = false;
	for (int i = 0; i < count2; i++) {
		GTreeWidgetItem* twi = dynamic_cast<GTreeWidgetItem*>(treeWidget->topLevelItem(i));
		Q_ASSERT(twi != nullptr);
		QToolButton* toolButton = dynamic_cast<QToolButton*>(treeWidget->itemWidget(twi, HostAnalyzer::ColumnAttack));
		Q_ASSERT(toolButton != nullptr);

		quintptr uip = toolButton->property("hostValue").toULongLong();
		Q_ASSERT(uip != 0);
		GHostMgr::HostValue* hostValue = GHostMgr::PHostValue(uip);
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
		socket->write("HTTP/1.1 200 OK\r\n\r\n<br><br><br><br><h1><center>Internet connection admitted</center><h1>");
	} else {
		qWarning() << QString("can not find %1").arg(QString(peerIp));
		socket->write("HTTP/1.1 503 Service Unavailable\r\n\r\n<br><br><br><br><h1><center>Can not find session</center><h1>");
	}
	socket->close();
}

void QrCodeDialog::propLoad(QJsonObject jo) {
	port_ = jo["port"].toInt(Port);
	sessionTimeOutSec_ = jo["sessionTimeOutSec"].toInt(Session::SessionSize);
	jo["rect"] >> GJson::rect(this);
}

void QrCodeDialog::propSave(QJsonObject& jo) {
	jo["port"] = port_;
	jo["sessionTimeOutSec"] = sessionTimeOutSec_;
	jo["rect"] << GJson::rect(this);
}
