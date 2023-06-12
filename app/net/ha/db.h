#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <GStateObj>
#include <GMac>
#include <GIp>

// ----------------------------------------------------------------------------
// Db
// ----------------------------------------------------------------------------
struct G_EXPORT Db : GStateObj {
	Q_OBJECT
	Q_PROPERTY(QString fileName MEMBER fileName_)

public:
	QString fileName_{"ha.db"};

public:
	Q_INVOKABLE Db(QObject* parent = nullptr);
	~Db() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	QSqlDatabase db_;

public:
	struct Device {
		uint64_t mac_{0};
		uint32_t ip_{0};
		QString host_;
		QString vendor_;
		QString alias_;
		bool isNull() { return mac_ == 0; };
		QString defaultName() {
			if (alias_ != "") return alias_;
			if (host_ != "") return host_;
			if (vendor_ != "") return vendor_;
			return QString(GMac(mac_));
		}
	};

protected:
	QSqlQuery* selectDeviceQuery_;
	QSqlQuery* insertDeviceQuery_;
	QSqlQuery* updateDeviceQuery_;
	QSqlQuery* insertLogQuery_;

protected:
	Device selectDevice(GMac mac);
	bool insertDevice(Device device);
	bool updateDevice(Device device);

public:
	bool insertOrUpdateDevice(Device device);
	bool insertLog(GMac mac, GIp ip, time_t begTime, time_t endTime);
};
