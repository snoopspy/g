#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <GStateObj>
#include <GMac>

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
		qint64 mac_{0};
		QString alias_;
		QString host_;
		QString vendor_;
		bool isNull() { return mac_ == 0; };
	};
protected:
	QSqlQuery* selectDeviceQuery_;
	QSqlQuery* insertDeviceQuery_;

public:
	Device selectDevice(GMac mac);
	bool insertDevice(Device device);
};
