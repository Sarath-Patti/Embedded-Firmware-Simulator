#include "drivers/uart/uart.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::drivers::uart {

UART::UART(mmio::MMIOBus& bus, common::Address baseAddress, std::uint32_t defaultBaudRate)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_dataRegister(std::make_shared<mmio::Register>(baseAddress + DATA_OFFSET, 0)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, STATUS_TX_EMPTY_BIT)),
      m_controlRegister(std::make_shared<mmio::Register>(baseAddress + CONTROL_OFFSET, 0)),
      m_baudRegister(std::make_shared<mmio::Register>(baseAddress + BAUD_OFFSET, defaultBaudRate)) {
    if (defaultBaudRate == 0) {
        common::Logger::error("Invalid initial baud rate: 0");
        throw std::invalid_argument("Invalid baud rate");
    }

    if (!m_bus.registerRegister(m_dataRegister) ||
        !m_bus.registerRegister(m_statusRegister) ||
        !m_bus.registerRegister(m_controlRegister) ||
        !m_bus.registerRegister(m_baudRegister)) {
        common::Logger::error("UART failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("UART MMIO registration failure");
    }

    updateStatusRegister();
}

UART::~UART() {
    m_bus.unregisterRegister(dataAddress());
    m_bus.unregisterRegister(statusAddress());
    m_bus.unregisterRegister(controlAddress());
    m_bus.unregisterRegister(baudAddress());
}

void UART::enable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

void UART::disable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

bool UART::enabled() const noexcept {
    return (m_controlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

void UART::writeByte(std::uint8_t byte) {
    if (!enabled()) {
        common::Logger::error("Attempted to write byte to disabled UART");
        throw std::runtime_error("UART is disabled");
    }
    m_txFifo.push(byte);
    m_dataRegister->write(byte);
    updateStatusRegister();
}

std::uint8_t UART::readByte() {
    if (!enabled()) {
        common::Logger::error("Attempted to read byte from disabled UART");
        throw std::runtime_error("UART is disabled");
    }
    if (m_rxFifo.empty()) {
        common::Logger::error("Attempted to read from empty UART RX FIFO");
        throw std::underflow_error("RX FIFO is empty");
    }
    std::uint8_t byte = m_rxFifo.front();
    m_rxFifo.pop();
    m_dataRegister->write(byte);
    updateStatusRegister();
    return byte;
}

void UART::pushReceivedByte(std::uint8_t byte) {
    if (!enabled()) {
        common::Logger::error("Attempted to push received byte into disabled UART");
        throw std::runtime_error("UART is disabled");
    }
    m_rxFifo.push(byte);
    updateStatusRegister();
}

bool UART::hasReceivedData() const noexcept {
    return !m_rxFifo.empty();
}

bool UART::txEmpty() const noexcept {
    return m_txFifo.empty();
}

void UART::setBaudRate(std::uint32_t baud) {
    if (baud == 0) {
        common::Logger::error("Attempted to set invalid baud rate 0 on UART");
        throw std::invalid_argument("Invalid baud rate");
    }
    m_baudRegister->write(baud);
}

std::uint32_t UART::baudRate() const noexcept {
    return m_baudRegister->read();
}

std::size_t UART::txFifoSize() const noexcept {
    return m_txFifo.size();
}

std::size_t UART::rxFifoSize() const noexcept {
    return m_rxFifo.size();
}

std::uint8_t UART::popTxByte() {
    if (m_txFifo.empty()) {
        common::Logger::error("Attempted to pop from empty UART TX FIFO");
        throw std::underflow_error("TX FIFO is empty");
    }
    std::uint8_t byte = m_txFifo.front();
    m_txFifo.pop();
    updateStatusRegister();
    return byte;
}

common::Address UART::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address UART::dataAddress() const noexcept {
    return m_baseAddress + DATA_OFFSET;
}

common::Address UART::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

common::Address UART::controlAddress() const noexcept {
    return m_baseAddress + CONTROL_OFFSET;
}

common::Address UART::baudAddress() const noexcept {
    return m_baseAddress + BAUD_OFFSET;
}

void UART::updateStatusRegister() {
    common::DWord status = 0;
    if (m_txFifo.empty()) {
        status |= STATUS_TX_EMPTY_BIT;
    }
    if (!m_rxFifo.empty()) {
        status |= STATUS_RX_AVAIL_BIT;
    }
    if (enabled()) {
        status |= STATUS_ENABLED_BIT;
    }
    m_statusRegister->write(status);
}

} // namespace efs::drivers::uart
