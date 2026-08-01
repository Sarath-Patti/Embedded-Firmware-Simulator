#ifndef EFS_HAL_SPI_HAL_HPP
#define EFS_HAL_SPI_HAL_HPP

#include "drivers/spi/spi_controller.hpp"
#include <cstdint>

namespace efs::hal {

/// Hardware Abstraction Layer for SPI peripheral communication.
class SPIHAL {
public:
    explicit SPIHAL(drivers::spi::SPIController* controller = nullptr) noexcept;
    explicit SPIHAL(drivers::spi::SPIController& controller) noexcept;
    ~SPIHAL() = default;

    SPIHAL(const SPIHAL&) = default;
    SPIHAL& operator=(const SPIHAL&) = default;
    SPIHAL(SPIHAL&&) noexcept = default;
    SPIHAL& operator=(SPIHAL&&) noexcept = default;

    /// Attaches or updates underlying SPI controller driver.
    void attachSPI(drivers::spi::SPIController* controller) noexcept;

    /// Returns true if an SPI controller driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Transmits a byte over SPI bus.
    void writeByte(std::uint8_t byte);

    /// Receives a byte from SPI RX buffer.
    [[nodiscard]] std::uint8_t readByte();

    /// Performs full-duplex transfer returning the received byte.
    std::uint8_t transfer(std::uint8_t byte);

    /// Configures SPI mode and clock divider.
    void configure(std::uint8_t mode, std::uint32_t clockDivider);

    /// Enables the SPI peripheral.
    void enable();

    /// Disables the SPI peripheral.
    void disable();

    /// Returns true if SPI peripheral is enabled.
    [[nodiscard]] bool enabled() const;

private:
    drivers::spi::SPIController* m_controller{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_SPI_HAL_HPP
