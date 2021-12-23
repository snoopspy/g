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

struct GErrCategory {
	enum {
		Base = 10000,
		Net = 20000,
		Pcap = 2100,
		Http = 2200,

		End = 99999
	};
};
