#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "system/clock/simulation_clock.hpp"
#include "system/scheduler/event_scheduler.hpp"
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
    cancelScheduledEvent();
    m_bus.unregisterRegister(ctrlAddress());
    m_bus.unregisterRegister(countAddress());
    m_bus.unregisterRegister(compareAddress());
    m_bus.unregisterRegister(statusAddress());
}

void Timer::start() {
    common::DWord ctrl = m_ctrlRegister->read();
    ctrl |= CTRL_ENABLE_BIT;
    m_ctrlRegister->write(ctrl);
    if (m_clock != nullptr) {
        m_lastClockCycles = m_clock->cycles();
    }
    updateScheduledEvent();
}

void Timer::stop() {
    common::DWord ctrl = m_ctrlRegister->read();
    ctrl &= ~CTRL_ENABLE_BIT;
    m_ctrlRegister->write(ctrl);
    cancelScheduledEvent();
}

void Timer::reset() {
    stop();
    m_countRegister->reset();
    m_statusRegister->reset();
    m_ctrlRegister->reset();
    if (m_clock != nullptr) {
        m_lastClockCycles = m_clock->cycles();
    }
}

void Timer::attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId) {
    m_interruptController = controller;
    m_interruptId = interruptId;
}

void Timer::detachInterruptController() noexcept {
    m_interruptController = nullptr;
    m_interruptId = 0;
}

void Timer::attachClock(system::clock::SimulationClock* clock) noexcept {
    m_clock = clock;
    if (m_clock != nullptr) {
        m_lastClockCycles = m_clock->cycles();
    }
}

void Timer::detachClock() noexcept {
    m_clock = nullptr;
    m_lastClockCycles = 0;
}

system::clock::SimulationClock* Timer::clock() const noexcept {
    return m_clock;
}

void Timer::attachScheduler(system::scheduler::EventScheduler* scheduler) noexcept {
    m_scheduler = scheduler;
    updateScheduledEvent();
}

void Timer::detachScheduler() noexcept {
    cancelScheduledEvent();
    m_scheduler = nullptr;
}

system::scheduler::EventScheduler* Timer::scheduler() const noexcept {
    return m_scheduler;
}

void Timer::updateScheduledEvent() {
    cancelScheduledEvent();

    if (!running() || m_scheduler == nullptr) {
        return;
    }

    common::DWord currentCount = counter();
    common::DWord targetCompare = compare();

    if (currentCount >= targetCompare) {
        return;
    }

    common::DWord delta = targetCompare - currentCount;
    common::QWord currentCycle = (m_clock != nullptr) ? m_clock->cycles() : 0;
    common::QWord targetCycle = currentCycle + delta;

    m_scheduledEventId = m_scheduler->schedule([this]() {
        handleCompareMatch();
    }, targetCycle, "Timer Compare Match");
}

void Timer::cancelScheduledEvent() {
    if (m_scheduledEventId != 0 && m_scheduler != nullptr) {
        m_scheduler->cancel(m_scheduledEventId);
        m_scheduledEventId = 0;
    }
}

void Timer::handleCompareMatch() {
    if (!running()) {
        return;
    }
    m_countRegister->write(compare());
    common::DWord status = m_statusRegister->read();
    status |= STATUS_MATCH_BIT;
    m_statusRegister->write(status);
    stop();
    if (m_interruptController != nullptr) {
        m_interruptController->trigger(m_interruptId);
    }
}

void Timer::tick() {
    if (!running()) {
        if (m_clock != nullptr) {
            m_lastClockCycles = m_clock->cycles();
        }
        return;
    }

    common::DWord stepAmount = 1;
    if (m_clock != nullptr) {
        common::QWord currentCycles = m_clock->cycles();
        if (currentCycles > m_lastClockCycles) {
            stepAmount = static_cast<common::DWord>(currentCycles - m_lastClockCycles);
        }
        m_lastClockCycles = currentCycles;
    }

    common::DWord currentCount = m_countRegister->read();
    common::DWord targetCompare = m_compareRegister->read();

    currentCount += stepAmount;
    m_countRegister->write(currentCount);

    if (m_scheduler == nullptr && currentCount >= targetCompare) {
        handleCompareMatch();
    }
}

void Timer::setCompare(common::DWord value) {
    m_compareRegister->write(value);
    updateScheduledEvent();
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

void Timer::clearMatch() {
    common::DWord status = m_statusRegister->read();
    status &= ~STATUS_MATCH_BIT;
    m_statusRegister->write(status);
}

void Timer::setInterruptEnabled(bool enable) {
    common::DWord ctrl = m_ctrlRegister->read();
    if (enable) {
        ctrl |= CTRL_INT_ENABLE_BIT;
    } else {
        ctrl &= ~CTRL_INT_ENABLE_BIT;
    }
    m_ctrlRegister->write(ctrl);
}

bool Timer::interruptEnabled() const noexcept {
    return (m_ctrlRegister->read() & CTRL_INT_ENABLE_BIT) != 0;
}

void Timer::setAutoReset(bool enable) {
    common::DWord ctrl = m_ctrlRegister->read();
    if (enable) {
        ctrl |= CTRL_AUTO_RESET_BIT;
    } else {
        ctrl &= ~CTRL_AUTO_RESET_BIT;
    }
    m_ctrlRegister->write(ctrl);
}

bool Timer::autoReset() const noexcept {
    return (m_ctrlRegister->read() & CTRL_AUTO_RESET_BIT) != 0;
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
