#include "cpu/registers/register_file.hpp"
#include "common/logger.hpp"
#include <stdexcept>
#include <string>

namespace efs::cpu::registers {

RegisterFile::RegisterFile() {
    reset();
}

common::DWord RegisterFile::readRegister(std::size_t index) const {
    validateIndex(index);
    return m_gpr[index];
}

void RegisterFile::writeRegister(std::size_t index, common::DWord value) {
    validateIndex(index);
    m_gpr[index] = value;
}

common::DWord RegisterFile::readPC() const noexcept {
    return m_pc;
}

void RegisterFile::writePC(common::DWord value) noexcept {
    m_pc = value;
}

common::DWord RegisterFile::readSP() const noexcept {
    return m_sp;
}

void RegisterFile::writeSP(common::DWord value) noexcept {
    m_sp = value;
}

common::DWord RegisterFile::readStatus() const noexcept {
    return m_sr;
}

void RegisterFile::writeStatus(common::DWord value) noexcept {
    m_sr = value;
}

void RegisterFile::reset() noexcept {
    m_gpr.fill(0);
    m_pc = 0;
    m_sp = 0;
    m_sr = 0;
}

void RegisterFile::validateIndex(std::size_t index) const {
    if (index >= NUM_GPRS) {
        common::Logger::error("Register index out of range: " + std::to_string(index));
        throw std::out_of_range("Register index out of range: " + std::to_string(index));
    }
}

} // namespace efs::cpu::registers
