#ifndef EFS_DRIVERS_TIMER_TIMER_HPP
#define EFS_DRIVERS_TIMER_TIMER_HPP

#include "common/types.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <memory>

namespace efs::kernel {
class InterruptController;
}

namespace efs::system::clock {
class SimulationClock;
}

namespace efs::drivers::timer {

class Timer {
public:
    static constexpr common::Address CTRL_OFFSET    = 0x00;
    static constexpr common::Address COUNT_OFFSET   = 0x04;
    static constexpr common::Address COMPARE_OFFSET = 0x08;
    static constexpr common::Address STATUS_OFFSET  = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT  = 0x1;
    static constexpr common::DWord STATUS_MATCH_BIT = 0x1;

    Timer(mmio::MMIOBus& bus, common::Address baseAddress);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;

    void start();
    void stop();
    void reset();
    void tick();

    void attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId);
    void detachInterruptController() noexcept;

    void attachClock(system::clock::SimulationClock* clock) noexcept;
    void detachClock() noexcept;
    [[nodiscard]] system::clock::SimulationClock* clock() const noexcept;

    void setCompare(common::DWord value);
    [[nodiscard]] common::DWord compare() const noexcept;
    [[nodiscard]] common::DWord counter() const noexcept;
    [[nodiscard]] bool running() const noexcept;
    [[nodiscard]] bool hasMatch() const noexcept;

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address ctrlAddress() const noexcept;
    [[nodiscard]] common::Address countAddress() const noexcept;
    [[nodiscard]] common::Address compareAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;

private:
    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_ctrlRegister;
    std::shared_ptr<mmio::Register> m_countRegister;
    std::shared_ptr<mmio::Register> m_compareRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;

    kernel::InterruptController* m_interruptController{nullptr};
    std::uint8_t m_interruptId{0};

    system::clock::SimulationClock* m_clock{nullptr};
    common::QWord m_lastClockCycles{0};
};

} // namespace efs::drivers::timer

#endif // EFS_DRIVERS_TIMER_TIMER_HPP
