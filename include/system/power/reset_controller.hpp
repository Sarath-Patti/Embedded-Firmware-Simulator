#ifndef EFS_SYSTEM_POWER_RESET_CONTROLLER_HPP
#define EFS_SYSTEM_POWER_RESET_CONTROLLER_HPP

#include "cpu/cpu.hpp"
#include "firmware/firmware_manager.hpp"
#include "system/system_bus.hpp"

namespace efs::system::power {

/// Reset Controller managing CPU, peripheral, firmware, and system-wide reset triggers.
class ResetController {
public:
    explicit ResetController(cpu::CPU* cpu = nullptr,
                            SystemBus* systemBus = nullptr,
                            firmware::FirmwareManager* firmwareManager = nullptr) noexcept;
    ~ResetController() = default;

    ResetController(const ResetController&) = delete;
    ResetController& operator=(const ResetController&) = delete;
    ResetController(ResetController&&) noexcept = default;
    ResetController& operator=(ResetController&&) noexcept = default;

    /// Attaches or updates CPU reference.
    void setCPU(cpu::CPU* cpu) noexcept;

    /// Attaches or updates SystemBus reference.
    void setSystemBus(SystemBus* systemBus) noexcept;

    /// Attaches or updates FirmwareManager reference.
    void setFirmwareManager(firmware::FirmwareManager* firmwareManager) noexcept;

    /// Resets CPU registers and cycle counter. Peripherals and firmware retain state.
    void resetCPU();

    /// Resets peripheral states (GPIO, Timer, UART, InterruptController). CPU and firmware retain state.
    void resetPeripherals();

    /// Resets active firmware application and restarts lifecycle. CPU and peripherals retain state.
    void resetFirmware();

    /// Performs full system reset (CPU, Peripherals, Firmware, Memory, Clock).
    void resetSystem();

private:
    cpu::CPU* m_cpu{nullptr};
    SystemBus* m_systemBus{nullptr};
    firmware::FirmwareManager* m_firmwareManager{nullptr};
};

} // namespace efs::system::power

#endif // EFS_SYSTEM_POWER_RESET_CONTROLLER_HPP
