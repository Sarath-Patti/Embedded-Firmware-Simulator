#ifndef EFS_DRIVERS_UART_UART_HPP
#define EFS_DRIVERS_UART_UART_HPP

#include "common/types.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <queue>

namespace efs::drivers::uart {

class UART {
public:
    static constexpr common::Address DATA_OFFSET    = 0x00;
    static constexpr common::Address STATUS_OFFSET  = 0x04;
    static constexpr common::Address CONTROL_OFFSET = 0x08;
    static constexpr common::Address BAUD_OFFSET    = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT     = (1U << 0);
    static constexpr common::DWord STATUS_TX_EMPTY_BIT = (1U << 0);
    static constexpr common::DWord STATUS_RX_AVAIL_BIT = (1U << 1);
    static constexpr common::DWord STATUS_ENABLED_BIT  = (1U << 2);

    static constexpr std::uint32_t DEFAULT_BAUD_RATE = 115200;

    UART(mmio::MMIOBus& bus, common::Address baseAddress, std::uint32_t defaultBaudRate = DEFAULT_BAUD_RATE);
    ~UART();

    UART(const UART&) = delete;
    UART& operator=(const UART&) = delete;
    UART(UART&&) = delete;
    UART& operator=(UART&&) = delete;

    /// Enables the UART peripheral.
    void enable();

    /// Disables the UART peripheral.
    void disable();

    /// Returns true if UART is enabled.
    [[nodiscard]] bool enabled() const noexcept;

    /// Transmits a byte by enqueueing it into TX FIFO.
    void writeByte(std::uint8_t byte);

    /// Receives a byte by dequeuing it from RX FIFO.
    [[nodiscard]] std::uint8_t readByte();

    /// Pushes a simulated incoming byte into RX FIFO.
    void pushReceivedByte(std::uint8_t byte);

    /// Returns true if RX FIFO is non-empty.
    [[nodiscard]] bool hasReceivedData() const noexcept;

    /// Returns true if TX FIFO is empty.
    [[nodiscard]] bool txEmpty() const noexcept;

    /// Configures baud rate setting.
    void setBaudRate(std::uint32_t baud);

    /// Returns current baud rate.
    [[nodiscard]] std::uint32_t baudRate() const noexcept;

    /// Returns current TX FIFO queue size.
    [[nodiscard]] std::size_t txFifoSize() const noexcept;

    /// Returns current RX FIFO queue size.
    [[nodiscard]] std::size_t rxFifoSize() const noexcept;

    /// Dequeues and returns the front byte from TX FIFO.
    [[nodiscard]] std::uint8_t popTxByte();

    /// Resets control, data registers, and flushes TX/RX FIFOs.
    void reset();

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address dataAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;
    [[nodiscard]] common::Address controlAddress() const noexcept;
    [[nodiscard]] common::Address baudAddress() const noexcept;

private:
    void updateStatusRegister();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_dataRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;
    std::shared_ptr<mmio::Register> m_controlRegister;
    std::shared_ptr<mmio::Register> m_baudRegister;

    std::queue<std::uint8_t> m_txFifo;
    std::queue<std::uint8_t> m_rxFifo;
};

} // namespace efs::drivers::uart

#endif // EFS_DRIVERS_UART_UART_HPP
