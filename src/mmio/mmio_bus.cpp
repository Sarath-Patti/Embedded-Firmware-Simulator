#include "mmio/mmio_bus.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::mmio {

bool MMIOBus::registerRegister(std::shared_ptr<Register> reg) {
    if (!reg) {
        common::Logger::error("MMIO attempt to register null register pointer");
        return false;
    }
    if (contains(reg->address())) {
        common::Logger::error("MMIO duplicate register registration at address " + std::to_string(reg->address()));
        return false;
    }
    m_registers[reg->address()] = reg;
    return true;
}

bool MMIOBus::registerRegister(common::Address address, common::DWord initialValue) {
    return registerRegister(std::make_shared<Register>(address, initialValue));
}

bool MMIOBus::unregisterRegister(common::Address address) {
    auto it = m_registers.find(address);
    if (it == m_registers.end()) {
        common::Logger::warning("MMIO attempt to unregister unmapped address " + std::to_string(address));
        return false;
    }
    m_registers.erase(it);
    return true;
}

bool MMIOBus::contains(common::Address address) const noexcept {
    return m_registers.find(address) != m_registers.end();
}

common::DWord MMIOBus::read(common::Address address) const {
    auto it = m_registers.find(address);
    if (it == m_registers.end()) {
        common::Logger::error("MMIO read unmapped address " + std::to_string(address));
        throw std::out_of_range("MMIO read unmapped address " + std::to_string(address));
    }
    return it->second->read();
}

void MMIOBus::write(common::Address address, common::DWord value) {
    auto it = m_registers.find(address);
    if (it == m_registers.end()) {
        common::Logger::error("MMIO write unmapped address " + std::to_string(address));
        throw std::out_of_range("MMIO write unmapped address " + std::to_string(address));
    }
    it->second->write(value);
}

} // namespace efs::mmio
