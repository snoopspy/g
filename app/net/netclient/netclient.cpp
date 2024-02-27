#include "widget.h"

#include <QApplication>
#include <GApp>

Q_DECLARE_METATYPE(QSsl::SslProtocol)

int main(int argc, char* argv[]) {
	GApp a(argc, argv, false, false);
	Widget w;
	w.show();
	return a.exec();
}
