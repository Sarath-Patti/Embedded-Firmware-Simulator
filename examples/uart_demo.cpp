#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <iostream>

int main() {
    std::cout << "Starting UART Peripheral Demo...\n";

    // 1. Create MMIO Bus
    efs::mmio::MMIOBus bus;

    // 2. Create UART Peripheral at base address 0x40003000 with baud rate 115200
    efs::drivers::uart::UART uart(bus, 0x40003000, 115200);

    // 3. Enable UART
    uart.enable();
    std::cout << "UART enabled at address 0x" << std::hex << uart.baseAddress() << std::dec << "\n";

    // 4. Configure Baud Rate
    uart.setBaudRate(9600);
    std::cout << "Configured Baud Rate: " << uart.baudRate() << "\n";

    // 5. Send Bytes (TX FIFO)
    std::cout << "Transmitting bytes: 'H', 'e', 'l', 'l', 'o'\n";
    uart.writeByte('H');
    uart.writeByte('e');
    uart.writeByte('l');
    uart.writeByte('l');
    uart.writeByte('o');

    // 6. Push Received Bytes (RX FIFO)
    std::cout << "Pushing simulated received byte: 'A'\n";
    uart.pushReceivedByte('A');

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
