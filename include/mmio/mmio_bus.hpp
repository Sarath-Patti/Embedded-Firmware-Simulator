#ifndef EFS_MMIO_MMIO_BUS_HPP
#define EFS_MMIO_MMIO_BUS_HPP

#include "common/types.hpp"
#include "mmio/register.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace efs::mmio {

/// Central Memory-Mapped I/O (MMIO) bus managing address decoding and register access.
class MMIOBus {
public:
    MMIOBus() = default;
    ~MMIOBus() = default;

    MMIOBus(const MMIOBus&) = delete;
    MMIOBus& operator=(const MMIOBus&) = delete;
    MMIOBus(MMIOBus&&) noexcept = default;
    MMIOBus& operator=(MMIOBus&&) noexcept = default;

    /// Registers a shared MMIO register instance on the bus. Throws std::invalid_argument if null or address exists.
    bool registerRegister(std::shared_ptr<Register> reg);

    /// Constructs and registers an MMIO register at specified address. Returns false if address exists.
    bool registerRegister(common::Address address, common::DWord initialValue = 0);

    /// Unregisters an MMIO register at specified address. Returns true if found and removed.
    bool unregisterRegister(common::Address address);

    /// Returns true if an MMIO register is mapped to specified address.
    [[nodiscard]] bool contains(common::Address address) const noexcept;

    /// Reads 32-bit value from MMIO register at address. Throws std::out_of_range if unmapped.
    [[nodiscard]] common::DWord read(common::Address address) const;

    /// Writes 32-bit value to MMIO register at address. Throws std::out_of_range if unmapped.
    void write(common::Address address, common::DWord value);

    /// Returns a list of all currently mapped MMIO addresses.
    [[nodiscard]] std::vector<common::Address> registeredAddresses() const;

private:
    std::unordered_map<common::Address, std::shared_ptr<Register>> m_registers;
};

} // namespace efs::mmio

#endif // EFS_MMIO_MMIO_BUS_HPP
