#include "drivers/gpio/gpio.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::drivers::gpio {

GPIO::GPIO(mmio::MMIOBus& bus, common::Address baseAddress)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_dirRegister(std::make_shared<mmio::Register>(baseAddress + DIR_OFFSET, 0)),
      m_outRegister(std::make_shared<mmio::Register>(baseAddress + OUT_OFFSET, 0)),
      m_inRegister(std::make_shared<mmio::Register>(baseAddress + IN_OFFSET, 0)) {
    if (!m_bus.registerRegister(m_dirRegister) ||
        !m_bus.registerRegister(m_outRegister) ||
        !m_bus.registerRegister(m_inRegister)) {
        common::Logger::error("GPIO failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("GPIO MMIO registration failure");
    }
}

GPIO::~GPIO() {
    m_bus.unregisterRegister(dirAddress());
    m_bus.unregisterRegister(outAddress());
    m_bus.unregisterRegister(inAddress());
}

void GPIO::configurePin(std::uint8_t pin, PinDirection direction) {
    validatePin(pin);
    common::DWord dir = m_dirRegister->read();
    if (direction == PinDirection::Output) {
        dir |= (1U << pin);
    } else {
        dir &= ~(1U << pin);
    }
    m_dirRegister->write(dir);
}

void GPIO::writePin(std::uint8_t pin, PinState state) {
    validatePin(pin);
    if (!isOutputPin(pin)) {
        common::Logger::error("GPIO write attempt on input-configured pin " + std::to_string(pin));
        throw std::logic_error("Cannot write to GPIO pin configured as Input: " + std::to_string(pin));
    }

    common::DWord out = m_outRegister->read();
    common::DWord in = m_inRegister->read();

    if (state == PinState::High) {
        out |= (1U << pin);
        in |= (1U << pin);
    } else {
        out &= ~(1U << pin);
        in &= ~(1U << pin);
    }

    m_outRegister->write(out);
    m_inRegister->write(in);
}

PinState GPIO::readPin(std::uint8_t pin) const {
    validatePin(pin);
    common::DWord in = m_inRegister->read();
    return ((in & (1U << pin)) != 0) ? PinState::High : PinState::Low;
}

void GPIO::togglePin(std::uint8_t pin) {
    validatePin(pin);
    if (!isOutputPin(pin)) {
        common::Logger::error("GPIO toggle attempt on input-configured pin " + std::to_string(pin));
        throw std::logic_error("Cannot toggle GPIO pin configured as Input: " + std::to_string(pin));
    }

    PinState current = readPin(pin);
    writePin(pin, (current == PinState::High) ? PinState::Low : PinState::High);
}

void GPIO::setExternalInput(std::uint8_t pin, PinState state) {
    validatePin(pin);
    if (isOutputPin(pin)) {
        common::Logger::error("Attempted external input drive on output-configured GPIO pin " + std::to_string(pin));
        throw std::logic_error("Cannot drive external input on output pin: " + std::to_string(pin));
    }

    common::DWord in = m_inRegister->read();
    if (state == PinState::High) {
        in |= (1U << pin);
    } else {
        in &= ~(1U << pin);
    }
    m_inRegister->write(in);
}

void GPIO::reset() {
    m_dirRegister->reset();
    m_outRegister->reset();
    m_inRegister->reset();
}

common::Address GPIO::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address GPIO::dirAddress() const noexcept {
    return m_baseAddress + DIR_OFFSET;
}

common::Address GPIO::outAddress() const noexcept {
    return m_baseAddress + OUT_OFFSET;
}

common::Address GPIO::inAddress() const noexcept {
    return m_baseAddress + IN_OFFSET;
}

void GPIO::validatePin(std::uint8_t pin) const {
    if (pin >= MAX_PINS) {
        common::Logger::error("GPIO pin index out of range: " + std::to_string(pin));
        throw std::out_of_range("GPIO pin index out of range: " + std::to_string(pin));
    }
}

bool GPIO::isOutputPin(std::uint8_t pin) const noexcept {
    return (m_dirRegister->read() & (1U << pin)) != 0;
}

} // namespace efs::drivers::gpio
