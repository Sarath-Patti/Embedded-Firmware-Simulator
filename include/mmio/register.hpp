#ifndef EFS_MMIO_REGISTER_HPP
#define EFS_MMIO_REGISTER_HPP

#include "common/types.hpp"

namespace efs::mmio {

/// Represents a single 32-bit Memory-Mapped I/O (MMIO) register.
class Register {
public:
    explicit Register(common::Address address, common::DWord initialValue = 0);
    ~Register() = default;

    Register(const Register&) = delete;
    Register& operator=(const Register&) = delete;
    Register(Register&&) noexcept = default;
    Register& operator=(Register&&) noexcept = default;

    /// Returns the assigned MMIO base address of the register.
    [[nodiscard]] common::Address address() const noexcept;

    /// Reads current 32-bit register value.
    [[nodiscard]] common::DWord read() const noexcept;

    /// Writes a new 32-bit value to the register.
    void write(common::DWord value) noexcept;

private:
    common::Address m_address;
    common::DWord m_value;
};

} // namespace efs::mmio

#endif // EFS_MMIO_REGISTER_HPP
