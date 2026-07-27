#ifndef EFS_MMIO_REGISTER_HPP
#define EFS_MMIO_REGISTER_HPP

#include "common/types.hpp"

namespace efs::mmio {

class Register {
public:
    explicit Register(common::Address address, common::DWord initialValue = 0);
    ~Register() = default;

    Register(const Register&) = delete;
    Register& operator=(const Register&) = delete;
    Register(Register&&) noexcept = default;
    Register& operator=(Register&&) noexcept = default;

    [[nodiscard]] common::Address address() const noexcept;
    [[nodiscard]] common::DWord read() const noexcept;
    void write(common::DWord value) noexcept;

private:
    common::Address m_address;
    common::DWord m_value;
};

} // namespace efs::mmio

#endif // EFS_MMIO_REGISTER_HPP
