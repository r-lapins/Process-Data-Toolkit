#pragma once

#include "pdt/io/rtlsdr/iq_frame.h"

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>

namespace pdt {

class IqRingBuffer {
  public:
    explicit IqRingBuffer(std::size_t capacity_frames);

    bool push(IqFrame frame);

    std::optional<IqFrame> pop_latest();
    std::optional<IqFrame> wait_pop_latest();

    void clear();
    void stop();

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] std::size_t capacity() const noexcept;
    [[nodiscard]] std::uint64_t dropped_frames() const noexcept;

  private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;

    std::deque<IqFrame> frames_;
    std::size_t capacity_{};
    std::uint64_t dropped_frames_{};
    bool stopped_{false};
};

} // namespace pdt
