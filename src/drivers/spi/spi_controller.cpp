#include "drivers/spi/spi_controller.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::drivers::spi {

SPIController::SPIController(mmio::MMIOBus& bus,
                             common::Address baseAddress,
                             std::uint32_t clockDivider,
                             std::uint8_t mode)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_dataRegister(std::make_shared<mmio::Register>(baseAddress + DATA_OFFSET, 0)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, 0)),
      m_controlRegister(std::make_shared<mmio::Register>(baseAddress + CONTROL_OFFSET, 0)),
      m_clockDivRegister(std::make_shared<mmio::Register>(baseAddress + CLOCK_DIV_OFFSET, clockDivider)) {
    if (clockDivider == 0) {
        common::Logger::error("Invalid initial SPI clock divider: 0");
        throw std::invalid_argument("Invalid clock divider");
    }

    if (mode > 3) {
        common::Logger::error("Invalid initial SPI mode: " + std::to_string(mode));
        throw std::invalid_argument("Invalid SPI mode");
    }

    if (!m_bus.registerRegister(m_dataRegister) ||
        !m_bus.registerRegister(m_statusRegister) ||
        !m_bus.registerRegister(m_controlRegister) ||
        !m_bus.registerRegister(m_clockDivRegister)) {
        common::Logger::error("SPIController failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("SPIController MMIO registration failure");
    }

    setMode(mode);
    updateStatusRegister();
}

SPIController::~SPIController() {
    m_bus.unregisterRegister(dataAddress());
    m_bus.unregisterRegister(statusAddress());
    m_bus.unregisterRegister(controlAddress());
    m_bus.unregisterRegister(clockDivAddress());
}

void SPIController::enable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

void SPIController::disable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

bool SPIController::enabled() const noexcept {
    return (m_controlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

std::uint8_t SPIController::transfer(std::uint8_t byte) {
    if (!enabled()) {
        common::Logger::error("Attempted SPI transfer while controller is disabled");
        throw std::runtime_error("SPI is disabled");
    }

    m_busy = true;
    updateStatusRegister();

    std::uint8_t rxByte = 0xFF;
    if (m_slave != nullptr) {
        rxByte = m_slave->transfer(byte);
    }

    m_rxBuffer.push(rxByte);
    m_dataRegister->write(rxByte);

    m_busy = false;
    updateStatusRegister();
    return rxByte;
}

void SPIController::writeByte(std::uint8_t byte) {
    (void)transfer(byte);
}

std::uint8_t SPIController::readByte() {
    if (!enabled()) {
        common::Logger::error("Attempted SPI readByte while controller is disabled");
        throw std::runtime_error("SPI is disabled");
    }
    if (m_rxBuffer.empty()) {
        common::Logger::error("Attempted SPI readByte from empty RX buffer");
        throw std::underflow_error("SPI RX buffer empty");
    }
    std::uint8_t rx = m_rxBuffer.front();
    m_rxBuffer.pop();
    updateStatusRegister();
    return rx;
}

bool SPIController::hasData() const noexcept {
    return !m_rxBuffer.empty();
}

void SPIController::setClockDivider(std::uint32_t value) {
    if (value == 0) {
        common::Logger::error("Attempted to set SPI clock divider to 0");
        throw std::invalid_argument("Clock divider cannot be 0");
    }
    m_clockDivRegister->write(value);
}

std::uint32_t SPIController::clockDivider() const noexcept {
    return m_clockDivRegister->read();
}

void SPIController::setMode(std::uint8_t mode) {
    if (mode > 3) {
        common::Logger::error("Attempted to set invalid SPI mode: " + std::to_string(mode));
        throw std::invalid_argument("Invalid SPI mode");
    }
    common::DWord ctrl = m_controlRegister->read();
    ctrl = (ctrl & ~CTRL_MODE_MASK) | (static_cast<common::DWord>(mode & 0x03) << 1);
    m_controlRegister->write(ctrl);
}

std::uint8_t SPIController::mode() const noexcept {
    return static_cast<std::uint8_t>((m_controlRegister->read() & CTRL_MODE_MASK) >> 1);
}

bool SPIController::busy() const noexcept {
    return m_busy;
}

void SPIController::attachSlave(SPIDevice* slave) noexcept {
    m_slave = slave;
}

void SPIController::detachSlave() noexcept {
    m_slave = nullptr;
}

SPIDevice* SPIController::slave() const noexcept {
    return m_slave;
}

void SPIController::reset() {
    disable();
    m_slave = nullptr;
    std::queue<std::uint8_t> empty;
    std::swap(m_rxBuffer, empty);
    m_dataRegister->reset();
    m_controlRegister->reset();
    m_clockDivRegister->reset();
    m_busy = false;
    updateStatusRegister();
}

common::Address SPIController::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address SPIController::dataAddress() const noexcept {
    return m_baseAddress + DATA_OFFSET;
}

common::Address SPIController::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

common::Address SPIController::controlAddress() const noexcept {
    return m_baseAddress + CONTROL_OFFSET;
}

common::Address SPIController::clockDivAddress() const noexcept {
    return m_baseAddress + CLOCK_DIV_OFFSET;
}

void SPIController::updateStatusRegister() {
    common::DWord status = 0;
    if (enabled()) {
        status |= STATUS_ENABLED_BIT;
    }
    if (m_busy) {
        status |= STATUS_BUSY_BIT;
    }
    if (!m_rxBuffer.empty()) {
        status |= STATUS_RX_AVAIL_BIT;
    }
    m_statusRegister->write(status);
}

} // namespace efs::drivers::spi
