#include "system/system_bus.hpp"
#include "common/logger.hpp"
#include <algorithm>

namespace efs::system {

SystemBus::SystemBus(memory::Memory* memory,
                     mmio::MMIOBus* mmioBus,
                     kernel::InterruptController* interruptController)
    : m_memory(memory),
      m_mmioBus(mmioBus),
      m_interruptController(interruptController) {
}

clock::SimulationClock& SystemBus::clock() noexcept {
    return m_clock;
}

const clock::SimulationClock& SystemBus::clock() const noexcept {
    return m_clock;
}

void SystemBus::setMemory(memory::Memory* memory) noexcept {
    m_memory = memory;
}

memory::Memory* SystemBus::memory() const noexcept {
    return m_memory;
}

void SystemBus::setMMIO(mmio::MMIOBus* mmioBus) noexcept {
    m_mmioBus = mmioBus;
}

mmio::MMIOBus* SystemBus::mmio() const noexcept {
    return m_mmioBus;
}

void SystemBus::setInterrupts(kernel::InterruptController* interruptController) noexcept {
    m_interruptController = interruptController;
}

kernel::InterruptController* SystemBus::interrupts() const noexcept {
    return m_interruptController;
}

bool SystemBus::attachTimer(drivers::timer::Timer* timer) {
    if (timer == nullptr) {
        common::Logger::warning("SystemBus attempt to attach null timer");
        return false;
    }
    auto it = std::find(m_timers.begin(), m_timers.end(), timer);
    if (it != m_timers.end()) {
        common::Logger::warning("Timer already attached to SystemBus");
        return false;
    }
    timer->attachClock(&m_clock);
    m_timers.push_back(timer);
    return true;
}

bool SystemBus::detachTimer(drivers::timer::Timer* timer) {
    if (timer == nullptr) {
        return false;
    }
    auto it = std::find(m_timers.begin(), m_timers.end(), timer);
    if (it == m_timers.end()) {
        common::Logger::warning("Attempted to detach unattached timer from SystemBus");
        return false;
    }
    timer->detachClock();
    m_timers.erase(it);
    return true;
}

const std::vector<drivers::timer::Timer*>& SystemBus::timers() const noexcept {
    return m_timers;
}

void SystemBus::tickTimers() {
    m_clock.tick();
    for (auto* timer : m_timers) {
        if (timer != nullptr) {
            timer->tick();
        }
    }
}

void SystemBus::attachGPIO(drivers::gpio::GPIO* gpio) noexcept {
    m_gpio = gpio;
}

drivers::gpio::GPIO* SystemBus::gpio() const noexcept {
    return m_gpio;
}

void SystemBus::attachUART(drivers::uart::UART* uart) noexcept {
    m_uart = uart;
}

drivers::uart::UART* SystemBus::uart() const noexcept {
    return m_uart;
}

} // namespace efs::system
