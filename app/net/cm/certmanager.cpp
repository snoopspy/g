#include "certmanager.h"
#include <QSslCertificate>
#include <QSslError>

CertManager::CertManager(QObject* parent) : GGraph(parent) {
	pcapDevice_.setObjectName("pcapDevice_");
	tcpFlowMgr_.setObjectName("tcpFlowMgr_");
	certMgr_.setObjectName("certMgr_");
	pcapFileWrite_.setObjectName("pcapFileWrite_");
	command_.setObjectName("command_");

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = -1;
	pcapDevice_.waitTimeout_ = 1000;
#endif // Q_OS_LINUX
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = 1000;
	pcapDevice_.waitTimeout_ = 1;
#endif

	nodes_.append(&pcapDevice_);
	nodes_.append(&tcpFlowMgr_);
	nodes_.append(&certMgr_);
	nodes_.append(&pcapFileWrite_);

	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&tcpFlowMgr_, &GTcpFlowMgr::managed, &certMgr_, &GCertMgr::manage, Qt::DirectConnection);
	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &pcapFileWrite_, &GPcapFileWrite::write, Qt::DirectConnection);
	QObject::connect(&certMgr_, &GCertMgr::certificatesDetected, this, &CertManager::doCertificatesDetected, Qt::DirectConnection);
}

CertManager::~CertManager() {
	close();
}

bool CertManager::doOpen() {
	if (saveCertFileType_ != None) {
		if (!GCertMgr::makeFolder(saveCertFileFolder_)) {
			SET_ERR(GErr::Fail, QString("can not create folder(%1)").arg(saveCertFileFolder_));
			return false;
		}
	}

	certMgr_.tcpFlowMgr_ = &tcpFlowMgr_;
	bool res = GGraph::doOpen();
	if (!res) return false;

	return true;
}

bool CertManager::doClose() {
	bool res = GGraph::doClose();
	return res;
}

void CertManager::doCertificatesDetected(QString serverName, struct timeval ts, QList<QByteArray> certs) {
	QList<QSslCertificate> chain;
	int count = certs.size();
	for (int i = 0; i < count; i++) {
		QByteArray cert = certs.at(i);
		QSslCertificate certificate(cert, QSsl::Der);
		qDebug() << i << certificate.subjectDisplayName();
		chain.append(certificate);
	}
	QList<QSslError> errors = QSslCertificate::verify(chain);
	bool ok = errors.size() == 0;

	if (showType_ == All || (showType_ == Abnormal && !ok)) {
		QMetaObject::invokeMethod(this, [this, serverName, errors, certs]() {
			GTreeWidgetItem* twi = new GTreeWidgetItem(treeWidget_);
			twi->setText(ColumnName, serverName);
			QStringList helps;
			for (const QSslError& error: errors)
				helps.append(error.errorString());
			twi->setProperty("help", helps.join("\n"));

			for (const QByteArray& cert: certs) {
				QSslCertificate certificate(cert, QSsl::Der);
				QString displayName = certificate.subjectDisplayName();

				GTreeWidgetItem* twi2 = new GTreeWidgetItem(twi);
				twi2->setText(ColumnName, displayName);

				helps.clear();
				helps.append(certificate.toText());
				twi2->setProperty("help", helps.join("\n"));
			}
			treeWidget_->addTopLevelItem(twi);
		}, Qt::BlockingQueuedConnection);
	}

	if (saveCertFileType_ == All || (saveCertFileType_ == Abnormal && !ok)) {
		certMgr_.saveCertFiles(saveCertFileFolder_, serverName, ts, certs);
	}
	qDebug() << "chain size=" << chain.size();
	qDebug() << errors;
}

#include "cmwidget.h"
void CertManager::processClosed() {
	qDebug() << "bef call close()"; // gilgil temp 2023.10.18
	CmWidget* haWidget = dynamic_cast<CmWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	haWidget->tbStop_->click();
	qDebug() << "aft call close()"; // gilgil temp 2023.10.18
}

void CertManager::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void CertManager::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
