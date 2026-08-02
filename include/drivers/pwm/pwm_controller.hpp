#ifndef EFS_DRIVERS_PWM_PWM_CONTROLLER_HPP
#define EFS_DRIVERS_PWM_PWM_CONTROLLER_HPP

#include "common/types.hpp"
#include "drivers/timer/timer.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstdint>
#include <memory>

namespace efs::drivers::pwm {

/// Pulse Width Modulation (PWM) Controller peripheral generating signal duty cycles using an internal Timer.
class PWMController {
public:
    static constexpr common::Address CTRL_OFFSET   = 0x00;
    static constexpr common::Address FREQ_OFFSET   = 0x04;
    static constexpr common::Address DUTY_OFFSET   = 0x08;
    static constexpr common::Address STATUS_OFFSET = 0x0C;
    static constexpr common::Address TIMER_OFFSET  = 0x10;

    static constexpr common::DWord CTRL_ENABLE_BIT    = (1U << 0);
    static constexpr common::DWord STATUS_ENABLED_BIT = (1U << 0);
    static constexpr common::DWord STATUS_OUTPUT_BIT  = (1U << 1);

    explicit PWMController(mmio::MMIOBus& bus,
                           common::Address baseAddress,
                           std::uint32_t frequencyHz = 1000,
                           std::uint32_t dutyCyclePercent = 50,
                           std::uint32_t systemClockHz = 1000000);
    ~PWMController();

    PWMController(const PWMController&) = delete;
    PWMController& operator=(const PWMController&) = delete;
    PWMController(PWMController&&) = delete;
    PWMController& operator=(PWMController&&) = delete;

    /// Enables PWM output signal generation.
    void enable();

    /// Disables PWM output signal generation.
    void disable();

    /// Returns true if PWM output signal generation is enabled.
    [[nodiscard]] bool enabled() const noexcept;

    /// Configures PWM signal frequency in Hz. Throws std::invalid_argument if 0 or > systemClockHz.
    void setFrequency(std::uint32_t frequencyHz);

    /// Returns configured PWM signal frequency in Hz.
    [[nodiscard]] std::uint32_t frequency() const noexcept;

    /// Configures PWM signal duty cycle percentage (0 to 100). Throws std::invalid_argument if > 100.
    void setDutyCycle(std::uint32_t percent);

    /// Returns configured PWM signal duty cycle percentage.
    [[nodiscard]] std::uint32_t dutyCycle() const noexcept;

    /// Returns current PWM output pin state (true = HIGH, false = LOW).
    [[nodiscard]] bool outputState() const noexcept;

    /// Advances internal timer counter by one clock cycle.
    void tick();

    /// Resets registers, internal timer, and PWM signal state.
    void reset();

    /// Returns reference to internal Timer peripheral.
    [[nodiscard]] const timer::Timer& timer() const noexcept;
    [[nodiscard]] timer::Timer& timer() noexcept;

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address controlAddress() const noexcept;
    [[nodiscard]] common::Address freqAddress() const noexcept;
    [[nodiscard]] common::Address dutyAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;

private:
    void updatePeriodAndDuty();
    void updateStatusRegister();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;
    std::uint32_t m_systemClockHz{1000000};

    std::shared_ptr<mmio::Register> m_controlRegister;
    std::shared_ptr<mmio::Register> m_freqRegister;
    std::shared_ptr<mmio::Register> m_dutyRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;

    timer::Timer m_timer;
    common::DWord m_periodTicks{1000};
    common::DWord m_onTicks{500};
};

} // namespace efs::drivers::pwm

#endif // EFS_DRIVERS_PWM_PWM_CONTROLLER_HPP
