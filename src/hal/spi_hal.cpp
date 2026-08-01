#include "hal/spi_hal.hpp"
#include "common/logger.hpp"
#include <stdexcept>

namespace efs::hal {

SPIHAL::SPIHAL(drivers::spi::SPIController* controller) noexcept
    : m_controller(controller) {
}

SPIHAL::SPIHAL(drivers::spi::SPIController& controller) noexcept
    : m_controller(&controller) {
}

void SPIHAL::attachSPI(drivers::spi::SPIController* controller) noexcept {
    m_controller = controller;
}

bool SPIHAL::isAttached() const noexcept {
    return m_controller != nullptr;
}

void SPIHAL::writeByte(std::uint8_t byte) {
    if (!isAttached()) {
        common::Logger::error("SPIHAL writeByte called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    m_controller->writeByte(byte);
}

std::uint8_t SPIHAL::readByte() {
    if (!isAttached()) {
        common::Logger::error("SPIHAL readByte called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    return m_controller->readByte();
}

std::uint8_t SPIHAL::transfer(std::uint8_t byte) {
    if (!isAttached()) {
        common::Logger::error("SPIHAL transfer called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    return m_controller->transfer(byte);
}

void SPIHAL::configure(std::uint8_t mode, std::uint32_t clockDivider) {
    if (!isAttached()) {
        common::Logger::error("SPIHAL configure called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    m_controller->setMode(mode);
    m_controller->setClockDivider(clockDivider);
}

void SPIHAL::enable() {
    if (!isAttached()) {
        common::Logger::error("SPIHAL enable called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    m_controller->enable();
}

void SPIHAL::disable() {
    if (!isAttached()) {
        common::Logger::error("SPIHAL disable called with no SPIController attached");
        throw std::runtime_error("SPIHAL: unattached driver");
    }
    m_controller->disable();
}

bool SPIHAL::enabled() const {
    if (!isAttached()) {
        return false;
    }
    return m_controller->enabled();
}

} // namespace efs::hal
