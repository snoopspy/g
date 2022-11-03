// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include <cstddef>

extern int _debug_gilgil;
extern __thread int _thread_gilgil;

typedef void *pvoid;
typedef char *pchar;
typedef unsigned char gbyte, *pbyte;

#ifdef WIN32
	#if defined(G_BUILD)
		// #define G_EXPORT __declspec(dllexport) --- for build dll
		#define G_EXPORT
	#else
		// #define G_EXPORT __declspec(dllimport) --- for build dll
		#define G_EXPORT
#endif
#endif
#ifdef linux
	#define G_EXPORT
#endif

typedef unsigned long GDuration;

struct G_EXPORT G {
	static constexpr GDuration Timeout = 5000;
	static const char* version();
	static const char* pcapLibVersion();
};

#define GDEBUG_CTOR { qDebug() << "constructor  " << pvoid(this); }
#define GDEBUG_DTOR { qDebug() << "destructor " << pvoid(this); }
