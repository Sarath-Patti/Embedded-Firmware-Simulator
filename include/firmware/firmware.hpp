#ifndef EFS_FIRMWARE_FIRMWARE_HPP
#define EFS_FIRMWARE_FIRMWARE_HPP

namespace efs::firmware {

/// Abstract base class defining the firmware application lifecycle interface.
class Firmware {
public:
    virtual ~Firmware() = default;

    /// Invoked once when the CPU starts execution.
    virtual void initialize() = 0;

    /// Invoked on every CPU simulation step while running.
    virtual void execute() = 0;

    /// Invoked when the CPU halts execution.
    virtual void shutdown() = 0;
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_FIRMWARE_HPP
