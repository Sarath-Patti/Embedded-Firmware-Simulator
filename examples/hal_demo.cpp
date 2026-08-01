#include "hal/hal.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "firmware/basic_firmware.hpp"
#include <iostream>

int main() {
    std::cout << "Starting Hardware Abstraction Layer (HAL) Demo...\n";

    // 1. Setup MMIO Bus & Peripheral Drivers
    efs::mmio::MMIOBus bus;
    efs::drivers::gpio::GPIO gpio(bus, 0x40000000);
    efs::drivers::timer::Timer timer(bus, 0x40001000);
    efs::drivers::uart::UART uart(bus, 0x40003000);

    // 2. Instantiate HAL Abstractions wrapping peripheral drivers
    efs::hal::GPIOHAL gpioHAL(gpio);
    efs::hal::TimerHAL timerHAL(timer);
    efs::hal::UARTHAL uartHAL(uart);

    // 3. Demonstrate GPIO HAL Operations
    std::cout << "\n--- GPIO HAL Demonstration ---\n";
    gpioHAL.configureOutput(5);
    std::cout << "Configured GPIO Pin 5 as Output.\n";

    gpioHAL.write(5, true);
    std::cout << "Wrote Pin 5 = High (read back: " << (gpioHAL.read(5) ? "High" : "Low") << ")\n";

    gpioHAL.toggle(5);
    std::cout << "Toggled Pin 5 (read back: " << (gpioHAL.read(5) ? "High" : "Low") << ")\n";

    // 4. Demonstrate Timer HAL Operations
    std::cout << "\n--- Timer HAL Demonstration ---\n";
    timerHAL.setCompare(100);
    timerHAL.start();
    std::cout << "Timer HAL started. Current counter: " << timerHAL.counter() << "\n";
    timerHAL.stop();
    timerHAL.reset();
    std::cout << "Timer HAL reset. Counter after reset: " << timerHAL.counter() << "\n";

    // 5. Demonstrate UART HAL Operations
    std::cout << "\n--- UART HAL Demonstration ---\n";
    uartHAL.setBaudRate(115200);
    uartHAL.enable();
    std::cout << "UART HAL enabled with baud rate 115200.\n";

    uartHAL.writeByte('H');
    uartHAL.writeByte('A');
    uartHAL.writeByte('L');
    std::cout << "Transmitted bytes 'H', 'A', 'L' via UARTHAL.\n";

    // Simulate incoming data
    uart.pushReceivedByte('O');
    uart.pushReceivedByte('K');

    if (uartHAL.hasData()) {
        std::cout << "Received byte via UARTHAL: '" << static_cast<char>(uartHAL.readByte()) << "'\n";
        std::cout << "Received byte via UARTHAL: '" << static_cast<char>(uartHAL.readByte()) << "'\n";
    }

    // 6. Demonstrate Firmware using HAL exclusively
    std::cout << "\n--- Firmware Execution via HAL ---\n";
    efs::firmware::BasicFirmware firmware(gpioHAL, 2, 2);
    firmware.initialize();
    std::cout << "BasicFirmware initialized for Pin 2 via GPIOHAL.\n";

    for (int cycle = 0; cycle < 5; ++cycle) {
        firmware.execute();
        std::cout << "Cycle " << (cycle + 1) << " executed. Pin 2 state: " << (gpioHAL.read(2) ? "High" : "Low") << "\n";
    }
    firmware.shutdown();

    std::cout << "\nHAL Demo completed successfully.\n";
    return 0;
}
