#include "mmio/register.hpp"

namespace efs::mmio {

Register::Register(common::Address address, common::DWord initialValue)
    : m_address(address), m_initialValue(initialValue), m_value(initialValue) {
}

common::Address Register::address() const noexcept {
    return m_address;
}

common::DWord Register::read() const noexcept {
    return m_value;
}

void Register::write(common::DWord value) noexcept {
    m_value = value;
}

void Register::reset() noexcept {
    m_value = m_initialValue;
}

common::DWord Register::initialValue() const noexcept {
    return m_initialValue;
}

} // namespace efs::mmio
