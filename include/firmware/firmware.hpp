#ifndef EFS_FIRMWARE_FIRMWARE_HPP
#define EFS_FIRMWARE_FIRMWARE_HPP

namespace efs::firmware {

class Firmware {
public:
    virtual ~Firmware() = default;

    virtual void initialize() = 0;
    virtual void execute() = 0;
    virtual void shutdown() = 0;
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_FIRMWARE_HPP
