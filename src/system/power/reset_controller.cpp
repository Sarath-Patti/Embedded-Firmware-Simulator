#include "system/power/reset_controller.hpp"
#include "common/logger.hpp"

namespace efs::system::power {

ResetController::ResetController(cpu::CPU* cpu,
                                 SystemBus* systemBus,
                                 firmware::FirmwareManager* firmwareManager) noexcept
    : m_cpu(cpu), m_systemBus(systemBus), m_firmwareManager(firmwareManager) {
}

void ResetController::setCPU(cpu::CPU* cpu) noexcept {
    m_cpu = cpu;
}

void ResetController::setSystemBus(SystemBus* systemBus) noexcept {
    m_systemBus = systemBus;
}

void ResetController::setFirmwareManager(firmware::FirmwareManager* firmwareManager) noexcept {
    m_firmwareManager = firmwareManager;
}

void ResetController::resetCPU() {
    common::Logger::info("ResetController: Resetting CPU");
    if (m_cpu != nullptr) {
        m_cpu->reset();
    }
}

void ResetController::resetPeripherals() {
    common::Logger::info("ResetController: Resetting Peripherals");
    if (m_systemBus != nullptr) {
        m_systemBus->reset();
    }
}

void ResetController::resetFirmware() {
    common::Logger::info("ResetController: Resetting Firmware");
    firmware::FirmwareManager* mgr = m_firmwareManager;
    if (mgr == nullptr && m_cpu != nullptr) {
        mgr = &m_cpu->firmwareManager();
    }
    if (mgr != nullptr) {
        mgr->reset();
        mgr->initialize();
    }
}

void ResetController::resetSystem() {
    common::Logger::info("ResetController: Performing Full System Reset");
    resetCPU();
    resetPeripherals();
    resetFirmware();
}

} // namespace efs::system::power
