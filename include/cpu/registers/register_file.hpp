#ifndef EFS_CPU_REGISTERS_REGISTER_FILE_HPP
#define EFS_CPU_REGISTERS_REGISTER_FILE_HPP

#include "common/types.hpp"
#include <array>
#include <cstddef>

namespace efs::cpu::registers {

constexpr std::size_t NUM_GPRS = 16;

/// Processor register file modeling 16 GPRs (R0-R15), Program Counter (PC), Stack Pointer (SP), and Status Register (SR).
class RegisterFile {
public:
    RegisterFile();
    ~RegisterFile() = default;

    RegisterFile(const RegisterFile&) = default;
    RegisterFile& operator=(const RegisterFile&) = default;
    RegisterFile(RegisterFile&&) noexcept = default;
    RegisterFile& operator=(RegisterFile&&) noexcept = default;

    /// Reads 32-bit GPR value at specified index (0 to NUM_GPRS - 1). Throws std::out_of_range if out of bounds.
    [[nodiscard]] common::DWord readRegister(std::size_t index) const;

    /// Writes 32-bit GPR value at specified index (0 to NUM_GPRS - 1). Throws std::out_of_range if out of bounds.
    void writeRegister(std::size_t index, common::DWord value);

    /// Reads Program Counter (PC) value.
    [[nodiscard]] common::DWord readPC() const noexcept;

    /// Writes Program Counter (PC) value.
    void writePC(common::DWord value) noexcept;

    /// Reads Stack Pointer (SP) value.
    [[nodiscard]] common::DWord readSP() const noexcept;

    /// Writes Stack Pointer (SP) value.
    void writeSP(common::DWord value) noexcept;

    /// Reads Status Register (SR) value.
    [[nodiscard]] common::DWord readStatus() const noexcept;

    /// Writes Status Register (SR) value.
    void writeStatus(common::DWord value) noexcept;

    /// Resets all GPRs, PC, SP, and SR to zero.
    void reset() noexcept;

private:
    void validateIndex(std::size_t index) const;

    std::array<common::DWord, NUM_GPRS> m_gpr;
    common::DWord m_pc{0};
    common::DWord m_sp{0};
    common::DWord m_sr{0};
};

} // namespace efs::cpu::registers

#endif // EFS_CPU_REGISTERS_REGISTER_FILE_HPP
