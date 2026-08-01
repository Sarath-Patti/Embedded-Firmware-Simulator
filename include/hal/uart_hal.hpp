#ifndef EFS_HAL_UART_HAL_HPP
#define EFS_HAL_UART_HAL_HPP

#include "drivers/uart/uart.hpp"
#include <cstdint>

namespace efs::hal {

/// Hardware Abstraction Layer for UART serial communication peripherals.
class UARTHAL {
public:
    explicit UARTHAL(drivers::uart::UART* uart = nullptr) noexcept;
    explicit UARTHAL(drivers::uart::UART& uart) noexcept;
    ~UARTHAL() = default;

    UARTHAL(const UARTHAL&) = default;
    UARTHAL& operator=(const UARTHAL&) = default;
    UARTHAL(UARTHAL&&) noexcept = default;
    UARTHAL& operator=(UARTHAL&&) noexcept = default;

    /// Attaches or updates underlying UART peripheral driver.
    void attachUART(drivers::uart::UART* uart) noexcept;

    /// Returns true if a UART peripheral driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Transmits a byte over serial interface.
    void writeByte(std::uint8_t byte);

    /// Receives a byte from serial interface.
    [[nodiscard]] std::uint8_t readByte();

    /// Returns true if incoming data is available in RX FIFO.
    [[nodiscard]] bool hasData() const;

    /// Configures baud rate setting.
    void setBaudRate(std::uint32_t rate);

    /// Enables the UART peripheral.
    void enable();

    /// Disables the UART peripheral.
    void disable();

    /// Returns true if UART peripheral is enabled.
    [[nodiscard]] bool enabled() const;

private:
    drivers::uart::UART* m_uart{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_UART_HAL_HPP
