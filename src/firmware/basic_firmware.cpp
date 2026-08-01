#include "firmware/basic_firmware.hpp"
#include "drivers/gpio/gpio.hpp"

namespace efs::firmware {

BasicFirmware::BasicFirmware(hal::GPIOHAL& gpioHAL, std::uint8_t pin, common::Size toggleInterval)
    : m_gpioHAL(gpioHAL), m_pin(pin), m_toggleInterval(toggleInterval == 0 ? 1 : toggleInterval) {
}

BasicFirmware::BasicFirmware(drivers::gpio::GPIO& gpio, std::uint8_t pin, common::Size toggleInterval)
    : m_gpioHAL(gpio), m_pin(pin), m_toggleInterval(toggleInterval == 0 ? 1 : toggleInterval) {
}

BasicFirmware::~BasicFirmware() = default;

void BasicFirmware::initialize() {
    m_initialized = true;
    m_shutdown = false;
    m_cycleCounter = 0;
    m_executionCount = 0;
    m_gpioHAL.configureOutput(m_pin);
    m_gpioHAL.write(m_pin, false);
}

void BasicFirmware::update() {
    if (!m_initialized || m_shutdown) {
        return;
    }
    m_executionCount++;
    m_cycleCounter++;

    if (m_cycleCounter >= m_toggleInterval) {
        m_gpioHAL.toggle(m_pin);
        m_cycleCounter = 0;
    }
}

void BasicFirmware::execute() {
    update();
}

void BasicFirmware::shutdown() {
    if (m_initialized && !m_shutdown) {
        m_gpioHAL.write(m_pin, false);
        m_shutdown = true;
    }
}

void BasicFirmware::reset() {
    if (m_initialized && !m_shutdown) {
        m_gpioHAL.write(m_pin, false);
    }
    m_initialized = false;
    m_shutdown = false;
    m_cycleCounter = 0;
    m_executionCount = 0;
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
