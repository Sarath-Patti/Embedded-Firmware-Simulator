#include "hal/adc_hal.hpp"
#include "common/logger.hpp"
#include <stdexcept>

namespace efs::hal {

ADCHAL::ADCHAL(drivers::adc::ADCController* controller) noexcept
    : m_controller(controller) {
}

ADCHAL::ADCHAL(drivers::adc::ADCController& controller) noexcept
    : m_controller(&controller) {
}

void ADCHAL::attachADC(drivers::adc::ADCController* controller) noexcept {
    m_controller = controller;
}

bool ADCHAL::isAttached() const noexcept {
    return m_controller != nullptr;
}

void ADCHAL::enable() {
    if (!isAttached()) {
        common::Logger::error("ADCHAL enable called with no ADCController attached");
        throw std::runtime_error("ADCHAL: unattached driver");
    }
    m_controller->enable();
}

void ADCHAL::disable() {
    if (!isAttached()) {
        common::Logger::error("ADCHAL disable called with no ADCController attached");
        throw std::runtime_error("ADCHAL: unattached driver");
    }
    m_controller->disable();
}

bool ADCHAL::enabled() const {
    if (!isAttached()) {
        return false;
    }
    return m_controller->enabled();
}

std::uint32_t ADCHAL::read(std::size_t channel) {
    if (!isAttached()) {
        common::Logger::error("ADCHAL read called with no ADCController attached");
        throw std::runtime_error("ADCHAL: unattached driver");
    }
    return m_controller->sample(channel);
}

void ADCHAL::setReferenceVoltage(double voltage) {
    if (!isAttached()) {
        common::Logger::error("ADCHAL setReferenceVoltage called with no ADCController attached");
        throw std::runtime_error("ADCHAL: unattached driver");
    }
    m_controller->setReferenceVoltage(voltage);
}

double ADCHAL::referenceVoltage() const {
    if (!isAttached()) {
        common::Logger::error("ADCHAL referenceVoltage called with no ADCController attached");
        throw std::runtime_error("ADCHAL: unattached driver");
    }
    return m_controller->referenceVoltage();
}

} // namespace efs::hal
