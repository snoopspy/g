#include "gtcpsegment.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GTcpSegment
// ----------------------------------------------------------------------------
GTcpSegment::GTcpSegment(uint32_t firstSeq) : firstSeq_(firstSeq), nextSeq_(firstSeq) {
}

QByteArray GTcpSegment::insert(uint32_t seq, QByteArray segment) {
	map_.insert({seq, segment});
	set_.insert(int32_t(seq));
	return reassemble();
}

#define DELTA 65536
QByteArray GTcpSegment::reassemble() {
	for (Map::iterator it = map_.begin(); it != map_.end();) {
		uint32_t seq = it->first;

		// printf("unsigned %u %u\n", nextSeq_, seq); // by gilgil 2024.09.28
		bool ok;
		if (nextSeq_ >= seq) {
			ok = true;
			if (seq < DELTA && nextSeq_ > UINT32_MAX - DELTA) // | seq <-------> nextSeq |
				ok = false;
		} else {
			ok = false;
			if (nextSeq_ < DELTA && seq > UINT32_MAX - DELTA) // | nextSeq <-------> seq |
				ok = true;
		}
		if (!ok) break;

		QByteArray segment = it->second;
		int32_t skip = nextSeq_ - seq;
		Q_ASSERT(skip >= 0);
		segment = segment.mid(skip);
		*this += segment;

		nextSeq_ += segment.size();
		it = map_.erase(it);
		set_.erase(int32_t(seq));
	}

	for (Set::iterator it = set_.begin(); it != set_.end();) {
		int32_t seq = int32_t(*it);

		// printf("signed %d %d\n", nextSeq_, seq); // by gilgil 2024.09.28
		bool ok;
		if (int32_t(nextSeq_) >= seq) {
			ok = true;
			if (int32_t(nextSeq_) > INT32_MAX - DELTA && seq < INT32_MIN + DELTA) // nextSeq INT32_MAX seq
				ok = false;
		} else {
			ok = false;
			if (seq > INT32_MAX - DELTA && int32_t(nextSeq_) < INT32_MIN + DELTA) // seq INT32_MAX nextSeq
				ok = true;
		}
		if (!ok) break;

		Map::iterator mapIt = map_.find(uint32_t(seq));
		Q_ASSERT(mapIt != map_.end());

		QByteArray& segment = mapIt->second;
		int32_t skip = nextSeq_ - seq;
		Q_ASSERT(skip >= 0);
		segment = segment.mid(skip);
		*this += segment;

		nextSeq_ += segment.size();
		map_.erase(mapIt);
		it = set_.erase(it);
	}

	return *this;
}

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

struct GTcpSegmentTest : testing::Test {

	// AAAAA
	//      BBBBB
	//           CCCCC
	void basic(uint32_t seq) {
		printf("basic %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_EQ(ba, "AAAAA");

		seq += 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBB");

		seq += 5;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_EQ(ba, "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//      BBBBB
	//      BBBBB
	//           CCCCC
	void duplicate(uint32_t seq) {
		printf("duplicate %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_EQ(ba, "AAAAA");
		seq += 5;

		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBB");

		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBB");

		seq += 5;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_EQ(ba, "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//           CCCCC
	//      BBBBB
	void reverse(uint32_t seq) {
		printf("reverse %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_EQ(ba, "AAAAA");

		seq += 10;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_EQ(ba, "AAAAA");

		seq -= 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//           CCCCC
	//      BBBBB
	//      BBBBB
	void reverseDuplicate(uint32_t seq) {
		printf("reverseDuplicate %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_EQ(ba, "AAAAA");

		seq += 10;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_EQ(ba, "AAAAA");

		seq -= 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBBCCCCC");

		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//    BBBBB
	//       CCCCC
	void overlap(uint32_t seq) {
		printf("overlap %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_EQ(ba, "AAAAA");

		seq += 3;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_EQ(ba, "AAAAABBB");

		seq += 3;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_EQ(ba, "AAAAABBBCCC");
	}
};

TEST_F(GTcpSegmentTest, basicTest) {
	basic(1);
	basic(uint32_t(-1));
	basic(uint32_t(-4));
	basic(uint32_t(-6));
	basic(INT32_MAX - 1);
	basic(INT32_MAX - 4);
	basic(INT32_MAX - 6);
}

TEST_F(GTcpSegmentTest, duplicateTest) {
	duplicate(1);
	duplicate(uint32_t(-1));
	duplicate(uint32_t(-4));
	duplicate(uint32_t(-6));
	duplicate(INT32_MAX - 1);
	duplicate(INT32_MAX - 4);
	duplicate(INT32_MAX - 6);
}

TEST_F(GTcpSegmentTest, reverseTest) {
	reverse(1);
	reverse(uint32_t(-1));
	reverse(uint32_t(-4));
	reverse(uint32_t(-6));
	reverse(INT32_MAX - 1);
	reverse(INT32_MAX - 4);
	reverse(INT32_MAX - 6);
}

TEST_F(GTcpSegmentTest, reverseDuplicateTest) {
	reverseDuplicate(1);
	reverseDuplicate(uint32_t(-1));
	reverseDuplicate(uint32_t(-4));
	reverseDuplicate(uint32_t(-6));
	reverseDuplicate(INT32_MAX - 1);
	reverseDuplicate(INT32_MAX - 4);
	reverseDuplicate(INT32_MAX - 6);
}

TEST_F(GTcpSegmentTest, overlapTest) {
	overlap(1);
	overlap(uint32_t(-1));
	overlap(uint32_t(-4));
	overlap(uint32_t(-6));
	overlap(INT32_MAX - 1);
	overlap(INT32_MAX - 4);
	overlap(INT32_MAX - 6);
}

#endif // GTEST
