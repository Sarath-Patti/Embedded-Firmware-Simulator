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

SystemBus::~SystemBus() {
    for (auto* timer : m_timers) {
        if (timer != nullptr) {
            timer->detachClock();
            timer->detachScheduler();
        }
    }
    m_timers.clear();
    m_dmas.clear();
}

clock::SimulationClock& SystemBus::clock() noexcept {
    return m_clock;
}

const clock::SimulationClock& SystemBus::clock() const noexcept {
    return m_clock;
}

scheduler::EventScheduler& SystemBus::scheduler() noexcept {
    return m_scheduler;
}

const scheduler::EventScheduler& SystemBus::scheduler() const noexcept {
    return m_scheduler;
}

void SystemBus::setMemory(memory::Memory* memory) noexcept {
    m_memory = memory;
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->attachMemory(memory);
        }
    }
}

memory::Memory* SystemBus::memory() const noexcept {
    return m_memory;
}

void SystemBus::setMMIO(mmio::MMIOBus* mmioBus) noexcept {
    m_mmioBus = mmioBus;
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->attachMMIO(mmioBus);
        }
    }
}

mmio::MMIOBus* SystemBus::mmio() const noexcept {
    return m_mmioBus;
}

void SystemBus::setInterrupts(kernel::InterruptController* interruptController) noexcept {
    m_interruptController = interruptController;
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->attachInterruptController(interruptController, dma->interruptId());
        }
    }
}

kernel::InterruptController* SystemBus::interrupts() const noexcept {
    return m_interruptController;
}

bool SystemBus::attachDMA(drivers::dma::DMAController* dma) {
    if (dma == nullptr) {
        common::Logger::warning("SystemBus attempt to attach null DMA controller");
        return false;
    }
    auto it = std::find(m_dmas.begin(), m_dmas.end(), dma);
    if (it != m_dmas.end()) {
        common::Logger::warning("DMA controller already attached to SystemBus");
        return false;
    }
    if (m_memory != nullptr) {
        dma->attachMemory(m_memory);
    }
    if (m_mmioBus != nullptr) {
        dma->attachMMIO(m_mmioBus);
    }
    if (m_interruptController != nullptr) {
        dma->attachInterruptController(m_interruptController, dma->interruptId());
    }
    if (m_uart != nullptr) {
        dma->attachUART(m_uart);
    }
    dma->attachScheduler(&m_scheduler);
    m_dmas.push_back(dma);
    return true;
}

bool SystemBus::detachDMA(drivers::dma::DMAController* dma) {
    if (dma == nullptr) {
        return false;
    }
    auto it = std::find(m_dmas.begin(), m_dmas.end(), dma);
    if (it == m_dmas.end()) {
        common::Logger::warning("Attempted to detach unattached DMA controller from SystemBus");
        return false;
    }
    dma->detachScheduler();
    m_dmas.erase(it);
    return true;
}

drivers::dma::DMAController* SystemBus::dma() const noexcept {
    return m_dmas.empty() ? nullptr : m_dmas.front();
}

const std::vector<drivers::dma::DMAController*>& SystemBus::dmas() const noexcept {
    return m_dmas;
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
    timer->attachScheduler(&m_scheduler);
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
    timer->detachScheduler();
    m_timers.erase(it);
    return true;
}

const std::vector<drivers::timer::Timer*>& SystemBus::timers() const noexcept {
    return m_timers;
}

void SystemBus::tickTimers() {
    m_clock.tick();
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->tick();
        }
    }
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
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->attachUART(uart);
        }
    }
}

drivers::uart::UART* SystemBus::uart() const noexcept {
    return m_uart;
}

void SystemBus::reset() {
    for (auto* dma : m_dmas) {
        if (dma != nullptr) {
            dma->reset();
        }
    }
    if (m_gpio != nullptr) {
        m_gpio->reset();
    }
    for (auto* timer : m_timers) {
        if (timer != nullptr) {
            timer->reset();
        }
    }
    if (m_uart != nullptr) {
        m_uart->reset();
    }
    if (m_interruptController != nullptr) {
        m_interruptController->reset();
    }
    if (m_memory != nullptr) {
        m_memory->clear();
    }
    m_clock.reset();
    m_scheduler.clear();
}

} // namespace efs::system
