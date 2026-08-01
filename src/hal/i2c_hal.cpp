#include "hal/i2c_hal.hpp"
#include "common/logger.hpp"
#include <stdexcept>

namespace efs::hal {

I2CHAL::I2CHAL(drivers::i2c::I2CController* controller) noexcept
    : m_controller(controller) {
}

I2CHAL::I2CHAL(drivers::i2c::I2CController& controller) noexcept
    : m_controller(&controller) {
}

void I2CHAL::attachI2C(drivers::i2c::I2CController* controller) noexcept {
    m_controller = controller;
}

bool I2CHAL::isAttached() const noexcept {
    return m_controller != nullptr;
}

bool I2CHAL::beginTransmission(std::uint8_t address) {
    if (!isAttached()) {
        common::Logger::error("I2CHAL beginTransmission called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    m_controller->setSlaveAddress(address);
    return m_controller->start(false /* write mode */);
}

bool I2CHAL::endTransmission() {
    if (!isAttached()) {
        common::Logger::error("I2CHAL endTransmission called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    bool ack = m_controller->lastAck();
    m_controller->stop();
    return ack;
}

bool I2CHAL::writeByte(std::uint8_t byte) {
    if (!isAttached()) {
        common::Logger::error("I2CHAL writeByte called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    return m_controller->writeByte(byte);
}

std::uint8_t I2CHAL::readByte() {
    if (!m_rxQueue.empty()) {
        std::uint8_t b = m_rxQueue.front();
        m_rxQueue.pop();
        return b;
    }
    if (!isAttached()) {
        common::Logger::error("I2CHAL readByte called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    return m_controller->readByte();
}

std::size_t I2CHAL::requestFrom(std::uint8_t address, std::size_t quantity) {
    if (!isAttached()) {
        common::Logger::error("I2CHAL requestFrom called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }

    std::queue<std::uint8_t> empty;
    std::swap(m_rxQueue, empty);

    m_controller->setSlaveAddress(address);
    bool ack = m_controller->start(true /* read mode */);
    if (!ack) {
        m_controller->stop();
        return 0;
    }

    std::size_t count = 0;
    for (std::size_t i = 0; i < quantity; ++i) {
        std::uint8_t b = m_controller->readByte();
        m_rxQueue.push(b);
        count++;
    }

    m_controller->stop();
    return count;
}

std::size_t I2CHAL::requestFrom(std::uint8_t address) {
    return requestFrom(address, 1);
}

std::size_t I2CHAL::available() const noexcept {
    return m_rxQueue.size();
}

void I2CHAL::enable() {
    if (!isAttached()) {
        common::Logger::error("I2CHAL enable called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    m_controller->enable();
}

void I2CHAL::disable() {
    if (!isAttached()) {
        common::Logger::error("I2CHAL disable called with no I2CController attached");
        throw std::runtime_error("I2CHAL: unattached driver");
    }
    m_controller->disable();
}

bool I2CHAL::enabled() const {
    if (!isAttached()) {
        return false;
    }
    return m_controller->enabled();
}

} // namespace efs::hal
