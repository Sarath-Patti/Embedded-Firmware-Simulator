#include "drivers/adc/adc_controller.hpp"
#include "common/logger.hpp"
#include <cmath>
#include <stdexcept>
#include <string>

namespace efs::drivers::adc {

ADCController::ADCController(mmio::MMIOBus& bus,
                             common::Address baseAddress,
                             std::uint8_t resolutionBits,
                             double referenceVoltage,
                             std::size_t channelCount)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_resolutionBits(resolutionBits),
      m_referenceVoltage(referenceVoltage),
      m_channels(channelCount, 0.0),
      m_controlRegister(std::make_shared<mmio::Register>(baseAddress + CTRL_OFFSET, 0)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, 0)),
      m_resRegister(std::make_shared<mmio::Register>(baseAddress + RES_OFFSET, resolutionBits)),
      m_dataRegister(std::make_shared<mmio::Register>(baseAddress + DATA_OFFSET, 0)) {
    if (channelCount == 0) {
        common::Logger::error("ADC channel count cannot be zero");
        throw std::invalid_argument("Channel count cannot be zero");
    }
    if (resolutionBits != 8 && resolutionBits != 10 && resolutionBits != 12) {
        common::Logger::error("Invalid initial ADC resolution: " + std::to_string(resolutionBits));
        throw std::invalid_argument("Invalid ADC resolution bits (must be 8, 10, or 12)");
    }
    if (referenceVoltage <= 0.0) {
        common::Logger::error("Invalid initial reference voltage: " + std::to_string(referenceVoltage));
        throw std::invalid_argument("Reference voltage must be positive");
    }

    if (!m_bus.registerRegister(m_controlRegister) ||
        !m_bus.registerRegister(m_statusRegister) ||
        !m_bus.registerRegister(m_resRegister) ||
        !m_bus.registerRegister(m_dataRegister)) {
        common::Logger::error("ADCController failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("ADCController MMIO registration failure");
    }

    updateStatusRegister();
}

ADCController::~ADCController() {
    m_bus.unregisterRegister(controlAddress());
    m_bus.unregisterRegister(statusAddress());
    m_bus.unregisterRegister(resAddress());
    m_bus.unregisterRegister(dataAddress());
}

void ADCController::enable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

void ADCController::disable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

bool ADCController::enabled() const noexcept {
    return (m_controlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

void ADCController::setResolution(std::uint8_t bits) {
    if (bits != 8 && bits != 10 && bits != 12) {
        common::Logger::error("Attempted to set invalid ADC resolution: " + std::to_string(bits));
        throw std::invalid_argument("Invalid resolution bits (must be 8, 10, or 12)");
    }
    m_resolutionBits = bits;
    m_resRegister->write(bits);
}

std::uint8_t ADCController::resolution() const noexcept {
    return m_resolutionBits;
}

void ADCController::setReferenceVoltage(double voltage) {
    if (voltage <= 0.0) {
        common::Logger::error("Attempted to set invalid reference voltage: " + std::to_string(voltage));
        throw std::invalid_argument("Reference voltage must be positive");
    }
    m_referenceVoltage = voltage;
}

double ADCController::referenceVoltage() const noexcept {
    return m_referenceVoltage;
}

void ADCController::setAnalogInput(std::size_t channel, double voltage) {
    if (channel >= m_channels.size()) {
        common::Logger::error("Invalid ADC channel index: " + std::to_string(channel));
        throw std::out_of_range("Invalid ADC channel");
    }
    if (voltage < 0.0) {
        common::Logger::error("Attempted to set negative analog input voltage: " + std::to_string(voltage));
        throw std::invalid_argument("Analog input voltage cannot be negative");
    }
    m_channels[channel] = voltage;
}

double ADCController::analogInput(std::size_t channel) const {
    if (channel >= m_channels.size()) {
        common::Logger::error("Invalid ADC channel index: " + std::to_string(channel));
        throw std::out_of_range("Invalid ADC channel");
    }
    return m_channels[channel];
}

std::uint32_t ADCController::sample(std::size_t channel) {
    if (!enabled()) {
        common::Logger::error("Attempted ADC sampling while peripheral is disabled");
        throw std::runtime_error("ADC is disabled");
    }
    if (channel >= m_channels.size()) {
        common::Logger::error("Attempted ADC sampling on invalid channel: " + std::to_string(channel));
        throw std::out_of_range("Invalid ADC channel");
    }

    double vin = m_channels[channel];
    double vref = m_referenceVoltage;
    std::uint32_t maxDigital = (1U << m_resolutionBits) - 1U;

    std::uint32_t raw = 0;
    if (vin <= 0.0) {
        raw = 0;
    } else if (vin >= vref) {
        raw = maxDigital;
    } else {
        double ratio = vin / vref;
        raw = static_cast<std::uint32_t>(std::round(ratio * static_cast<double>(maxDigital)));
        if (raw > maxDigital) {
            raw = maxDigital;
        }
    }

    m_lastSample = raw;
    m_dataRegister->write(raw);
    updateStatusRegister();
    return raw;
}

std::uint32_t ADCController::lastSample() const noexcept {
    return m_lastSample;
}

std::size_t ADCController::channelCount() const noexcept {
    return m_channels.size();
}

void ADCController::reset() {
    disable();
    std::fill(m_channels.begin(), m_channels.end(), 0.0);
    m_resolutionBits = 12;
    m_referenceVoltage = 3.3;
    m_lastSample = 0;
    m_controlRegister->reset();
    m_resRegister->write(12);
    m_dataRegister->reset();
    updateStatusRegister();
}

common::Address ADCController::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address ADCController::controlAddress() const noexcept {
    return m_baseAddress + CTRL_OFFSET;
}

common::Address ADCController::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

common::Address ADCController::resAddress() const noexcept {
    return m_baseAddress + RES_OFFSET;
}

common::Address ADCController::dataAddress() const noexcept {
    return m_baseAddress + DATA_OFFSET;
}

void ADCController::updateStatusRegister() {
    common::DWord status = 0;
    if (enabled()) {
        status |= STATUS_ENABLED_BIT;
        status |= STATUS_COMPLETE_BIT;
    }
    m_statusRegister->write(status);
}

} // namespace efs::drivers::adc
