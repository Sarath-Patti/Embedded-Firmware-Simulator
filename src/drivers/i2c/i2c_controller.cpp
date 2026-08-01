#include "drivers/i2c/i2c_controller.hpp"
#include "common/logger.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace efs::drivers::i2c {

I2CController::I2CController(mmio::MMIOBus& bus,
                             common::Address baseAddress,
                             std::uint8_t defaultSlaveAddress)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_dataRegister(std::make_shared<mmio::Register>(baseAddress + DATA_OFFSET, 0)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, 0)),
      m_controlRegister(std::make_shared<mmio::Register>(baseAddress + CONTROL_OFFSET, 0)),
      m_addrRegister(std::make_shared<mmio::Register>(baseAddress + ADDR_OFFSET, defaultSlaveAddress & 0x7F)) {
    if (!m_bus.registerRegister(m_dataRegister) ||
        !m_bus.registerRegister(m_statusRegister) ||
        !m_bus.registerRegister(m_controlRegister) ||
        !m_bus.registerRegister(m_addrRegister)) {
        common::Logger::error("I2CController failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("I2CController MMIO registration failure");
    }

    updateStatusRegister();
}

I2CController::~I2CController() {
    m_bus.unregisterRegister(dataAddress());
    m_bus.unregisterRegister(statusAddress());
    m_bus.unregisterRegister(controlAddress());
    m_bus.unregisterRegister(slaveAddrAddress());
}

void I2CController::enable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    updateStatusRegister();
}

void I2CController::disable() {
    common::DWord ctrl = m_controlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_controlRegister->write(ctrl);
    m_busy = false;
    m_activeDevice = nullptr;
    updateStatusRegister();
}

bool I2CController::enabled() const noexcept {
    return (m_controlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

bool I2CController::start(bool isRead) {
    if (!enabled()) {
        common::Logger::error("Attempted I2C start while controller is disabled");
        throw std::runtime_error("I2C is disabled");
    }

    m_busy = true;
    m_isReadMode = isRead;
    std::uint8_t target = slaveAddress();

    auto it = std::find_if(m_devices.begin(), m_devices.end(), [target](const I2CDevice* dev) {
        return dev != nullptr && dev->address() == target;
    });

    if (it != m_devices.end()) {
        bool ack = (*it)->onStart(isRead);
        m_lastAck = ack;
        m_activeDevice = ack ? *it : nullptr;
    } else {
        m_lastAck = false;
        m_activeDevice = nullptr;
    }

    updateStatusRegister();
    return m_lastAck;
}

void I2CController::stop() {
    if (m_activeDevice != nullptr) {
        m_activeDevice->onStop();
    }
    m_activeDevice = nullptr;
    m_busy = false;
    updateStatusRegister();
}

bool I2CController::writeByte(std::uint8_t byte) {
    if (!enabled()) {
        common::Logger::error("Attempted I2C writeByte while controller is disabled");
        throw std::runtime_error("I2C is disabled");
    }
    if (!m_busy) {
        common::Logger::error("Attempted I2C writeByte without active transaction START");
        throw std::runtime_error("I2C transaction not started");
    }

    bool ack = false;
    if (m_activeDevice != nullptr) {
        ack = m_activeDevice->onWrite(byte);
    }
    m_lastAck = ack;
    m_dataRegister->write(byte);

    updateStatusRegister();
    return ack;
}

std::uint8_t I2CController::readByte() {
    if (!enabled()) {
        common::Logger::error("Attempted I2C readByte while controller is disabled");
        throw std::runtime_error("I2C is disabled");
    }

    if (!m_rxBuffer.empty()) {
        std::uint8_t b = m_rxBuffer.front();
        m_rxBuffer.pop();
        updateStatusRegister();
        return b;
    }

    if (!m_busy || m_activeDevice == nullptr) {
        common::Logger::error("Attempted I2C readByte from empty RX buffer and no active slave device");
        throw std::underflow_error("I2C RX buffer empty");
    }

    std::uint8_t b = m_activeDevice->onRead();
    m_dataRegister->write(b);
    updateStatusRegister();
    return b;
}

bool I2CController::hasData() const noexcept {
    return !m_rxBuffer.empty();
}

void I2CController::setSlaveAddress(std::uint8_t address) {
    m_addrRegister->write(address & 0x7F);
}

std::uint8_t I2CController::slaveAddress() const noexcept {
    return static_cast<std::uint8_t>(m_addrRegister->read() & 0x7F);
}

bool I2CController::busy() const noexcept {
    return m_busy;
}

bool I2CController::lastAck() const noexcept {
    return m_lastAck;
}

void I2CController::attachDevice(I2CDevice* device) {
    if (device == nullptr) {
        return;
    }
    auto it = std::find(m_devices.begin(), m_devices.end(), device);
    if (it == m_devices.end()) {
        m_devices.push_back(device);
    }
}

void I2CController::detachDevice(I2CDevice* device) {
    if (device == nullptr) {
        return;
    }
    auto it = std::find(m_devices.begin(), m_devices.end(), device);
    if (it != m_devices.end()) {
        m_devices.erase(it);
    }
    if (m_activeDevice == device) {
        m_activeDevice = nullptr;
    }
}

void I2CController::reset() {
    disable();
    m_devices.clear();
    m_activeDevice = nullptr;
    std::queue<std::uint8_t> empty;
    std::swap(m_rxBuffer, empty);
    m_dataRegister->reset();
    m_controlRegister->reset();
    m_addrRegister->reset();
    m_busy = false;
    m_lastAck = false;
    updateStatusRegister();
}

common::Address I2CController::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address I2CController::dataAddress() const noexcept {
    return m_baseAddress + DATA_OFFSET;
}

common::Address I2CController::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

common::Address I2CController::controlAddress() const noexcept {
    return m_baseAddress + CONTROL_OFFSET;
}

common::Address I2CController::slaveAddrAddress() const noexcept {
    return m_baseAddress + ADDR_OFFSET;
}

void I2CController::updateStatusRegister() {
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
    if (!m_lastAck && m_busy) {
        status |= STATUS_NACK_BIT;
    }
    m_statusRegister->write(status);
}

} // namespace efs::drivers::i2c
