#ifndef EFS_CPU_REGISTERS_REGISTER_FILE_HPP
#define EFS_CPU_REGISTERS_REGISTER_FILE_HPP

#include "common/types.hpp"
#include <array>
#include <cstddef>

namespace efs::cpu::registers {

constexpr std::size_t NUM_GPRS = 16;

class RegisterFile {
public:
    RegisterFile();
    ~RegisterFile() = default;

    RegisterFile(const RegisterFile&) = default;
    RegisterFile& operator=(const RegisterFile&) = default;
    RegisterFile(RegisterFile&&) noexcept = default;
    RegisterFile& operator=(RegisterFile&&) noexcept = default;

    [[nodiscard]] common::DWord readRegister(std::size_t index) const;
    void writeRegister(std::size_t index, common::DWord value);

    [[nodiscard]] common::DWord readPC() const noexcept;
    void writePC(common::DWord value) noexcept;

    [[nodiscard]] common::DWord readSP() const noexcept;
    void writeSP(common::DWord value) noexcept;

    [[nodiscard]] common::DWord readStatus() const noexcept;
    void writeStatus(common::DWord value) noexcept;

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
