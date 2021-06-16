#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ctime>

struct LPacket {
	typedef enum : int {
		Eof = -2, // read
		Fail = -1, // read write
		Timeout = 0, // read
		Ok = 1, // read write
	} Result;

	struct timeval ts_;
	uint8_t* buf_;
	size_t len_;

	void clear() {
		buf_ = nullptr;
		len_ = 0;
	}
};
