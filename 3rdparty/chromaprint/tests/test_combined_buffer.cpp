#include <gtest/gtest.h>
#include <algorithm>
#include "combined_buffer.h"

using namespace std;
using namespace Chromaprint;

TEST(CombinedBuffer, Size) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	EXPECT_EQ(8, buffer.Size());
	buffer.Shift(1);
	EXPECT_EQ(7, buffer.Size());
}

TEST(CombinedBuffer, AccessElements) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(1 + i, buffer[i]);
	}
	buffer.Shift(1);
	for (int i = 0; i < 7; i++) {
		EXPECT_EQ(2 + i, buffer[i]);
	}
	buffer.Shift(5);
	for (int i = 0; i < 2; i++) {
		EXPECT_EQ(7 + i, buffer[i]);
	}
}

TEST(CombinedBuffer, AccessElementsViaIterator) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	CombinedBuffer<short>::Iterator iter = buffer.Begin();
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(1 + i, *iter);
		++iter;
	}
	EXPECT_TRUE(buffer.End() == iter);
}

TEST(CombinedBuffer, AccessElementsViaIteratorAfterShift) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	buffer.Shift(6);
	CombinedBuffer<short>::Iterator iter = buffer.Begin();
	for (int i = 0; i < 2; i++) {
		EXPECT_EQ(7 + i, *iter);
		++iter;
	}
	EXPECT_TRUE(buffer.End() == iter);
}

TEST(CombinedBuffer, CopyUsingStlAlgorithms) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	short tmp[10];
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	fill(tmp, tmp + 10, 0);
	for (int i = 0; i < 10; i++) {
		EXPECT_EQ(0, tmp[i]);
	}
	copy(buffer.Begin(), buffer.End(), tmp);
	for (int i = 0; i < 8; i++) {
		EXPECT_EQ(1 + i, tmp[i]);
	}
	for (int i = 8; i < 10; i++) {
		EXPECT_EQ(0, tmp[i]);
	}
}

TEST(CombinedBuffer, CopyUsingStlAlgorithmsAfterShift) {
	short buffer1[] = { 1, 2, 3, 4, 5 };
	short buffer2[] = { 6, 7, 8 };
	short tmp[10];
	CombinedBuffer<short> buffer(buffer1, 5, buffer2, 3);
	buffer.Shift(6);
	fill(tmp, tmp + 10, 0);
	for (int i = 0; i < 10; i++) {
		EXPECT_EQ(0, tmp[i]);
	}
	copy(buffer.Begin(), buffer.End(), tmp);
	for (int i = 0; i < 2; i++) {
		EXPECT_EQ(7 + i, tmp[i]);
	}
	for (int i = 2; i < 10; i++) {
		EXPECT_EQ(0, tmp[i]);
	}
}

