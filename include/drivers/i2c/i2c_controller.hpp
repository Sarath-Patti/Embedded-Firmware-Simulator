#ifndef EFS_DRIVERS_I2C_I2C_CONTROLLER_HPP
#define EFS_DRIVERS_I2C_I2C_CONTROLLER_HPP

#include "common/types.hpp"
#include "drivers/i2c/i2c_device.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstdint>
#include <memory>
#include <queue>
#include <vector>

namespace efs::drivers::i2c {

/// Inter-Integrated Circuit (I2C) master controller modeling bus transactions and MMIO registers.
class I2CController {
public:
    static constexpr common::Address DATA_OFFSET   = 0x00;
    static constexpr common::Address STATUS_OFFSET = 0x04;
    static constexpr common::Address CONTROL_OFFSET= 0x08;
    static constexpr common::Address ADDR_OFFSET   = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT = (1U << 0);
    static constexpr common::DWord CTRL_START_BIT  = (1U << 1);
    static constexpr common::DWord CTRL_STOP_BIT   = (1U << 2);

    static constexpr common::DWord STATUS_ENABLED_BIT  = (1U << 0);
    static constexpr common::DWord STATUS_BUSY_BIT     = (1U << 1);
    static constexpr common::DWord STATUS_RX_AVAIL_BIT = (1U << 2);
    static constexpr common::DWord STATUS_NACK_BIT     = (1U << 3);

    explicit I2CController(mmio::MMIOBus& bus,
                           common::Address baseAddress,
                           std::uint8_t defaultSlaveAddress = 0x50);
    ~I2CController();

    I2CController(const I2CController&) = delete;
    I2CController& operator=(const I2CController&) = delete;
    I2CController(I2CController&&) = delete;
    I2CController& operator=(I2CController&&) = delete;

    /// Enables the I2C controller peripheral.
    void enable();

    /// Disables the I2C controller peripheral.
    void disable();

    /// Returns true if the I2C controller is enabled.
    [[nodiscard]] bool enabled() const noexcept;

    /// Issues START condition on I2C bus to currently set slave address.
    /// Returns true if slave acknowledged address (ACK), false for NACK or error.
    bool start(bool isRead = false);

    /// Issues STOP condition on I2C bus.
    void stop();

    /// Transmits a byte to currently addressed I2C slave device.
    /// Returns true for ACK, false for NACK.
    bool writeByte(std::uint8_t byte);

    /// Receives a byte from currently addressed I2C slave device.
    [[nodiscard]] std::uint8_t readByte();

    /// Returns true if RX data buffer is non-empty.
    [[nodiscard]] bool hasData() const noexcept;

    /// Sets target 7-bit slave address.
    void setSlaveAddress(std::uint8_t address);

    /// Returns target 7-bit slave address.
    [[nodiscard]] std::uint8_t slaveAddress() const noexcept;

    /// Returns true if I2C bus transaction is currently in progress.
    [[nodiscard]] bool busy() const noexcept;

    /// Returns true if last transaction byte/address resulted in ACK.
    [[nodiscard]] bool lastAck() const noexcept;

    /// Attaches an I2C slave device to the bus.
    void attachDevice(I2CDevice* device);

    /// Detaches an I2C slave device from the bus.
    void detachDevice(I2CDevice* device);

    /// Resets registers, internal FIFOs, and bus state.
    void reset();

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address dataAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;
    [[nodiscard]] common::Address controlAddress() const noexcept;
    [[nodiscard]] common::Address slaveAddrAddress() const noexcept;

private:
    void updateStatusRegister();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_dataRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;
    std::shared_ptr<mmio::Register> m_controlRegister;
    std::shared_ptr<mmio::Register> m_addrRegister;

    std::vector<I2CDevice*> m_devices;
    I2CDevice* m_activeDevice{nullptr};
    std::queue<std::uint8_t> m_rxBuffer;

    bool m_busy{false};
    bool m_lastAck{false};
    bool m_isReadMode{false};
};

} // namespace efs::drivers::i2c

#endif // EFS_DRIVERS_I2C_I2C_CONTROLLER_HPP
