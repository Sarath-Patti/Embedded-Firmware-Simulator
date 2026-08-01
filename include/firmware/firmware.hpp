#ifndef EFS_FIRMWARE_FIRMWARE_HPP
#define EFS_FIRMWARE_FIRMWARE_HPP

namespace efs::firmware {

/// Abstract base class defining the firmware application lifecycle interface.
/// Firmware applications must communicate exclusively through the Hardware Abstraction Layer (HAL).
class Firmware {
public:
    virtual ~Firmware();

    /// Invoked once when the firmware application is initialized.
    virtual void initialize() = 0;

    /// Invoked on every simulation cycle step to perform firmware logic.
    virtual void update() = 0;

    /// Invoked when the firmware application is halted or shut down.
    virtual void shutdown() = 0;

    /// Resets internal firmware application state to initial pre-execution state.
    virtual void reset() = 0;

    /// Legacy simulation step entrypoint (forwards to update() by default).
    virtual void execute();
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_FIRMWARE_HPP
