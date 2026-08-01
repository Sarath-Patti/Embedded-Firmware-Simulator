#ifndef EFS_DRIVERS_SPI_SPI_CONTROLLER_HPP
#define EFS_DRIVERS_SPI_SPI_CONTROLLER_HPP

#include "common/types.hpp"
#include "drivers/spi/spi_device.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstdint>
#include <memory>
#include <queue>

namespace efs::drivers::spi {

/// Serial Peripheral Interface (SPI) master controller modeling full-duplex synchronous transfers and MMIO registers.
class SPIController {
public:
    static constexpr common::Address DATA_OFFSET      = 0x00;
    static constexpr common::Address STATUS_OFFSET    = 0x04;
    static constexpr common::Address CONTROL_OFFSET   = 0x08;
    static constexpr common::Address CLOCK_DIV_OFFSET = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT   = (1U << 0);
    static constexpr common::DWord CTRL_MODE_MASK    = (0x03 << 1);

    static constexpr common::DWord STATUS_ENABLED_BIT  = (1U << 0);
    static constexpr common::DWord STATUS_BUSY_BIT     = (1U << 1);
    static constexpr common::DWord STATUS_RX_AVAIL_BIT = (1U << 2);

    explicit SPIController(mmio::MMIOBus& bus,
                           common::Address baseAddress,
                           std::uint32_t clockDivider = 2,
                           std::uint8_t mode = 0);
    ~SPIController();

    SPIController(const SPIController&) = delete;
    SPIController& operator=(const SPIController&) = delete;
    SPIController(SPIController&&) = delete;
    SPIController& operator=(SPIController&&) = delete;

    /// Enables the SPI controller peripheral.
    void enable();

    /// Disables the SPI controller peripheral.
    void disable();

    /// Returns true if the SPI controller is enabled.
    [[nodiscard]] bool enabled() const noexcept;

    /// Performs full-duplex SPI byte transfer with attached slave device.
    std::uint8_t transfer(std::uint8_t byte);

    /// Transmits a byte to slave device and enqueues received byte into RX buffer.
    void writeByte(std::uint8_t byte);

    /// Receives a byte from RX buffer. Throws std::underflow_error if empty.
    [[nodiscard]] std::uint8_t readByte();

    /// Returns true if RX data buffer is non-empty.
    [[nodiscard]] bool hasData() const noexcept;

    /// Configures clock divider setting.
    void setClockDivider(std::uint32_t value);

    /// Returns current clock divider setting.
    [[nodiscard]] std::uint32_t clockDivider() const noexcept;

    /// Sets SPI operation mode (0, 1, 2, 3). Throws std::invalid_argument if mode > 3.
    void setMode(std::uint8_t mode);

    /// Returns current SPI operation mode (0, 1, 2, 3).
    [[nodiscard]] std::uint8_t mode() const noexcept;

    /// Returns true if an SPI transfer is currently in progress.
    [[nodiscard]] bool busy() const noexcept;

    /// Attaches an SPI slave device.
    void attachSlave(SPIDevice* slave) noexcept;

    /// Detaches the active SPI slave device.
    void detachSlave() noexcept;

    /// Returns pointer to attached SPI slave device or nullptr.
    [[nodiscard]] SPIDevice* slave() const noexcept;

    /// Resets control, data, clock registers and clears internal FIFOs/state.
    void reset();

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address dataAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;
    [[nodiscard]] common::Address controlAddress() const noexcept;
    [[nodiscard]] common::Address clockDivAddress() const noexcept;

private:
    void updateStatusRegister();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_dataRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;
    std::shared_ptr<mmio::Register> m_controlRegister;
    std::shared_ptr<mmio::Register> m_clockDivRegister;

    SPIDevice* m_slave{nullptr};
    std::queue<std::uint8_t> m_rxBuffer;
    bool m_busy{false};
    bool m_rxHasData{false};
};

} // namespace efs::drivers::spi

#endif // EFS_DRIVERS_SPI_SPI_CONTROLLER_HPP
