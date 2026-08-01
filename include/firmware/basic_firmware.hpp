#ifndef EFS_FIRMWARE_BASIC_FIRMWARE_HPP
#define EFS_FIRMWARE_BASIC_FIRMWARE_HPP

#include "firmware/firmware.hpp"
#include "hal/gpio_hal.hpp"
#include <cstdint>

namespace efs::drivers::gpio {
class GPIO;
}

namespace efs::firmware {

/// Sample firmware toggling a GPIO pin using GPIOHAL abstraction.
class BasicFirmware : public Firmware {
public:
    BasicFirmware(hal::GPIOHAL& gpioHAL, std::uint8_t pin, common::Size toggleInterval = 1);
    BasicFirmware(drivers::gpio::GPIO& gpio, std::uint8_t pin, common::Size toggleInterval = 1);
    ~BasicFirmware() override;

    void initialize() override;
    void execute() override;
    void shutdown() override;

    [[nodiscard]] common::Size executionCount() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isShutdown() const noexcept;

private:
    hal::GPIOHAL m_gpioHAL;
    std::uint8_t m_pin;
    common::Size m_toggleInterval;
    common::Size m_cycleCounter{0};
    common::Size m_executionCount{0};
    bool m_initialized{false};
    bool m_shutdown{false};
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_BASIC_FIRMWARE_HPP
