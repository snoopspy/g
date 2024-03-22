#include "widget.h"

#include <QApplication>
#include <GApp>

int main(int argc, char* argv[]) {
	GApp a(argc, argv, false, false);
	Widget w;
	w.show();
	return a.exec();
}
