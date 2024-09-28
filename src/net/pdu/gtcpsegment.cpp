#include "gtcpsegment.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GTcpSegment
// ----------------------------------------------------------------------------
GTcpSegment::GTcpSegment(uint32_t firstSeq) : firstSeq_(firstSeq), nextSeq_(firstSeq) {
}

QByteArray GTcpSegment::insert(uint32_t seq, QByteArray segment) {
	map_.insert(seq, segment);
	return reassemble();
}

QByteArray GTcpSegment::reassemble() {
	for (auto it = map_.begin(); it != map_.end();) {
		uint32_t seq = it.key();
		QByteArray& segment = it.value();
		bool ok = true;
		if (nextSeq_ < 0x0000FFFF && seq > 0xFFFF0000) // maybe overflow
			ok = true;
		else if (seq < 0x0000FFFF && nextSeq_ > 0xFFFF0000) // maybe overflow
			ok = false;
		else if (nextSeq_ < seq)
			ok = false;
		if (!ok) break;
		segment = segment.mid(nextSeq_ - seq);
		*this += segment;
		nextSeq_ += segment.size();
		it = map_.erase(it);
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
		EXPECT_TRUE(ba == "AAAAA");

		seq += 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBB");

		seq += 5;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_TRUE(ba == "AAAAABBBBBCCCCC");
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
		EXPECT_TRUE(ba == "AAAAA");
		seq += 5;

		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBB");

		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBB");

		seq += 5;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_TRUE(ba == "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//           CCCCC
	//      BBBBB
	void reverse(uint32_t seq) {
		printf("reverse %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_TRUE(ba == "AAAAA");

		seq += 10;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_TRUE(ba == "AAAAA");

		seq -= 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBBCCCCC");
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
		EXPECT_TRUE(ba == "AAAAA");

		seq += 10;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_TRUE(ba == "AAAAA");

		seq -= 5;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBBCCCCC");

		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBBBBCCCCC");
	}

	// AAAAA
	//    BBBBB
	//       CCCCC
	void overlap(uint32_t seq) {
		printf("overlap %d %u %08x\n", seq, seq, seq);
		GTcpSegment ts(seq);
		QByteArray ba;

		ba = ts.insert(seq, "AAAAA");
		EXPECT_TRUE(ba == "AAAAA");

		seq += 3;
		ba = ts.insert(seq, "BBBBB");
		EXPECT_TRUE(ba == "AAAAABBB");

		seq += 3;
		ba = ts.insert(seq, "CCCCC");
		EXPECT_TRUE(ba == "AAAAABBBCCC");
	}
};

TEST_F(GTcpSegmentTest, basicTest) {
	basic(1);
	basic(uint32_t(-1));
	basic(uint32_t(-4));
	basic(uint32_t(-6));
	basic(0x80000000 - 1);
	basic(0x80000000 - 4);
	basic(0x80000000 - 6);
}

TEST_F(GTcpSegmentTest, duplicateTest) {
	duplicate(1);
	duplicate(uint32_t(-1));
	duplicate(uint32_t(-4));
	duplicate(uint32_t(-6));
	duplicate(0x80000000 - 1);
	duplicate(0x80000000 - 4);
	duplicate(0x80000000 - 6);
}

TEST_F(GTcpSegmentTest, reverseTest) {
	reverse(1);
	reverse(uint32_t(-1));
	reverse(uint32_t(-4));
	reverse(uint32_t(-6));
	reverse(0x80000000 - 1);
	reverse(0x80000000 - 4);
	reverse(0x80000000 - 6);
}

TEST_F(GTcpSegmentTest, reverseDuplicateTest) {
	reverseDuplicate(1);
	reverseDuplicate(uint32_t(-1));
	reverseDuplicate(uint32_t(-4));
	reverseDuplicate(uint32_t(-6));
	reverseDuplicate(0x80000000 - 1);
	reverseDuplicate(0x80000000 - 4);
	reverseDuplicate(0x80000000 - 6);
}

TEST_F(GTcpSegmentTest, overlapTest) {
	overlap(1);
	overlap(uint32_t(-1));
	overlap(uint32_t(-4));
	overlap(uint32_t(-6));
	overlap(uint32_t(-1));
	overlap(uint32_t(-4));
	overlap(uint32_t(-6));
}

#endif // GTEST
