#ifndef EFS_DRIVERS_TIMER_TIMER_HPP
#define EFS_DRIVERS_TIMER_TIMER_HPP

#include "common/types.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include "system/clock/simulation_clock.hpp"
#include "system/scheduler/event_scheduler.hpp"
#include <cstdint>
#include <memory>

namespace efs::drivers::timer {

/// Hardware Timer peripheral modeling CTRL, COUNT, COMPARE, and STATUS MMIO registers.
class Timer {
public:
    static constexpr common::Address CTRL_OFFSET    = 0x00;
    static constexpr common::Address COUNT_OFFSET   = 0x04;
    static constexpr common::Address COMPARE_OFFSET = 0x08;
    static constexpr common::Address STATUS_OFFSET  = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT       = (1U << 0);
    static constexpr common::DWord CTRL_INT_ENABLE_BIT   = (1U << 1);
    static constexpr common::DWord CTRL_AUTO_RESET_BIT   = (1U << 2);
    static constexpr common::DWord STATUS_MATCH_BIT      = (1U << 0);

    Timer(mmio::MMIOBus& bus, common::Address baseAddress);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;

    /// Starts the hardware timer counter.
    void start();

    /// Stops the hardware timer counter.
    void stop();

    /// Returns true if the timer counter is running.
    [[nodiscard]] bool running() const noexcept;

    /// Sets the compare match value.
    void setCompare(common::DWord compareValue);

    /// Returns the compare match value.
    [[nodiscard]] common::DWord compare() const noexcept;

    /// Returns current counter value.
    [[nodiscard]] common::DWord counter() const noexcept;

    /// Resets counter value to zero.
    void reset();

    /// Returns true if compare match status bit is set.
    [[nodiscard]] bool hasMatch() const noexcept;

    /// Clears compare match status bit.
    void clearMatch();

    /// Enables or disables timer interrupts.
    void setInterruptEnabled(bool enable);

    /// Returns true if timer interrupt is enabled.
    [[nodiscard]] bool interruptEnabled() const noexcept;

    /// Enables or disables auto-reset of counter upon compare match.
    void setAutoReset(bool enable);

    /// Returns true if auto-reset upon compare match is enabled.
    [[nodiscard]] bool autoReset() const noexcept;

    /// Advances timer by one clock cycle.
    void tick();

    /// Attaches an InterruptController line to trigger on compare match.
    void attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId);

    /// Detaches the active InterruptController.
    void detachInterruptController() noexcept;

    /// Attaches SimulationClock reference for event scheduling.
    void attachClock(system::clock::SimulationClock* clock) noexcept;

    /// Detaches SimulationClock reference.
    void detachClock() noexcept;

    /// Returns attached SimulationClock or nullptr.
    [[nodiscard]] system::clock::SimulationClock* clock() const noexcept;

    /// Attaches EventScheduler reference for deterministic compare event scheduling.
    void attachScheduler(system::scheduler::EventScheduler* scheduler) noexcept;

    /// Detaches EventScheduler reference.
    void detachScheduler() noexcept;

    /// Returns attached EventScheduler or nullptr.
    [[nodiscard]] system::scheduler::EventScheduler* scheduler() const noexcept;

    /// Returns MMIO base address.
    [[nodiscard]] common::Address baseAddress() const noexcept;

    /// Returns address of CTRL register.
    [[nodiscard]] common::Address ctrlAddress() const noexcept;

    /// Returns address of COUNT register.
    [[nodiscard]] common::Address countAddress() const noexcept;

    /// Returns address of COMPARE register.
    [[nodiscard]] common::Address compareAddress() const noexcept;

    /// Returns address of STATUS register.
    [[nodiscard]] common::Address statusAddress() const noexcept;

private:
    void updateScheduledEvent();
    void cancelScheduledEvent();
    void handleCompareMatch();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_ctrlRegister;
    std::shared_ptr<mmio::Register> m_countRegister;
    std::shared_ptr<mmio::Register> m_compareRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;

    kernel::InterruptController* m_interruptController{nullptr};
    std::uint8_t m_interruptId{0};

    system::clock::SimulationClock* m_clock{nullptr};
    system::scheduler::EventScheduler* m_scheduler{nullptr};
    system::scheduler::EventId m_scheduledEventId{0};
    common::QWord m_lastClockCycles{0};
};

} // namespace efs::drivers::timer

#endif // EFS_DRIVERS_TIMER_TIMER_HPP
