#pragma once

#include <GGraph>
#include <GPcapDevice>
#include <GTcpFlowMgr>
#include <GCertMgr>
#include <GPcapFileWrite>
#include <GCommand>
#include <GTreeWidget>

struct G_EXPORT CertManager : GGraph {
	Q_OBJECT
	Q_PROPERTY(Type showType MEMBER showType_)
	Q_PROPERTY(Type saveCertFileType MEMBER saveCertFileType_)
	Q_PROPERTY(QString saveCertFileFolder MEMBER saveCertFileFolder_)
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef tcpFlowMgr READ getTcpFlowMgr)
	Q_PROPERTY(GObjRef certMgr READ getCertMgr)
	Q_PROPERTY(GObjRef pcapFileWrite READ getPcapFileWrite)
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_ENUMS(Type)

public:
	enum Type {
		Abnormal,
		All,
		None
	};

public:
	Type showType_{All};
	Type saveCertFileType_{Abnormal};
	QString saveCertFileFolder_{QString("certificate") + QDir::separator()};

	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getTcpFlowMgr() { return &tcpFlowMgr_; }
	GObjRef getCertMgr() { return &certMgr_; }
	GObjRef getPcapFileWrite() { return &pcapFileWrite_; }
	GObjRef getCommand() { return &command_; }

public:
	Q_INVOKABLE CertManager(QObject* parent = nullptr);
	~CertManager() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPcapDevice pcapDevice_{this};
	GTcpFlowMgr tcpFlowMgr_{this};
	GCertMgr certMgr_{this};
	GPcapFileWrite pcapFileWrite_{this};
	GCommand command_{this};

public:
	GTreeWidget* treeWidget_{nullptr}; // reference

public:
	const static int ColumnName = 0;

public slots:
	void doCertificatesDetected(QString serverName, struct timeval ts, QList<QByteArray> certs);
	void processClosed();

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
