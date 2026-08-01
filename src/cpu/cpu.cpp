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

void CPU::start() {
    if (!m_running) {
        m_running = true;
        m_firmwareManager.initialize();
    }
}

void CPU::stop() {
    if (m_running) {
        m_running = false;
        m_firmwareManager.shutdown();
    }
}

void CPU::reset() {
    m_cycleCount = 0;
    m_registerFile.reset();
    m_firmwareManager.reset();
}

const registers::RegisterFile& CPU::registerFile() const noexcept {
    return m_registerFile;
}

registers::RegisterFile& CPU::registerFile() noexcept {
    return m_registerFile;
}

void CPU::step() {
    m_cycleCount++;

    if (m_systemBus != nullptr) {
        m_systemBus->tickTimers();
        m_systemBus->scheduler().executeReadyEvents(m_systemBus->clock().cycles());
        if (m_systemBus->interrupts() != nullptr) {
            m_systemBus->interrupts()->dispatch();
        }
    }

    if (m_running) {
        m_firmwareManager.update();
    }
}

void CPU::run(common::QWord cycles) {
    start();
    for (common::QWord i = 0; i < cycles && m_running; ++i) {
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
        // If registration failed because name exists, update reference
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

bool CPU::attachTimer(drivers::timer::Timer* timer) {
    if (m_systemBus == nullptr) {
        return false;
    }
    return m_systemBus->attachTimer(timer);
}

bool CPU::detachTimer(drivers::timer::Timer* timer) {
    if (m_systemBus == nullptr) {
        return false;
    }
    return m_systemBus->detachTimer(timer);
}

void CPU::setInterruptController(kernel::InterruptController* controller) {
    if (m_systemBus != nullptr) {
        m_systemBus->setInterrupts(controller);
    }
}

kernel::InterruptController* CPU::interruptController() const noexcept {
    return m_systemBus ? m_systemBus->interrupts() : nullptr;
}

common::QWord CPU::cycleCount() const noexcept {
    return m_cycleCount;
}

bool CPU::running() const noexcept {
    return m_running;
}

} // namespace efs::cpu
