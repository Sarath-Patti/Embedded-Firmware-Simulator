#ifndef EFS_MEMORY_MEMORY_HPP
#define EFS_MEMORY_MEMORY_HPP

#include "common/types.hpp"
#include <vector>

namespace efs::memory {

class Memory {
public:
    explicit Memory(common::Size size);
    ~Memory() = default;

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;
    Memory(Memory&&) noexcept = default;
    Memory& operator=(Memory&&) noexcept = default;

    [[nodiscard]] common::Byte read(common::Address address) const;
    void write(common::Address address, common::Byte value);
    [[nodiscard]] common::Size size() const noexcept;
    void clear() noexcept;

    [[nodiscard]] bool isValidAddress(common::Address address) const noexcept;

private:
    std::vector<common::Byte> m_storage;
};

} // namespace efs::memory

#endif // EFS_MEMORY_MEMORY_HPP
