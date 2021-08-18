#pragma once

#include "gpdu.h"

#pragma pack(push, 1)
struct RadiotapHdr {
	le8_t ver_;
	le8_t pad_;
	le16_t len_;
	le32_t present_;

	bool isShortPreamble() { return len_ == 13; }
	static RadiotapHdr* check(char* p, uint32_t size);

	// ----- gilgil temp 2020.11.07 -----
	/*
	struct LenghChecker {
		le16_t real_{0};
		le16_t send_{sizeof(RadiotapHdr)};
		le16_t ignore_{0};
		bool checked_{false};
		bool check(std::string interface);
	};	*/
	// -----------------------------------
};
typedef RadiotapHdr *PRadiotapHdr;
#pragma pack(pop)
