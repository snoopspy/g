#include <GStateObj>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

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
};
