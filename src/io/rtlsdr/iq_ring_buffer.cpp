#include "pdt/io/rtlsdr/iq_ring_buffer.h"

#include <stdexcept>

namespace pdt {

IqRingBuffer::IqRingBuffer(std::size_t capacity_frames)
    : capacity_(capacity_frames)
{
    if (capacity_ == 0) {
        throw std::invalid_argument("IqRingBuffer capacity must be > 0");
    }
}

bool IqRingBuffer::push(IqFrame frame)
{
    {
        std::scoped_lock lock(mutex_);

        if (stopped_) {
            return false;
        }

        if (frames_.size() >= capacity_) {
            frames_.pop_front();
            ++dropped_frames_;
        }

        frames_.push_back(std::move(frame));
    }

    cv_.notify_one();
    return true;
}

std::optional<IqFrame> IqRingBuffer::pop_latest()
{
    std::scoped_lock lock(mutex_);

    if (frames_.empty()) {
        return std::nullopt;
    }

    IqFrame latest = std::move(frames_.back());
    dropped_frames_ += frames_.size() - 1;
    frames_.clear();

    return latest;
}

std::optional<IqFrame> IqRingBuffer::wait_pop_latest()
{
    std::unique_lock lock(mutex_);

    cv_.wait(lock, [this] {
        return stopped_ || !frames_.empty();
    });

    if (frames_.empty()) {
        return std::nullopt;
    }

    IqFrame latest = std::move(frames_.back());
    dropped_frames_ += frames_.size() - 1;
    frames_.clear();

    return latest;
}

void IqRingBuffer::clear()
{
    std::scoped_lock lock(mutex_);
    frames_.clear();
}

void IqRingBuffer::stop()
{
    {
        std::scoped_lock lock(mutex_);
        stopped_ = true;
    }

    cv_.notify_all();
}

std::size_t IqRingBuffer::size() const
{
    std::scoped_lock lock(mutex_);
    return frames_.size();
}

std::size_t IqRingBuffer::capacity() const noexcept
{
    return capacity_;
}

std::uint64_t IqRingBuffer::dropped_frames() const noexcept
{
    return dropped_frames_;
}

} // namespace pdt
