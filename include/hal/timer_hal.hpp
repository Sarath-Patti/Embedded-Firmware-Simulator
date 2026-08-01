#ifndef EFS_HAL_TIMER_HAL_HPP
#define EFS_HAL_TIMER_HAL_HPP

#include "common/types.hpp"
#include "drivers/timer/timer.hpp"

namespace efs::hal {

/// Hardware Abstraction Layer for Hardware Timer peripherals.
class TimerHAL {
public:
    explicit TimerHAL(drivers::timer::Timer* timer = nullptr) noexcept;
    explicit TimerHAL(drivers::timer::Timer& timer) noexcept;
    ~TimerHAL() = default;

    TimerHAL(const TimerHAL&) = default;
    TimerHAL& operator=(const TimerHAL&) = default;
    TimerHAL(TimerHAL&&) noexcept = default;
    TimerHAL& operator=(TimerHAL&&) noexcept = default;

    /// Attaches or updates underlying Timer peripheral driver.
    void attachTimer(drivers::timer::Timer* timer) noexcept;

    /// Returns true if a Timer peripheral driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Starts the timer.
    void start();

    /// Stops the timer.
    void stop();

    /// Resets timer counter to 0.
    void reset();

    /// Returns current counter value.
    [[nodiscard]] common::DWord counter() const;

    /// Sets compare match register value.
    void setCompare(common::DWord value);

    /// Returns true if compare match flag is set.
    [[nodiscard]] bool hasMatch() const;

    /// Returns true if timer is currently running.
    [[nodiscard]] bool running() const;

private:
    drivers::timer::Timer* m_timer{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_TIMER_HAL_HPP
