#include "memory/memory.hpp"
#include "common/logger.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace efs::memory {

Memory::Memory(common::Size size)
    : m_storage(size, static_cast<common::Byte>(0)) {
}

common::Byte Memory::read(common::Address address) const {
    if (!isValidAddress(address)) {
        common::Logger::error("Memory read out of bounds at address " + std::to_string(address));
        throw std::out_of_range("Memory read out of bounds at address " + std::to_string(address));
    }
    return m_storage[address];
}

void Memory::write(common::Address address, common::Byte value) {
    if (!isValidAddress(address)) {
        common::Logger::error("Memory write out of bounds at address " + std::to_string(address));
        throw std::out_of_range("Memory write out of bounds at address " + std::to_string(address));
    }
    m_storage[address] = value;
}

common::Size Memory::size() const noexcept {
    return m_storage.size();
}

void Memory::clear() noexcept {
    std::fill(m_storage.begin(), m_storage.end(), static_cast<common::Byte>(0));
}

bool Memory::isValidAddress(common::Address address) const noexcept {
    return address < m_storage.size();
}

} // namespace efs::memory
