#include "firmware/basic_firmware.hpp"

namespace efs::firmware {

BasicFirmware::BasicFirmware(drivers::gpio::GPIO& gpio, std::uint8_t pin, common::Size toggleInterval)
    : m_gpio(gpio), m_pin(pin), m_toggleInterval(toggleInterval == 0 ? 1 : toggleInterval) {
}

void BasicFirmware::initialize() {
    m_initialized = true;
    m_shutdown = false;
    m_cycleCounter = 0;
    m_executionCount = 0;
    m_gpio.configurePin(m_pin, drivers::gpio::PinDirection::Output);
    m_gpio.writePin(m_pin, drivers::gpio::PinState::Low);
}

void BasicFirmware::execute() {
    if (!m_initialized || m_shutdown) {
        return;
    }
    m_executionCount++;
    m_cycleCounter++;

    if (m_cycleCounter >= m_toggleInterval) {
        m_gpio.togglePin(m_pin);
        m_cycleCounter = 0;
    }
}

void BasicFirmware::shutdown() {
    if (m_initialized && !m_shutdown) {
        m_gpio.writePin(m_pin, drivers::gpio::PinState::Low);
        m_shutdown = true;
    }
}

common::Size BasicFirmware::executionCount() const noexcept {
    return m_executionCount;
}

bool BasicFirmware::isInitialized() const noexcept {
    return m_initialized;
}

bool BasicFirmware::isShutdown() const noexcept {
    return m_shutdown;
}

} // namespace efs::firmware
