#include "pdt/io/rtlsdr/iq_ring_buffer.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <thread>

namespace {

pdt::IqFrame make_frame(std::uint64_t sequence)
{
    pdt::IqFrame frame;
    frame.sequence = sequence;
    frame.sample_rate = 1'024'000;
    frame.samples = {{static_cast<float>(sequence), -static_cast<float>(sequence)}};
    return frame;
}

} // namespace

TEST(IqRingBufferTest, RejectsZeroCapacity)
{
    EXPECT_THROW(pdt::IqRingBuffer buffer(0), std::invalid_argument);
}

TEST(IqRingBufferTest, PopLatestReturnsNewestAndDropsOlderFrames)
{
    pdt::IqRingBuffer buffer(3);

    EXPECT_TRUE(buffer.push(make_frame(1)));
    EXPECT_TRUE(buffer.push(make_frame(2)));
    EXPECT_TRUE(buffer.push(make_frame(3)));

    auto frame = buffer.pop_latest();

    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->sequence, 3);
    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_EQ(buffer.dropped_frames(), 2U);
}

TEST(IqRingBufferTest, PushDropsOldestWhenCapacityIsReached)
{
    pdt::IqRingBuffer buffer(2);

    EXPECT_TRUE(buffer.push(make_frame(1)));
    EXPECT_TRUE(buffer.push(make_frame(2)));
    EXPECT_TRUE(buffer.push(make_frame(3)));

    EXPECT_EQ(buffer.size(), 2U);
    EXPECT_EQ(buffer.dropped_frames(), 1U);

    auto frame = buffer.pop_latest();

    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->sequence, 3);
    EXPECT_EQ(buffer.dropped_frames(), 2U);
}

TEST(IqRingBufferTest, WaitPopLatestReturnsWhenFrameArrives)
{
    pdt::IqRingBuffer buffer(2);

    std::thread producer([&buffer] {
        buffer.push(make_frame(7));
    });

    auto frame = buffer.wait_pop_latest();
    producer.join();

    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->sequence, 7);
}

TEST(IqRingBufferTest, StopWakesWaitersAndRejectsPush)
{
    pdt::IqRingBuffer buffer(2);

    buffer.stop();

    EXPECT_FALSE(buffer.push(make_frame(1)));
    EXPECT_FALSE(buffer.wait_pop_latest().has_value());
}
