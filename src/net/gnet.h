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

#include <QtCore>
#include "base/gbase.h"

typedef uint8_t le8_t;
typedef uint16_t le16_t;
typedef uint32_t le32_t;
typedef uint64_t le64_t;

#ifdef Q_OS_LINUX
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
#endif
#ifdef Q_OS_WIN
	#ifndef NOMINMAX
	#define NOMINMAX
	#endif // NOMINMAX
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>
#endif
