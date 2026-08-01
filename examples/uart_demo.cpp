#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cstdint>
#include <iostream>

int main() {
    std::cout << "Starting UART Peripheral Demo...\n";

    // 1. Create MMIO Bus
    efs::mmio::MMIOBus bus;

    // 2. Create UART Peripheral at base address 0x40003000 with baud rate 115200
    constexpr efs::common::Address UART_BASE = 0x40003000U;
    constexpr std::uint32_t INITIAL_BAUD = 115200U;
    efs::drivers::uart::UART uart(bus, UART_BASE, INITIAL_BAUD);

    // 3. Enable UART
    uart.enable();
    std::cout << "UART enabled at address 0x" << std::hex << uart.baseAddress() << std::dec << "\n";

    // 4. Configure Baud Rate
    constexpr std::uint32_t NEW_BAUD = 9600U;
    uart.setBaudRate(NEW_BAUD);
    std::cout << "Configured Baud Rate: " << uart.baudRate() << "\n";

    // 5. Send Bytes (TX FIFO)
    std::cout << "Transmitting bytes: 'H', 'e', 'l', 'l', 'o'\n";
    uart.writeByte(static_cast<std::uint8_t>('H'));
    uart.writeByte(static_cast<std::uint8_t>('e'));
    uart.writeByte(static_cast<std::uint8_t>('l'));
    uart.writeByte(static_cast<std::uint8_t>('l'));
    uart.writeByte(static_cast<std::uint8_t>('o'));

    // 6. Push Received Bytes (RX FIFO)
    std::cout << "Pushing simulated received byte: 'A'\n";
    uart.pushReceivedByte(static_cast<std::uint8_t>('A'));

    // 7. Receive/Read Byte from RX FIFO
    if (uart.hasReceivedData()) {
        std::uint8_t rxByte = uart.readByte();
        std::cout << "Received byte from RX FIFO: '" << static_cast<char>(rxByte) << "' (0x" << std::hex << static_cast<int>(rxByte) << std::dec << ")\n";
    }

    // 8. Print UART State using Monitor
    std::cout << "\n--- Monitor UART State output ---\n";
    efs::monitor::Monitor monitor(nullptr, nullptr, &bus, nullptr, nullptr, nullptr, &uart);
    monitor.executeCommand("uart");

    std::cout << "\nUART Demo completed successfully.\n";
    return 0;
}
