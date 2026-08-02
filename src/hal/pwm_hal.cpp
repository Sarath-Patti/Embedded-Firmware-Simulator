#include "hal/pwm_hal.hpp"
#include "common/logger.hpp"
#include <stdexcept>

namespace efs::hal {

PWMHAL::PWMHAL(drivers::pwm::PWMController* controller) noexcept
    : m_controller(controller) {
}

PWMHAL::PWMHAL(drivers::pwm::PWMController& controller) noexcept
    : m_controller(&controller) {
}

void PWMHAL::attachPWM(drivers::pwm::PWMController* controller) noexcept {
    m_controller = controller;
}

bool PWMHAL::isAttached() const noexcept {
    return m_controller != nullptr;
}

void PWMHAL::enable() {
    if (!isAttached()) {
        common::Logger::error("PWMHAL enable called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    m_controller->enable();
}

void PWMHAL::disable() {
    if (!isAttached()) {
        common::Logger::error("PWMHAL disable called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    m_controller->disable();
}

bool PWMHAL::enabled() const {
    if (!isAttached()) {
        return false;
    }
    return m_controller->enabled();
}

void PWMHAL::setDutyCycle(std::uint32_t percent) {
    if (!isAttached()) {
        common::Logger::error("PWMHAL setDutyCycle called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    m_controller->setDutyCycle(percent);
}

std::uint32_t PWMHAL::dutyCycle() const {
    if (!isAttached()) {
        common::Logger::error("PWMHAL dutyCycle called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    return m_controller->dutyCycle();
}

void PWMHAL::setFrequency(std::uint32_t hz) {
    if (!isAttached()) {
        common::Logger::error("PWMHAL setFrequency called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    m_controller->setFrequency(hz);
}

std::uint32_t PWMHAL::frequency() const {
    if (!isAttached()) {
        common::Logger::error("PWMHAL frequency called with no PWMController attached");
        throw std::runtime_error("PWMHAL: unattached driver");
    }
    return m_controller->frequency();
}

bool PWMHAL::outputState() const {
    if (!isAttached()) {
        return false;
    }
    return m_controller->outputState();
}

} // namespace efs::hal
