#include "cpu/cpu.hpp"
#include "common/logger.hpp"
#include <algorithm>

namespace efs::cpu {

CPU::CPU(system::SystemBus* systemBus)
    : m_systemBus(systemBus) {
    if (m_systemBus == nullptr) {
        m_ownedSystemBus = std::make_unique<system::SystemBus>();
        m_systemBus = m_ownedSystemBus.get();
    }
}

CPU::CPU(kernel::InterruptController* interruptController) {
    m_ownedSystemBus = std::make_unique<system::SystemBus>(nullptr, nullptr, interruptController);
    m_systemBus = m_ownedSystemBus.get();
}

CPU::~CPU() {
    stop();
}

system::power::PowerController& CPU::powerController() noexcept {
    return *m_powerController;
}

const system::power::PowerController& CPU::powerController() const noexcept {
    return *m_powerController;
}

void CPU::setPowerController(system::power::PowerController* powerController) noexcept {
    if (powerController != nullptr) {
        m_powerController = powerController;
    } else {
        m_powerController = &m_ownedPowerController;
    }
}

firmware::FirmwareManager& CPU::firmwareManager() noexcept {
    return m_firmwareManager;
}

const firmware::FirmwareManager& CPU::firmwareManager() const noexcept {
    return m_firmwareManager;
}

void CPU::setSystemBus(system::SystemBus* systemBus) noexcept {
    m_systemBus = systemBus;
    m_ownedSystemBus.reset();
}

system::SystemBus* CPU::systemBus() const noexcept {
    return m_systemBus;
}

void CPU::attachRTOSScheduler(rtos::Scheduler* scheduler) noexcept {
    m_rtosScheduler = scheduler;
}

rtos::Scheduler* CPU::rtosScheduler() const noexcept {
    return m_rtosScheduler;
}

void CPU::start() {
    if (!m_running) {
        m_running = true;
        m_firmwareManager.initialize();
        if (m_rtosScheduler != nullptr) {
            m_rtosScheduler->start();
        }
    }
}

void CPU::stop() {
    if (m_running) {
        m_running = false;
        m_firmwareManager.shutdown();
        if (m_rtosScheduler != nullptr) {
            m_rtosScheduler->stop();
        }
    }
}

void CPU::reset() {
    m_cycleCount = 0;
    m_registerFile.reset();
    m_firmwareManager.reset();
    if (m_rtosScheduler != nullptr) {
        m_rtosScheduler->reset();
    }
    if (m_systemBus != nullptr) {
        m_systemBus->reset();
    }
}

const registers::RegisterFile& CPU::registerFile() const noexcept {
    return m_registerFile;
}

registers::RegisterFile& CPU::registerFile() noexcept {
    return m_registerFile;
}

common::QWord CPU::cycleCount() const noexcept {
    return m_cycleCount;
}

void CPU::step() {
    // Instruction execution is restricted while power state is OFF or SLEEP
    if (!m_powerController->isPowerOn()) {
        return;
    }

    m_cycleCount++;

    if (m_systemBus != nullptr) {
        m_systemBus->tickTimers();
        m_systemBus->scheduler().executeReadyEvents(m_systemBus->clock().cycles());
        if (m_systemBus->interrupts() != nullptr) {
            m_systemBus->interrupts()->dispatch();
        }
    }

    if (m_rtosScheduler != nullptr && m_rtosScheduler->running()) {
        m_rtosScheduler->tick();
        m_rtosScheduler->schedule();
    } else if (m_running) {
        m_firmwareManager.update();
    }
}

void CPU::run(common::QWord cycles) {
    start();
    for (common::QWord i = 0; i < cycles && m_running; ++i) {
        if (!m_powerController->isPowerOn()) {
            break;
        }
        step();
    }
}

bool CPU::loadFirmware(std::shared_ptr<firmware::Firmware> firmware, const std::string& name) {
    if (!firmware) {
        common::Logger::warning("Attempted to load null firmware into CPU");
        return false;
    }
    if (m_running) {
        stop();
    }
    if (!m_firmwareManager.registerFirmware(name, firmware)) {
        m_firmwareManager.setActiveFirmware(name);
    } else {
        m_firmwareManager.setActiveFirmware(name);
    }
    return true;
}

void CPU::unloadFirmware() {
    if (m_running) {
        stop();
    }
    std::string activeName = m_firmwareManager.activeFirmwareName();
    if (!activeName.empty()) {
        m_firmwareManager.unregisterFirmware(activeName);
    }
}

bool CPU::firmwareLoaded() const noexcept {
    return m_firmwareManager.activeFirmware() != nullptr;
}

std::shared_ptr<firmware::Firmware> CPU::firmware() const noexcept {
    return m_firmwareManager.activeFirmware();
}

bool CPU::attachDMA(drivers::dma::DMAController* dma) {
    return m_systemBus != nullptr && m_systemBus->attachDMA(dma);
}

bool CPU::detachDMA(drivers::dma::DMAController* dma) {
    return m_systemBus != nullptr && m_systemBus->detachDMA(dma);
}

drivers::dma::DMAController* CPU::dmaController() const noexcept {
    return m_systemBus != nullptr ? m_systemBus->dma() : nullptr;
}

bool CPU::attachTimer(drivers::timer::Timer* timer) {
    return m_systemBus != nullptr && m_systemBus->attachTimer(timer);
}

bool CPU::detachTimer(drivers::timer::Timer* timer) {
    return m_systemBus != nullptr && m_systemBus->detachTimer(timer);
}

void CPU::setInterruptController(kernel::InterruptController* controller) {
    if (m_systemBus != nullptr) {
        m_systemBus->setInterrupts(controller);
    }
}

kernel::InterruptController* CPU::interruptController() const noexcept {
    return m_systemBus != nullptr ? m_systemBus->interrupts() : nullptr;
}

} // namespace efs::cpu
