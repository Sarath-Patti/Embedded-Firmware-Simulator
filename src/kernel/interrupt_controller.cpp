#include "kernel/interrupt_controller.hpp"
#include "common/logger.hpp"
#include <algorithm>
#include <limits>
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
    m_priorities.fill(128); // Default priority value
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
    m_priorities[id] = 128;
    return true;
}

bool InterruptController::unregisterInterrupt(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Attempted to unregister unregistered interrupt ID: " + std::to_string(id));
        return false;
    }
    disable(id);
    clear(id);
    m_handlers[id] = nullptr;
    m_registered[id] = false;
    return true;
}

void InterruptController::enable(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot enable unregistered interrupt ID: " + std::to_string(id));
        return;
    }
    common::DWord en = m_enableRegister->read();
    en |= (1U << id);
    m_enableRegister->write(en);
}

void InterruptController::disable(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        return;
    }
    common::DWord en = m_enableRegister->read();
    en &= ~(1U << id);
    m_enableRegister->write(en);
}

void InterruptController::trigger(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        common::Logger::warning("Cannot trigger unregistered interrupt ID: " + std::to_string(id));
        return;
    }
    common::DWord pend = m_pendingRegister->read();
    pend |= (1U << id);
    m_pendingRegister->write(pend);
}

void InterruptController::clear(std::uint8_t id) {
    validateInterruptId(id);
    if (!m_registered[id]) {
        return;
    }
    common::DWord pend = m_pendingRegister->read();
    pend &= ~(1U << id);
    m_pendingRegister->write(pend);
}

bool InterruptController::pending(std::uint8_t id) const {
    validateInterruptId(id);
    return (m_pendingRegister->read() & (1U << id)) != 0;
}

bool InterruptController::enabled(std::uint8_t id) const {
    validateInterruptId(id);
    return (m_enableRegister->read() & (1U << id)) != 0;
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
    int bestId = -1;
    std::uint8_t highestPriority = std::numeric_limits<std::uint8_t>::max();

    for (std::uint8_t id = 0; id < MAX_INTERRUPTS; ++id) {
        if (m_registered[id] && enabled(id) && pending(id) && m_handlers[id] != nullptr) {
            std::uint8_t prio = m_priorities[id];
            // Lower priority numerical value indicates higher execution priority
            if (bestId == -1 || prio < highestPriority) {
                bestId = static_cast<int>(id);
                highestPriority = prio;
            }
        }
    }

    if (bestId == -1) {
        return false;
    }

    auto targetId = static_cast<std::uint8_t>(bestId);
    clear(targetId);

    if (m_handlers[targetId]) {
        m_handlers[targetId]();
    }

    return true;
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
