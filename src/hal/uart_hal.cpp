#include "hal/uart_hal.hpp"
#include "common/logger.hpp"

namespace efs::hal {

UARTHAL::UARTHAL(drivers::uart::UART* uart) noexcept {
    attachUART(uart);
}

UARTHAL::UARTHAL(drivers::uart::UART& uart) noexcept {
    attachUART(&uart);
}

void UARTHAL::attachUART(drivers::uart::UART* uart) noexcept {
    m_uart = uart;
    if (m_uart != nullptr) {
        m_uart->enable();
    }
}

bool UARTHAL::isAttached() const noexcept {
    return m_uart != nullptr;
}

void UARTHAL::writeByte(std::uint8_t byte) {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted writeByte with no UART driver attached.");
        return;
    }
    m_uart->writeByte(byte);
}

std::uint8_t UARTHAL::readByte() {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted readByte with no UART driver attached.");
        return 0;
    }
    return m_uart->readByte();
}

bool UARTHAL::hasData() const {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted hasData read with no UART driver attached.");
        return false;
    }
    return m_uart->hasReceivedData();
}

void UARTHAL::setBaudRate(std::uint32_t rate) {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted setBaudRate with no UART driver attached.");
        return;
    }
    m_uart->setBaudRate(rate);
}

void UARTHAL::enable() {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted enable with no UART driver attached.");
        return;
    }
    m_uart->enable();
}

void UARTHAL::disable() {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted disable with no UART driver attached.");
        return;
    }
    m_uart->disable();
}

bool UARTHAL::enabled() const {
    if (!m_uart) {
        common::Logger::warning("UARTHAL: Attempted enabled read with no UART driver attached.");
        return false;
    }
    return m_uart->enabled();
}

} // namespace efs::hal
