#ifndef EFS_SYSTEM_POWER_POWER_CONTROLLER_HPP
#define EFS_SYSTEM_POWER_POWER_CONTROLLER_HPP

#include "system/power/power_state.hpp"

namespace efs::system::power {

/// System Power Controller managing power state transitions (ON, OFF, SLEEP).
class PowerController {
public:
    PowerController() = default;
    ~PowerController() = default;

    PowerController(const PowerController&) = delete;
    PowerController& operator=(const PowerController&) = delete;
    PowerController(PowerController&&) noexcept = default;
    PowerController& operator=(PowerController&&) noexcept = default;

    /// Powers on the system (transitions to ON state).
    void powerOn() noexcept;

    /// Powers off the system (transitions to OFF state).
    void powerOff() noexcept;

    /// Places the system in low-power sleep mode (transitions to SLEEP state).
    void sleep() noexcept;

    /// Wakes up the system from sleep mode (transitions to ON state).
    void wake() noexcept;

    /// Returns the current system power state.
    [[nodiscard]] PowerState state() const noexcept;

    /// Returns true if system power state is ON.
    [[nodiscard]] bool isPowerOn() const noexcept;

    /// Returns true if system power state is SLEEP.
    [[nodiscard]] bool isSleep() const noexcept;

    /// Returns true if system power state is OFF.
    [[nodiscard]] bool isPowerOff() const noexcept;

private:
    PowerState m_state{PowerState::ON};
};

} // namespace efs::system::power

#endif // EFS_SYSTEM_POWER_POWER_CONTROLLER_HPP
