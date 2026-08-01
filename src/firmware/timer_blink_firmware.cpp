#include "firmware/timer_blink_firmware.hpp"

namespace efs::firmware {

TimerBlinkFirmware::TimerBlinkFirmware(hal::GPIOHAL& gpioHAL, hal::TimerHAL& timerHAL, std::uint8_t pin, common::Size compareValue)
    : m_gpioHAL(gpioHAL), m_timerHAL(timerHAL), m_pin(pin), m_compareValue(compareValue == 0 ? 1 : compareValue) {
}

TimerBlinkFirmware::~TimerBlinkFirmware() = default;

void TimerBlinkFirmware::initialize() {
    m_initialized = true;
    m_shutdown = false;
    m_toggleCount = 0;

    m_gpioHAL.configureOutput(m_pin);
    m_gpioHAL.write(m_pin, false);

    m_timerHAL.stop();
    m_timerHAL.reset();
    m_timerHAL.setCompare(m_compareValue);
    m_timerHAL.start();
}

void TimerBlinkFirmware::update() {
    if (!m_initialized || m_shutdown) {
        return;
    }

    if (m_timerHAL.hasMatch() || m_timerHAL.counter() >= m_compareValue) {
        m_gpioHAL.toggle(m_pin);
        m_toggleCount++;
        m_timerHAL.reset();
        m_timerHAL.start();
    }
}

void TimerBlinkFirmware::execute() {
    update();
}

void TimerBlinkFirmware::shutdown() {
    if (m_initialized && !m_shutdown) {
        m_timerHAL.stop();
        m_gpioHAL.write(m_pin, false);
        m_shutdown = true;
    }
}

void TimerBlinkFirmware::reset() {
    if (m_initialized && !m_shutdown) {
        m_timerHAL.stop();
        m_gpioHAL.write(m_pin, false);
    }
    m_initialized = false;
    m_shutdown = false;
    m_toggleCount = 0;
}

common::Size TimerBlinkFirmware::toggleCount() const noexcept {
    return m_toggleCount;
}

bool TimerBlinkFirmware::isInitialized() const noexcept {
    return m_initialized;
}

bool TimerBlinkFirmware::isShutdown() const noexcept {
    return m_shutdown;
}

} // namespace efs::firmware
