#ifndef EFS_HAL_GPIO_HAL_HPP
#define EFS_HAL_GPIO_HAL_HPP

#include "drivers/gpio/gpio.hpp"
#include <cstdint>

namespace efs::hal {

/// Hardware Abstraction Layer for GPIO peripherals.
class GPIOHAL {
public:
    explicit GPIOHAL(drivers::gpio::GPIO* gpio = nullptr) noexcept;
    explicit GPIOHAL(drivers::gpio::GPIO& gpio) noexcept;
    ~GPIOHAL() = default;

    GPIOHAL(const GPIOHAL&) = default;
    GPIOHAL& operator=(const GPIOHAL&) = default;
    GPIOHAL(GPIOHAL&&) noexcept = default;
    GPIOHAL& operator=(GPIOHAL&&) noexcept = default;

    /// Attaches or updates underlying GPIO peripheral driver.
    void attachGPIO(drivers::gpio::GPIO* gpio) noexcept;

    /// Returns true if a GPIO peripheral driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Configures pin as output.
    void configureOutput(std::uint8_t pin);

    /// Configures pin as input.
    void configureInput(std::uint8_t pin);

    /// Writes boolean state (true = High, false = Low) to pin.
    void write(std::uint8_t pin, bool state);

    /// Reads boolean state (true = High, false = Low) from pin.
    [[nodiscard]] bool read(std::uint8_t pin) const;

    /// Toggles state of output pin.
    void toggle(std::uint8_t pin);

private:
    drivers::gpio::GPIO* m_gpio{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_GPIO_HAL_HPP
