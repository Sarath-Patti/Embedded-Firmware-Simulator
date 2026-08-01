#ifndef EFS_HAL_I2C_HAL_HPP
#define EFS_HAL_I2C_HAL_HPP

#include "drivers/i2c/i2c_controller.hpp"
#include <cstddef>
#include <cstdint>
#include <queue>

namespace efs::hal {

/// Hardware Abstraction Layer for I2C master bus transactions.
class I2CHAL {
public:
    explicit I2CHAL(drivers::i2c::I2CController* controller = nullptr) noexcept;
    explicit I2CHAL(drivers::i2c::I2CController& controller) noexcept;
    ~I2CHAL() = default;

    I2CHAL(const I2CHAL&) = default;
    I2CHAL& operator=(const I2CHAL&) = default;
    I2CHAL(I2CHAL&&) noexcept = default;
    I2CHAL& operator=(I2CHAL&&) noexcept = default;

    /// Attaches or updates underlying I2C controller driver.
    void attachI2C(drivers::i2c::I2CController* controller) noexcept;

    /// Returns true if an I2C controller driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Begins write transmission to 7-bit slave address. Returns true if slave ACKed address.
    bool beginTransmission(std::uint8_t address);

    /// Ends active write transmission by issuing STOP condition. Returns true if last byte/address was ACKed.
    bool endTransmission();

    /// Transmits a single byte over I2C bus. Returns true if slave ACKed byte.
    bool writeByte(std::uint8_t byte);

    /// Receives a byte from internal HAL RX queue.
    [[nodiscard]] std::uint8_t readByte();

    /// Requests quantity bytes from 7-bit slave address into internal HAL RX queue.
    /// Returns number of bytes successfully read and ACKed.
    std::size_t requestFrom(std::uint8_t address, std::size_t quantity);

    /// Convenience single-argument requestFrom defaulting to quantity = 1.
    std::size_t requestFrom(std::uint8_t address);

    /// Returns number of bytes available to read in internal HAL RX queue.
    [[nodiscard]] std::size_t available() const noexcept;

    /// Enables the I2C peripheral.
    void enable();

    /// Disables the I2C peripheral.
    void disable();

    /// Returns true if I2C peripheral is enabled.
    [[nodiscard]] bool enabled() const;

private:
    drivers::i2c::I2CController* m_controller{nullptr};
    std::queue<std::uint8_t> m_rxQueue;
};

} // namespace efs::hal

#endif // EFS_HAL_I2C_HAL_HPP
