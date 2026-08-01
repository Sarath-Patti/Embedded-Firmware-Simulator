#include "hal/gpio_hal.hpp"
#include "common/logger.hpp"

namespace efs::hal {

GPIOHAL::GPIOHAL(drivers::gpio::GPIO* gpio) noexcept
    : m_gpio(gpio) {
}

GPIOHAL::GPIOHAL(drivers::gpio::GPIO& gpio) noexcept
    : m_gpio(&gpio) {
}

void GPIOHAL::attachGPIO(drivers::gpio::GPIO* gpio) noexcept {
    m_gpio = gpio;
}

bool GPIOHAL::isAttached() const noexcept {
    return m_gpio != nullptr;
}

void GPIOHAL::configureOutput(std::uint8_t pin) {
    if (!m_gpio) {
        common::Logger::warning("GPIOHAL: Attempted configureOutput with no GPIO driver attached.");
        return;
    }
    m_gpio->configurePin(pin, drivers::gpio::PinDirection::Output);
}

void GPIOHAL::configureInput(std::uint8_t pin) {
    if (!m_gpio) {
        common::Logger::warning("GPIOHAL: Attempted configureInput with no GPIO driver attached.");
        return;
    }
    m_gpio->configurePin(pin, drivers::gpio::PinDirection::Input);
}

void GPIOHAL::write(std::uint8_t pin, bool state) {
    if (!m_gpio) {
        common::Logger::warning("GPIOHAL: Attempted write with no GPIO driver attached.");
        return;
    }
    m_gpio->writePin(pin, state ? drivers::gpio::PinState::High : drivers::gpio::PinState::Low);
}

bool GPIOHAL::read(std::uint8_t pin) const {
    if (!m_gpio) {
        common::Logger::warning("GPIOHAL: Attempted read with no GPIO driver attached.");
        return false;
    }
    return m_gpio->readPin(pin) == drivers::gpio::PinState::High;
}

void GPIOHAL::toggle(std::uint8_t pin) {
    if (!m_gpio) {
        common::Logger::warning("GPIOHAL: Attempted toggle with no GPIO driver attached.");
        return;
    }
    m_gpio->togglePin(pin);
}

} // namespace efs::hal
