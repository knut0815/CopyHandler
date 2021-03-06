#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TWriteBufferQueueWrapper.h"
#include <memory>
#include "../../common/GTestMacros.h"

using namespace chengine;
using namespace chcore;

TEST(TWriteBufferQueueWrapperTests, ConstructorWithNullParam)
{
	EXPECT_THROW(TWriteBufferQueueWrapper(nullptr, 0, nullptr), TCoreException);
}

TEST(TWriteBufferQueueWrapperTests, Constructor)
{
	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TWriteBufferQueueWrapper queue(spQueue, 1, spOtfBufferCount);
	EXPECT_EQ(0UL, queue.GetCount());
	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
}

TEST(TWriteBufferQueueWrapperTests, Pop_EmptyQueue)
{
	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TWriteBufferQueueWrapper queue(spQueue, 1, spOtfBufferCount);

	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TWriteBufferQueueWrapperTests, Pop_FromBufferList)
{
	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(1000);
	buffer2.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer3(1024, nullptr);
	buffer3.SetFilePosition(2000);
	buffer3.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	buffer4.SetFilePosition(3000);
	buffer4.SetRequestedDataSize(1000);

	spQueue->Push(&buffer1);
	spQueue->Push(&buffer2);
	spQueue->Push(&buffer3);
	spQueue->Push(&buffer4);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TWriteBufferQueueWrapper queue(spQueue, 1, spOtfBufferCount);

	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(0, buffer1.GetFilePosition());
	EXPECT_EQ(1000UL, buffer1.GetRequestedDataSize());

	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(1000, buffer2.GetFilePosition());
	EXPECT_EQ(1000UL, buffer2.GetRequestedDataSize());

	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(2000, buffer3.GetFilePosition());
	EXPECT_EQ(1000UL, buffer3.GetRequestedDataSize());

	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(3000, buffer4.GetFilePosition());
	EXPECT_EQ(1000UL, buffer4.GetRequestedDataSize());

	EXPECT_EQ(nullptr, queue.Pop());
}

TEST(TWriteBufferQueueWrapperTests, PushPop_ClaimedBuffers)
{
	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TWriteBufferQueueWrapper queue(spQueue, 1, spOtfBufferCount);

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(1000);
	buffer2.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer3(1024, nullptr);
	buffer3.SetFilePosition(2000);
	buffer3.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	buffer4.SetFilePosition(3000);
	buffer4.SetRequestedDataSize(1000);

	queue.Push(&buffer4);
	queue.Push(&buffer3);
	queue.Push(&buffer2);
	queue.Push(&buffer1);

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(0, buffer1.GetFilePosition());
	EXPECT_EQ(1000UL, buffer1.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(1000, buffer2.GetFilePosition());
	EXPECT_EQ(1000UL, buffer2.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(2000, buffer3.GetFilePosition());
	EXPECT_EQ(1000UL, buffer3.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(3000, buffer4.GetFilePosition());
	EXPECT_EQ(1000UL, buffer4.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Pop());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
}

TEST(TWriteBufferQueueWrapperTests, PushPop_MixedBuffers)
{
	TBufferListPtr spEmptyBuffers(std::make_shared<TBufferList>());
	TOrderedBufferQueuePtr spQueue(std::make_shared<TOrderedBufferQueue>(spEmptyBuffers, 0));

	TOverlappedDataBuffer buffer1(1024, nullptr);
	buffer1.SetFilePosition(0);
	buffer1.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer2(1024, nullptr);
	buffer2.SetFilePosition(1000);
	buffer2.SetRequestedDataSize(1000);

	spQueue->Push(&buffer1);
	spQueue->Push(&buffer2);

	TSharedCountPtr<size_t> spOtfBufferCount(std::make_shared<TSharedCount<size_t>>());
	TWriteBufferQueueWrapper queue(spQueue, 1, spOtfBufferCount);

	TOverlappedDataBuffer buffer3(1024, nullptr);
	buffer3.SetFilePosition(2000);
	buffer3.SetRequestedDataSize(1000);
	TOverlappedDataBuffer buffer4(1024, nullptr);
	buffer4.SetFilePosition(3000);
	buffer4.SetRequestedDataSize(1000);
	queue.Push(&buffer3);
	queue.Push(&buffer4);

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer3, queue.Pop());
	EXPECT_EQ(2000, buffer3.GetFilePosition());
	EXPECT_EQ(1000UL, buffer3.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer4, queue.Pop());
	EXPECT_EQ(3000, buffer4.GetFilePosition());
	EXPECT_EQ(1000UL, buffer4.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer1, queue.Pop());
	EXPECT_EQ(0, buffer1.GetFilePosition());
	EXPECT_EQ(1000UL, buffer1.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(&buffer2, queue.Pop());
	EXPECT_EQ(1000, buffer2.GetFilePosition());
	EXPECT_EQ(1000UL, buffer2.GetRequestedDataSize());

	EXPECT_SIGNALED(queue.GetHasBuffersEvent());
	EXPECT_EQ(nullptr, queue.Pop());
	EXPECT_TIMEOUT(queue.GetHasBuffersEvent());
}
