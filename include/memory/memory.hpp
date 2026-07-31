#ifndef EFS_MEMORY_MEMORY_HPP
#define EFS_MEMORY_MEMORY_HPP

#include "common/types.hpp"
#include <vector>

namespace efs::memory {

/// Represents a contiguous, byte-addressable memory buffer with bounds checking.
class Memory {
public:
    explicit Memory(common::Size size);
    ~Memory() = default;

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    Memory(Memory&&) noexcept = default;
    Memory& operator=(Memory&&) noexcept = default;

    /// Reads a byte from the specified address. Throws std::out_of_range if out of bounds.
    [[nodiscard]] common::Byte read(common::Address address) const;

    /// Writes a byte to the specified address. Throws std::out_of_range if out of bounds.
    void write(common::Address address, common::Byte value);

    /// Returns the total size of memory in bytes.
    [[nodiscard]] common::Size size() const noexcept;

    /// Resets all memory storage to zero.
    void clear() noexcept;

    /// Checks if an address lies within valid memory bounds.
    [[nodiscard]] bool isValidAddress(common::Address address) const noexcept;

private:
    std::vector<common::Byte> m_storage;
};

} // namespace efs::memory

#endif // EFS_MEMORY_MEMORY_HPP
