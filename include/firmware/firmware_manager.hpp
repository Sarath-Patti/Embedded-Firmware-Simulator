#ifndef EFS_FIRMWARE_FIRMWARE_MANAGER_HPP
#define EFS_FIRMWARE_FIRMWARE_MANAGER_HPP

#include "firmware/firmware.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace efs::firmware {

/// Central FirmwareManager component registering, lifecycle-controlling, and switching firmware applications.
class FirmwareManager {
public:
    FirmwareManager() = default;
    ~FirmwareManager();

    FirmwareManager(const FirmwareManager&) = delete;
    FirmwareManager& operator=(const FirmwareManager&) = delete;
    FirmwareManager(FirmwareManager&&) noexcept = default;
    FirmwareManager& operator=(FirmwareManager&&) noexcept = default;

    /// Registers a firmware application under a unique name.
    bool registerFirmware(const std::string& name, std::shared_ptr<Firmware> firmware);

    /// Unregisters a firmware application by name. Shuts down if active.
    bool unregisterFirmware(const std::string& name);

    /// Sets the active firmware application by name. Shuts down previous active firmware first.
    bool setActiveFirmware(const std::string& name);

    /// Returns pointer to currently active firmware or nullptr.
    [[nodiscard]] std::shared_ptr<Firmware> activeFirmware() const noexcept;

    /// Returns the name of the currently active firmware, or empty string.
    [[nodiscard]] std::string activeFirmwareName() const noexcept;

    /// Returns true if a firmware with the given name is registered.
    [[nodiscard]] bool hasFirmware(const std::string& name) const noexcept;

    /// Returns total number of registered firmware applications.
    [[nodiscard]] std::size_t count() const noexcept;

    /// Returns sorted list of all registered firmware names.
    [[nodiscard]] std::vector<std::string> registeredNames() const;

    /// Initializes active firmware application.
    void initialize();

    /// Advances active firmware simulation step logic.
    void update();

    /// Halts and shuts down active firmware application.
    void shutdown();

    /// Resets active firmware application state.
    void reset();

private:
    std::map<std::string, std::shared_ptr<Firmware>> m_firmwareRegistry;
    std::string m_activeName;
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_FIRMWARE_MANAGER_HPP
