#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::drivers::timer {

Timer::Timer(mmio::MMIOBus& bus, common::Address baseAddress)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_ctrlRegister(std::make_shared<mmio::Register>(baseAddress + CTRL_OFFSET, 0)),
      m_countRegister(std::make_shared<mmio::Register>(baseAddress + COUNT_OFFSET, 0)),
      m_compareRegister(std::make_shared<mmio::Register>(baseAddress + COMPARE_OFFSET, 0)),
      m_statusRegister(std::make_shared<mmio::Register>(baseAddress + STATUS_OFFSET, 0)) {
    if (!m_bus.registerRegister(m_ctrlRegister) ||
        !m_bus.registerRegister(m_countRegister) ||
        !m_bus.registerRegister(m_compareRegister) ||
        !m_bus.registerRegister(m_statusRegister)) {
        common::Logger::error("Timer failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("Timer MMIO registration failure");
    }
}

Timer::~Timer() {
    m_bus.unregisterRegister(ctrlAddress());
    m_bus.unregisterRegister(countAddress());
    m_bus.unregisterRegister(compareAddress());
    m_bus.unregisterRegister(statusAddress());
}

void Timer::start() {
    common::DWord ctrl = m_ctrlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_ctrlRegister->write(ctrl);
}

void Timer::stop() {
    common::DWord ctrl = m_ctrlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_ctrlRegister->write(ctrl);
}

void Timer::reset() {
    m_countRegister->write(0);
    m_statusRegister->write(0);
}

void Timer::attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId) {
    m_interruptController = controller;
    m_interruptId = interruptId;
}

void Timer::detachInterruptController() noexcept {
    m_interruptController = nullptr;
    m_interruptId = 0;
}

void Timer::tick() {
    if (!running()) {
        return;
    }

    common::DWord currentCount = m_countRegister->read();
    common::DWord targetCompare = m_compareRegister->read();

    currentCount++;
    m_countRegister->write(currentCount);

    if (currentCount >= targetCompare) {
        common::DWord status = m_statusRegister->read();
        status |= STATUS_MATCH_BIT;
        m_statusRegister->write(status);
        stop();
        if (m_interruptController != nullptr) {
            m_interruptController->trigger(m_interruptId);
        }
    }
}

void Timer::setCompare(common::DWord value) {
    m_compareRegister->write(value);
}

common::DWord Timer::compare() const noexcept {
    return m_compareRegister->read();
}

common::DWord Timer::counter() const noexcept {
    return m_countRegister->read();
}

bool Timer::running() const noexcept {
    return (m_ctrlRegister->read() & CTRL_ENABLE_BIT) != 0;
}

bool Timer::hasMatch() const noexcept {
    return (m_statusRegister->read() & STATUS_MATCH_BIT) != 0;
}

common::Address Timer::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address Timer::ctrlAddress() const noexcept {
    return m_baseAddress + CTRL_OFFSET;
}

common::Address Timer::countAddress() const noexcept {
    return m_baseAddress + COUNT_OFFSET;
}

common::Address Timer::compareAddress() const noexcept {
    return m_baseAddress + COMPARE_OFFSET;
}

common::Address Timer::statusAddress() const noexcept {
    return m_baseAddress + STATUS_OFFSET;
}

} // namespace efs::drivers::timer
