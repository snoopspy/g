#include <QDebug>
#include <GErr>

using namespace std;

struct Obj {
	GErr& err() {
		return err_;
	}
private:
	GErr err_ { GErr::Fail, "NOT_SUPPORTED in Obj class" };
};

int main() {
	{
		GErr err;
		qDebug() << err << Qt::endl;
	}

	{
		GErr err{ GErr::NotSupported };
		qDebug() << err << Qt::endl;
	}

	{
		GErr err { GErr::NotSupported, "NOT_SUPPORTED" };
		qDebug() << err << Qt::endl;
	}

	{
		Obj obj;
		GErr& err = obj.err();
		qDebug() << err << Qt::endl;
	}
}
