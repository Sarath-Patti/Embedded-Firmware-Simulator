#include "kernel/interrupt_controller.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::kernel {

InterruptController::InterruptController(mmio::MMIOBus& bus, common::Address baseAddress)
    : m_bus(bus),
      m_baseAddress(baseAddress),
      m_enableRegister(std::make_shared<mmio::Register>(baseAddress + ENABLE_OFFSET, 0)),
      m_pendingRegister(std::make_shared<mmio::Register>(baseAddress + PENDING_OFFSET, 0)),
      m_priorityRegister(std::make_shared<mmio::Register>(baseAddress + PRIORITY_OFFSET, 0)) {
    m_registered.fill(false);
    m_priorities.fill(0);
    m_handlers.fill(nullptr);

    if (!m_bus.registerRegister(m_enableRegister) ||
        !m_bus.registerRegister(m_pendingRegister) ||
        !m_bus.registerRegister(m_priorityRegister)) {
        common::Logger::error("InterruptController failed to register MMIO registers at base address " + std::to_string(baseAddress));
        throw std::runtime_error("InterruptController MMIO registration failure");
    }
}

InterruptController::~InterruptController() {
    m_bus.unregisterRegister(enableAddress());
    m_bus.unregisterRegister(pendingAddress());
    m_bus.unregisterRegister(priorityAddress());
}

bool InterruptController::registerInterrupt(std::uint8_t id) {
    validateInterruptId(id);
    if (m_registered[id]) {
        common::Logger::warning("Interrupt ID already registered: " + std::to_string(id));
        return false;
    }
    m_registered[id] = true;
    m_priorities[id] = 0;
    m_handlers[id] = nullptr;
    return true;
}

bool InterruptController::unregisterInterrupt(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        return false;
    }
    disable(id);
    clear(id);
    m_registered[id] = false;
    m_handlers[id] = nullptr;
    return true;
}

void InterruptController::enable(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot enable unregistered interrupt ID: " + std::to_string(id));
        return;
    }
    common::DWord enableReg = m_enableRegister->read();
    enableReg |= (1U << id);
    m_enableRegister->write(enableReg);
}

void InterruptController::disable(std::uint8_t id) {
    validateInterruptId(id);
    common::DWord enableReg = m_enableRegister->read();
    enableReg &= ~(1U << id);
    m_enableRegister->write(enableReg);
}

void InterruptController::trigger(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot trigger unregistered interrupt ID: " + std::to_string(id));
        return;
    }
    common::DWord pendingReg = m_pendingRegister->read();
    pendingReg |= (1U << id);
    m_pendingRegister->write(pendingReg);
}

void InterruptController::clear(std::uint8_t id) {
    validateInterruptId(id);
    common::DWord pendingReg = m_pendingRegister->read();
    pendingReg &= ~(1U << id);
    m_pendingRegister->write(pendingReg);
}

bool InterruptController::pending(std::uint8_t id) const {
    validateInterruptId(id);
    common::DWord pendingReg = m_pendingRegister->read();
    return (pendingReg & (1U << id)) != 0;
}

bool InterruptController::enabled(std::uint8_t id) const {
    validateInterruptId(id);
    common::DWord enableReg = m_enableRegister->read();
    return (enableReg & (1U << id)) != 0;
}

bool InterruptController::isRegistered(std::uint8_t id) const noexcept {
    if (id >= MAX_INTERRUPTS) {
        return false;
    }
    return m_registered[id];
}

void InterruptController::setPriority(std::uint8_t id, std::uint8_t priority) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot set priority for unregistered interrupt ID: " + std::to_string(id));
        return;
    }
    m_priorities[id] = priority;
}

std::uint8_t InterruptController::getPriority(std::uint8_t id) const {
    validateInterruptId(id);
    return m_priorities[id];
}

bool InterruptController::registerHandler(std::uint8_t id, ISRHandler callback) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot register handler for unregistered interrupt ID: " + std::to_string(id));
        return false;
    }
    m_handlers[id] = std::move(callback);
    return true;
}

bool InterruptController::unregisterHandler(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        return false;
    }
    m_handlers[id] = nullptr;
    return true;
}

bool InterruptController::dispatch() {
    common::DWord enableReg = m_enableRegister->read();
    common::DWord pendingReg = m_pendingRegister->read();
    common::DWord active = enableReg & pendingReg;

    if (active == 0) {
        return false;
    }

    int highestPriorityId = -1;
    std::uint8_t highestPriorityVal = 255;

    for (std::uint8_t i = 0; i < MAX_INTERRUPTS; ++i) {
        if ((active & (1U << i)) != 0) {
            std::uint8_t priority = m_priorities[i];
            if (highestPriorityId == -1 || priority < highestPriorityVal) {
                highestPriorityId = i;
                highestPriorityVal = priority;
            }
        }
    }

    if (highestPriorityId != -1) {
        auto targetId = static_cast<std::uint8_t>(highestPriorityId);
        clear(targetId);
        if (m_handlers[targetId] != nullptr) {
            m_handlers[targetId]();
        }
        return true;
    }

    return false;
}

void InterruptController::reset() {
    m_enableRegister->reset();
    m_pendingRegister->reset();
    m_priorityRegister->reset();
}

common::Address InterruptController::baseAddress() const noexcept {
    return m_baseAddress;
}

common::Address InterruptController::enableAddress() const noexcept {
    return m_baseAddress + ENABLE_OFFSET;
}

common::Address InterruptController::pendingAddress() const noexcept {
    return m_baseAddress + PENDING_OFFSET;
}

common::Address InterruptController::priorityAddress() const noexcept {
    return m_baseAddress + PRIORITY_OFFSET;
}

void InterruptController::validateInterruptId(std::uint8_t id) const {
    if (id >= MAX_INTERRUPTS) {
        common::Logger::error("Interrupt ID out of range: " + std::to_string(id));
        throw std::out_of_range("Interrupt ID out of range: " + std::to_string(id));
    }
}

} // namespace efs::kernel
