#ifndef EFS_MMIO_MMIO_BUS_HPP
#define EFS_MMIO_MMIO_BUS_HPP

#include "common/types.hpp"
#include "mmio/register.hpp"
#include <memory>
#include <unordered_map>

namespace efs::mmio {

class MMIOBus {
public:
    MMIOBus() = default;
    ~MMIOBus() = default;

    MMIOBus(const MMIOBus&) = delete;
    MMIOBus& operator=(const MMIOBus&) = delete;
    MMIOBus(MMIOBus&&) noexcept = default;
    MMIOBus& operator=(MMIOBus&&) noexcept = default;

    bool registerRegister(std::shared_ptr<Register> reg);
    bool registerRegister(common::Address address, common::DWord initialValue = 0);
    bool unregisterRegister(common::Address address);

    [[nodiscard]] bool contains(common::Address address) const noexcept;
    [[nodiscard]] common::DWord read(common::Address address) const;
    void write(common::Address address, common::DWord value);

private:
    std::unordered_map<common::Address, std::shared_ptr<Register>> m_registers;
};

} // namespace efs::mmio

#endif // EFS_MMIO_MMIO_BUS_HPP
