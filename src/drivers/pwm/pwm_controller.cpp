#include "drivers/pwm/pwm_controller.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::drivers::pwm {

PWMController::PWMController(mmio::MMIOBus& bus,
                             common::Address baseAddress,
                             std::uint32_t frequencyHz,
                             std::uint32_t dutyCyclePercent,
                             std::uint32_t systemClockHz)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_systemClockHz(systemClockHz),
      m_controlRegister(std::make_shared<mmio::Register>(baseAddress + CTRL_OFFSET, 0)),
      m_freqRegister(std::make_shared<mmio::Register>(baseAddress + FREQ_OFFSET, frequencyHz)),
      m_dutyRegister(std::make_shared<mmio::Register>(baseAddress + DUTY_OFFSET, dutyCyclePercent)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, 0)),
      m_timer(bus, baseAddress + TIMER_OFFSET) {
    if (systemClockHz == 0) {
        common::Logger::error("Invalid system clock frequency: 0");
        throw std::invalid_argument("System clock frequency cannot be zero");
    }
    if (frequencyHz == 0 || frequencyHz > systemClockHz) {
        common::Logger::error("Invalid initial PWM frequency: " + std::to_string(frequencyHz));
        throw std::invalid_argument("Invalid PWM frequency");
    }
    if (dutyCyclePercent > 100) {
        common::Logger::error("Invalid initial PWM duty cycle: " + std::to_string(dutyCyclePercent));
        throw std::invalid_argument("Invalid PWM duty cycle");
    }

    if (!m_bus.registerRegister(m_controlRegister) ||
        !m_bus.registerRegister(m_freqRegister) ||
        !m_bus.registerRegister(m_dutyRegister) ||
        !m_bus.registerRegister(m_statusRegister)) {
        common::Logger::error("PWMController failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("PWMController MMIO registration failure");
    }

    updatePeriodAndDuty();
    updateStatusRegister();
}

PWMController::~PWMController() {
    m_bus.unregisterRegister(controlAddress());
    m_bus.unregisterRegister(freqAddress());
    m_bus.unregisterRegister(dutyAddress());
    m_bus.unregisterRegister(statusAddress());
}

void PWMController::enable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);

    m_timer.setAutoReset(true);
    m_timer.setCompare(m_periodTicks);
    m_timer.start();

    updateStatusRegister();
}

void PWMController::disable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);

    m_timer.stop();
    updateStatusRegister();
}

bool PWMController::enabled() const noexcept {
    return (m_controlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

void PWMController::setFrequency(std::uint32_t frequencyHz) {
    if (frequencyHz == 0 || frequencyHz > m_systemClockHz) {
        common::Logger::error("Attempted to set invalid PWM frequency: " + std::to_string(frequencyHz));
        throw std::invalid_argument("Invalid PWM frequency");
    }
    m_freqRegister->write(frequencyHz);
    updatePeriodAndDuty();
    updateStatusRegister();
}

std::uint32_t PWMController::frequency() const noexcept {
    return m_freqRegister->read();
}

void PWMController::setDutyCycle(std::uint32_t percent) {
    if (percent > 100) {
        common::Logger::error("Attempted to set invalid PWM duty cycle: " + std::to_string(percent));
        throw std::invalid_argument("Duty cycle percentage must be 0-100");
    }
    m_dutyRegister->write(percent);
    updatePeriodAndDuty();
    updateStatusRegister();
}

std::uint32_t PWMController::dutyCycle() const noexcept {
    return m_dutyRegister->read();
}

bool PWMController::outputState() const noexcept {
    if (!enabled()) {
        return false;
    }
    std::uint32_t duty = dutyCycle();
    if (duty == 0) {
        return false;
    }
    if (duty == 100) {
        return true;
    }
    common::DWord cnt = (m_periodTicks > 0) ? (m_timer.counter() % m_periodTicks) : 0;
    return cnt < m_onTicks;
}

void PWMController::tick() {
    if (enabled()) {
        m_timer.tick();
    }
    updateStatusRegister();
}

void PWMController::reset() {
    disable();
    m_controlRegister->reset();
    m_freqRegister->reset();
    m_dutyRegister->reset();
    m_timer.reset();
    updatePeriodAndDuty();
    updateStatusRegister();
}

const timer::Timer& PWMController::timer() const noexcept {
    return m_timer;
}

timer::Timer& PWMController::timer() noexcept {
    return m_timer;
}

common::Address PWMController::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address PWMController::controlAddress() const noexcept {
    return m_baseAddress + CTRL_OFFSET;
}

common::Address PWMController::freqAddress() const noexcept {
    return m_baseAddress + FREQ_OFFSET;
}

common::Address PWMController::dutyAddress() const noexcept {
    return m_baseAddress + DUTY_OFFSET;
}

common::Address PWMController::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

void PWMController::updatePeriodAndDuty() {
    std::uint32_t freq = frequency();
    std::uint32_t duty = dutyCycle();
    if (freq == 0) {
        freq = 1;
    }
    m_periodTicks = m_systemClockHz / freq;
    if (m_periodTicks == 0) {
        m_periodTicks = 1;
    }
    m_onTicks = static_cast<common::DWord>((static_cast<std::uint64_t>(m_periodTicks) * duty) / 100);
    if (enabled()) {
        m_timer.setCompare(m_periodTicks);
    }
}

void PWMController::updateStatusRegister() {
    common::DWord status = 0;
    if (enabled()) {
        status |= STATUS_ENABLED_BIT;
    }
    if (outputState()) {
        status |= STATUS_OUTPUT_BIT;
    }
    m_statusRegister->write(status);
}

} // namespace efs::drivers::pwm
