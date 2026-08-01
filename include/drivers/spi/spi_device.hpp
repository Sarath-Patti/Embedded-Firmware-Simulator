#ifndef EFS_DRIVERS_SPI_SPI_DEVICE_HPP
#define EFS_DRIVERS_SPI_SPI_DEVICE_HPP

#include <cstdint>
#include <queue>
#include <vector>

namespace efs::drivers::spi {

/// Abstract base class for simulated SPI slave devices.
class SPIDevice {
public:
    virtual ~SPIDevice() = default;

    /// Performs full-duplex SPI transfer receiving txByte from master and returning response byte.
    virtual std::uint8_t transfer(std::uint8_t txByte) = 0;
};

/// Simulated concrete SPI slave device capable of recording received bytes and returning configurable response bytes.
class SimulatedSPIDevice : public SPIDevice {
public:
    explicit SimulatedSPIDevice(std::uint8_t defaultResponseByte = 0x00)
        : m_defaultResponseByte(defaultResponseByte) {}

    ~SimulatedSPIDevice() override = default;

    /// Sets the static default response byte when response queue is empty.
    void setDefaultResponseByte(std::uint8_t byte) noexcept {
        m_defaultResponseByte = byte;
    }

    /// Enqueues a response byte to be returned on the next transfer.
    void queueResponseByte(std::uint8_t byte) {
        m_responseQueue.push(byte);
    }

    /// Clears the response byte queue.
    void clearResponseQueue() noexcept {
        std::queue<std::uint8_t> empty;
        std::swap(m_responseQueue, empty);
    }

    /// Returns the last byte received from SPI master.
    [[nodiscard]] std::uint8_t lastReceivedByte() const noexcept {
        return m_lastReceivedByte;
    }

    /// Returns history of all received bytes.
    [[nodiscard]] const std::vector<std::uint8_t>& receivedBytes() const noexcept {
        return m_receivedBytes;
    }

    /// Clears received bytes log history.
    void clearReceivedBytes() noexcept {
        m_receivedBytes.clear();
        m_lastReceivedByte = 0;
    }

    /// Full-duplex transfer implementation.
    std::uint8_t transfer(std::uint8_t txByte) override {
        m_lastReceivedByte = txByte;
        m_receivedBytes.push_back(txByte);

        std::uint8_t rxByte = m_defaultResponseByte;
        if (!m_responseQueue.empty()) {
            rxByte = m_responseQueue.front();
            m_responseQueue.pop();
        }
        return rxByte;
    }

private:
    std::uint8_t m_defaultResponseByte{0x00};
    std::uint8_t m_lastReceivedByte{0x00};
    std::queue<std::uint8_t> m_responseQueue;
    std::vector<std::uint8_t> m_receivedBytes;
};

} // namespace efs::drivers::spi

#endif // EFS_DRIVERS_SPI_SPI_DEVICE_HPP
