#include "cpu/cpu.hpp"
#include "common/logger.hpp"
#include <algorithm>

namespace efs::cpu {

CPU::CPU(kernel::InterruptController* interruptController)
    : m_interruptController(interruptController) {
}

void CPU::start() {
    m_running = true;
}

void CPU::stop() {
    m_running = false;
}

void CPU::reset() {
    m_cycleCount = 0;
}

void CPU::step() {
    m_cycleCount++;

    for (auto* timer : m_timers) {
        if (timer != nullptr) {
            timer->tick();
        }
    }

    if (m_interruptController != nullptr) {
        m_interruptController->dispatch();
    }
}

void CPU::run(common::QWord cycles) {
    start();
    for (common::QWord i = 0; i < cycles && m_running; ++i) {
        step();
    }
}

bool CPU::attachTimer(drivers::timer::Timer* timer) {
    if (timer == nullptr) {
        common::Logger::warning("CPU attempt to attach null timer");
        return false;
    }
    auto it = std::find(m_timers.begin(), m_timers.end(), timer);
    if (it != m_timers.end()) {
        common::Logger::warning("Timer already attached to CPU");
        return false;
    }
    m_timers.push_back(timer);
    return true;
}

bool CPU::detachTimer(drivers::timer::Timer* timer) {
    if (timer == nullptr) {
        return false;
    }
    auto it = std::find(m_timers.begin(), m_timers.end(), timer);
    if (it == m_timers.end()) {
        common::Logger::warning("Attempted to detach unattached timer from CPU");
        return false;
    }
    m_timers.erase(it);
    return true;
}

void CPU::setInterruptController(kernel::InterruptController* controller) {
    m_interruptController = controller;
}

kernel::InterruptController* CPU::interruptController() const noexcept {
    return m_interruptController;
}

common::QWord CPU::cycleCount() const noexcept {
    return m_cycleCount;
}

bool CPU::running() const noexcept {
    return m_running;
}

} // namespace efs::cpu
