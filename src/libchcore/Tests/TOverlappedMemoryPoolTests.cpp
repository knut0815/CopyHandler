#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TOverlappedMemoryPool.h"
#include "../../liblogger/TLogFileData.h"
#include "../TOverlappedDataBuffer.h"
#include "../TCoreException.h"

using namespace chcore;

TEST(TOverlappedReaderTests, AllocatingConstructor_CheckBufferSizes)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	TBufferListPtr spBufferList = spBuffers->GetBufferList();

	EXPECT_EQ(3, spBufferList->GetCount());

	EXPECT_EQ(32768, spBufferList->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBufferList->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBufferList->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBufferList->Pop());
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_FailsWithBuffersInUse)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	spBuffers->GetBufferList()->Pop();

	EXPECT_THROW(spBuffers->ReinitializeBuffers(3, 65536), TCoreException);
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_ZeroLengthBuffers)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));

	EXPECT_THROW(spBuffers->ReinitializeBuffers(3, 0), TCoreException);
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_SameSizeSameCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	spBuffers->ReinitializeBuffers(3, 32768);

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBuffers->GetBufferList()->Pop());
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_IncreaseSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	spBuffers->ReinitializeBuffers(3, 65536);

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(65536, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(65536, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(65536, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(65536, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBuffers->GetBufferList()->Pop());
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_DecreaseSize)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 65536));
	spBuffers->ReinitializeBuffers(3, 32768);

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBuffers->GetBufferList()->Pop());
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_IncreaseCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(3, 32768));
	spBuffers->ReinitializeBuffers(5, 32768);

	EXPECT_EQ(5, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBuffers->GetBufferList()->Pop());
}

TEST(TOverlappedReaderTests, ReinitializeBuffer_DecreaseCount)
{
	logger::TLogFileDataPtr spLogData(std::make_shared<logger::TLogFileData>());

	TOverlappedMemoryPoolPtr spBuffers(std::make_shared<TOverlappedMemoryPool>(5, 32768));
	spBuffers->ReinitializeBuffers(3, 32768);

	EXPECT_EQ(3, spBuffers->GetTotalBufferCount());
	EXPECT_EQ(32768, spBuffers->GetSingleBufferSize());

	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(32768, spBuffers->GetBufferList()->Pop()->GetBufferSize());
	EXPECT_EQ(nullptr, spBuffers->GetBufferList()->Pop());
}
