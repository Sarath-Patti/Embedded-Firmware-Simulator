#include "firmware/firmware_manager.hpp"
#include "common/logger.hpp"

namespace efs::firmware {

FirmwareManager::~FirmwareManager() {
    shutdown();
}

bool FirmwareManager::registerFirmware(const std::string& name, std::shared_ptr<Firmware> firmware) {
    if (name.empty() || firmware == nullptr) {
        common::Logger::warning("FirmwareManager: Attempted to register null or empty-named firmware.");
        return false;
    }
    if (m_firmwareRegistry.find(name) != m_firmwareRegistry.end()) {
        common::Logger::warning("FirmwareManager: Firmware with name '" + name + "' already registered.");
        return false;
    }

    m_firmwareRegistry[name] = std::move(firmware);

    // Auto-select first registered firmware if active selection is empty
    if (m_activeName.empty()) {
        m_activeName = name;
    }
    return true;
}

bool FirmwareManager::unregisterFirmware(const std::string& name) {
    auto it = m_firmwareRegistry.find(name);
    if (it == m_firmwareRegistry.end()) {
        common::Logger::warning("FirmwareManager: Cannot unregister unknown firmware '" + name + "'.");
        return false;
    }

    if (m_activeName == name) {
        if (it->second != nullptr) {
            it->second->shutdown();
        }
        m_activeName.clear();
    }

    m_firmwareRegistry.erase(it);

    // If active became empty but registry still has elements, select first remaining
    if (m_activeName.empty() && !m_firmwareRegistry.empty()) {
        m_activeName = m_firmwareRegistry.begin()->first;
    }

    return true;
}

bool FirmwareManager::setActiveFirmware(const std::string& name) {
    auto it = m_firmwareRegistry.find(name);
    if (it == m_firmwareRegistry.end()) {
        common::Logger::warning("FirmwareManager: Cannot set active unknown firmware '" + name + "'.");
        return false;
    }

    if (m_activeName == name) {
        return true;
    }

    // Shutdown previous active firmware
    shutdown();

    m_activeName = name;
    return true;
}

std::shared_ptr<Firmware> FirmwareManager::activeFirmware() const noexcept {
    auto it = m_firmwareRegistry.find(m_activeName);
    if (it != m_firmwareRegistry.end()) {
        return it->second;
    }
    return nullptr;
}

std::string FirmwareManager::activeFirmwareName() const noexcept {
    return m_activeName;
}

bool FirmwareManager::hasFirmware(const std::string& name) const noexcept {
    return m_firmwareRegistry.find(name) != m_firmwareRegistry.end();
}

std::size_t FirmwareManager::count() const noexcept {
    return m_firmwareRegistry.size();
}

std::vector<std::string> FirmwareManager::registeredNames() const {
    std::vector<std::string> names;
    names.reserve(m_firmwareRegistry.size());
    for (const auto& [name, fw] : m_firmwareRegistry) {
        names.push_back(name);
    }
    return names;
}

void FirmwareManager::initialize() {
    auto active = activeFirmware();
    if (active != nullptr) {
        active->initialize();
    }
}

void FirmwareManager::update() {
    auto active = activeFirmware();
    if (active != nullptr) {
        active->update();
    }
}

void FirmwareManager::shutdown() {
    auto active = activeFirmware();
    if (active != nullptr) {
        active->shutdown();
    }
}

void FirmwareManager::reset() {
    auto active = activeFirmware();
    if (active != nullptr) {
        active->reset();
    }
}

} // namespace efs::firmware
