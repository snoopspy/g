#include "gnexmon.h"
#include <QFile>

// ----------------------------------------------------------------------------
// GNexmon
// ----------------------------------------------------------------------------
QString GNexmon::preloadFileName() {
	if (QFile::exists("/system/lib/libfakeioctl.so") || QFile::exists("/system/lib64/libfakeioctl.so"))
		return "libfakeioctl.so";
	if (QFile::exists("/system/lib/libnexmon.so") || QFile::exists("/system/lib64/libnexmon.so"))
		return "libnexmon.so";
	return "";
}
