#ifndef EFS_HAL_PWM_HAL_HPP
#define EFS_HAL_PWM_HAL_HPP

#include "drivers/pwm/pwm_controller.hpp"
#include <cstdint>

namespace efs::hal {

/// Hardware Abstraction Layer for Pulse Width Modulation (PWM) signal generation.
class PWMHAL {
public:
    explicit PWMHAL(drivers::pwm::PWMController* controller = nullptr) noexcept;
    explicit PWMHAL(drivers::pwm::PWMController& controller) noexcept;
    ~PWMHAL() = default;

    PWMHAL(const PWMHAL&) = default;
    PWMHAL& operator=(const PWMHAL&) = default;
    PWMHAL(PWMHAL&&) noexcept = default;
    PWMHAL& operator=(PWMHAL&&) noexcept = default;

    /// Attaches or updates underlying PWM controller driver.
    void attachPWM(drivers::pwm::PWMController* controller) noexcept;

    /// Returns true if a PWM controller driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Enables PWM output.
    void enable();

    /// Disables PWM output.
    void disable();

    /// Returns true if PWM is enabled.
    [[nodiscard]] bool enabled() const;

    /// Configures PWM duty cycle percentage (0 to 100).
    void setDutyCycle(std::uint32_t percent);

    /// Returns current PWM duty cycle percentage.
    [[nodiscard]] std::uint32_t dutyCycle() const;

    /// Configures PWM signal frequency in Hz.
    void setFrequency(std::uint32_t hz);

    /// Returns current PWM signal frequency in Hz.
    [[nodiscard]] std::uint32_t frequency() const;

    /// Returns current PWM pin output state (true = HIGH, false = LOW).
    [[nodiscard]] bool outputState() const;

private:
    drivers::pwm::PWMController* m_controller{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_PWM_HAL_HPP
