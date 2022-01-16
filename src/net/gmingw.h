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

#include "gnet.h"

#ifdef Q_OS_WIN

typedef int socklen_t;

char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
int inet_pton(int at, const char* src, void* dst);

#endif
