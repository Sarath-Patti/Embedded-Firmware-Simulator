#include "hal/hal.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "firmware/basic_firmware.hpp"
#include <cstdint>
#include <iostream>

int main() {
    std::cout << "Starting Hardware Abstraction Layer (HAL) Demo...\n";

    // 1. Setup MMIO Bus & Peripheral Drivers
    efs::mmio::MMIOBus bus;
    constexpr efs::common::Address GPIO_BASE = 0x40000000U;
    constexpr efs::common::Address TIMER_BASE = 0x40001000U;
    constexpr efs::common::Address UART_BASE = 0x40003000U;

    efs::drivers::gpio::GPIO gpio(bus, GPIO_BASE);
    efs::drivers::timer::Timer timer(bus, TIMER_BASE);
    efs::drivers::uart::UART uart(bus, UART_BASE);

    // 2. Instantiate HAL Abstractions wrapping peripheral drivers
    efs::hal::GPIOHAL gpioHAL(gpio);
    efs::hal::TimerHAL timerHAL(timer);
    efs::hal::UARTHAL uartHAL(uart);

    // 3. Demonstrate GPIO HAL Operations
    std::cout << "\n--- GPIO HAL Demonstration ---\n";
    constexpr std::uint8_t DEMO_PIN = 5;
    gpioHAL.configureOutput(DEMO_PIN);
    std::cout << "Configured GPIO Pin 5 as Output.\n";

    gpioHAL.write(DEMO_PIN, true);
    std::cout << "Wrote Pin 5 = High (read back: " << (gpioHAL.read(DEMO_PIN) ? "High" : "Low") << ")\n";

    gpioHAL.toggle(DEMO_PIN);
    std::cout << "Toggled Pin 5 (read back: " << (gpioHAL.read(DEMO_PIN) ? "High" : "Low") << ")\n";

    // 4. Demonstrate Timer HAL Operations
    std::cout << "\n--- Timer HAL Demonstration ---\n";
    constexpr efs::common::Size TIMER_COMPARE = 100;
    timerHAL.setCompare(TIMER_COMPARE);
    timerHAL.start();
    std::cout << "Timer HAL started. Current counter: " << timerHAL.counter() << "\n";
    timerHAL.stop();
    timerHAL.reset();
    std::cout << "Timer HAL reset. Counter after reset: " << timerHAL.counter() << "\n";

    // 5. Demonstrate UART HAL Operations
    std::cout << "\n--- UART HAL Demonstration ---\n";
    constexpr std::uint32_t BAUD_RATE = 115200;
    uartHAL.setBaudRate(BAUD_RATE);
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
    constexpr std::uint8_t FW_PIN = 2;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    efs::firmware::BasicFirmware firmware(gpioHAL, FW_PIN, TOGGLE_INTERVAL);
    firmware.initialize();
    std::cout << "BasicFirmware initialized for Pin 2 via GPIOHAL.\n";

    for (int cycle = 0; cycle < 5; ++cycle) {
        firmware.execute();
        std::cout << "Cycle " << (cycle + 1) << " executed. Pin 2 state: " << (gpioHAL.read(FW_PIN) ? "High" : "Low") << "\n";
    }
    firmware.shutdown();

    std::cout << "\nHAL Demo completed successfully.\n";
    return 0;
}
