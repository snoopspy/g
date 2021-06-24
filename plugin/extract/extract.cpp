#include "ghttpextract.h"

#ifdef Q_OS_WIN
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" {

EXPORT int count() {
	qRegisterMetaType<GHttpExtract*>();
	return 1;
}

EXPORT const char* name(int index) {
	switch (index) {
		case 0: return GHttpExtract::staticMetaObject.className();
		default: return nullptr;
	}
}

EXPORT void* create(int index) {
	switch (index) {
		case 0: return new GHttpExtract;
		default: return nullptr;
	}
}

}
