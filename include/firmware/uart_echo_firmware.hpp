#ifndef EFS_FIRMWARE_UART_ECHO_FIRMWARE_HPP
#define EFS_FIRMWARE_UART_ECHO_FIRMWARE_HPP

#include "firmware/firmware.hpp"
#include "hal/uart_hal.hpp"
#include <cstdint>

namespace efs::firmware {

/// Sample firmware reading incoming bytes via UARTHAL and echoing them back to TX.
class UARTEchoFirmware : public Firmware {
public:
    explicit UARTEchoFirmware(hal::UARTHAL& uartHAL, std::uint32_t baudRate = 115200U);
    ~UARTEchoFirmware() override;

    void initialize() override;
    void update() override;
    void execute() override;
    void shutdown() override;
    void reset() override;

    [[nodiscard]] common::Size echoCount() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isShutdown() const noexcept;

private:
    hal::UARTHAL& m_uartHAL;
    std::uint32_t m_baudRate;
    common::Size m_echoCount{0};
    bool m_initialized{false};
    bool m_shutdown{false};
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_UART_ECHO_FIRMWARE_HPP
