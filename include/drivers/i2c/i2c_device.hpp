#ifndef EFS_DRIVERS_I2C_I2C_DEVICE_HPP
#define EFS_DRIVERS_I2C_I2C_DEVICE_HPP

#include <cstdint>
#include <queue>
#include <vector>

namespace efs::drivers::i2c {

/// Abstract base class for simulated I2C slave devices.
class I2CDevice {
public:
    virtual ~I2CDevice() = default;

    /// Returns the 7-bit slave address.
    [[nodiscard]] virtual std::uint8_t address() const noexcept = 0;

    /// Triggered when master initiates transmission to this slave address (isRead = true for read, false for write).
    /// Returns true for ACK, false for NACK.
    virtual bool onStart(bool isRead) = 0;

    /// Triggered when master writes a byte to this slave. Returns true for ACK, false for NACK.
    virtual bool onWrite(std::uint8_t byte) = 0;

    /// Triggered when master reads a byte from this slave. Returns byte.
    virtual std::uint8_t onRead() = 0;

    /// Triggered when master issues STOP condition on bus.
    virtual void onStop() = 0;
};

/// Simulated concrete I2C slave device supporting configurable address, ACK/NACK, and response buffers.
class SimulatedI2CDevice : public I2CDevice {
public:
    explicit SimulatedI2CDevice(std::uint8_t address = 0x50, std::uint8_t defaultResponse = 0x00)
        : m_address(address), m_defaultResponse(defaultResponse) {}

    ~SimulatedI2CDevice() override = default;

    /// Sets 7-bit slave address.
    void setAddress(std::uint8_t address) noexcept {
        m_address = address;
    }

    [[nodiscard]] std::uint8_t address() const noexcept override {
        return m_address;
    }

    /// Sets default ACK response behavior for address and write calls.
    void setAcknowledge(bool ack) noexcept {
        m_ackEnabled = ack;
    }

    /// Enqueues a response byte to be returned during master reads.
    void queueResponseByte(std::uint8_t byte) {
        m_responseQueue.push(byte);
    }

    /// Clears response byte queue.
    void clearResponseQueue() noexcept {
        std::queue<std::uint8_t> empty;
        std::swap(m_responseQueue, empty);
    }

    /// Returns log of received bytes written by master.
    [[nodiscard]] const std::vector<std::uint8_t>& receivedBytes() const noexcept {
        return m_receivedBytes;
    }

    /// Clears received bytes log history.
    void clearReceivedBytes() noexcept {
        m_receivedBytes.clear();
    }

    bool onStart(bool isRead) override {
        (void)isRead;
        return m_ackEnabled;
    }

    bool onWrite(std::uint8_t byte) override {
        if (!m_ackEnabled) {
            return false;
        }
        m_receivedBytes.push_back(byte);
        return true;
    }

    std::uint8_t onRead() override {
        std::uint8_t resp = m_defaultResponse;
        if (!m_responseQueue.empty()) {
            resp = m_responseQueue.front();
            m_responseQueue.pop();
        }
        return resp;
    }

    void onStop() override {
        // No-op for simulated device
    }

private:
    std::uint8_t m_address{0x50};
    std::uint8_t m_defaultResponse{0x00};
    bool m_ackEnabled{true};
    std::queue<std::uint8_t> m_responseQueue;
    std::vector<std::uint8_t> m_receivedBytes;
};

} // namespace efs::drivers::i2c

#endif // EFS_DRIVERS_I2C_I2C_DEVICE_HPP
