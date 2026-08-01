#include "firmware/uart_echo_firmware.hpp"

namespace efs::firmware {

UARTEchoFirmware::UARTEchoFirmware(hal::UARTHAL& uartHAL, std::uint32_t baudRate)
    : m_uartHAL(uartHAL), m_baudRate(baudRate) {
}

UARTEchoFirmware::~UARTEchoFirmware() = default;

void UARTEchoFirmware::initialize() {
    m_initialized = true;
    m_shutdown = false;
    m_echoCount = 0;

    m_uartHAL.setBaudRate(m_baudRate);
    m_uartHAL.enable();
}

void UARTEchoFirmware::update() {
    if (!m_initialized || m_shutdown) {
        return;
    }

    while (m_uartHAL.hasData()) {
        std::uint8_t rxByte = m_uartHAL.readByte();
        m_uartHAL.writeByte(rxByte);
        m_echoCount++;
    }
}

void UARTEchoFirmware::execute() {
    update();
}

void UARTEchoFirmware::shutdown() {
    if (m_initialized && !m_shutdown) {
        m_uartHAL.disable();
        m_shutdown = true;
    }
}

void UARTEchoFirmware::reset() {
    if (m_initialized && !m_shutdown) {
        m_uartHAL.disable();
    }
    m_initialized = false;
    m_shutdown = false;
    m_echoCount = 0;
}

common::Size UARTEchoFirmware::echoCount() const noexcept {
    return m_echoCount;
}

bool UARTEchoFirmware::isInitialized() const noexcept {
    return m_initialized;
}

bool UARTEchoFirmware::isShutdown() const noexcept {
    return m_shutdown;
}

} // namespace efs::firmware
