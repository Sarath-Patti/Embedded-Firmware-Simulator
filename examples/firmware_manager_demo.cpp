#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "hal/hal.hpp"
#include "firmware/basic_firmware.hpp"
#include "firmware/firmware_manager.hpp"
#include "firmware/timer_blink_firmware.hpp"
#include "firmware/uart_echo_firmware.hpp"
#include "mmio/mmio_bus.hpp"
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    std::cout << "Starting FirmwareManager Framework Demo...\n";

    // 1. Initialize MMIO Bus and Hardware Peripheral Drivers
    efs::mmio::MMIOBus bus;
    constexpr efs::common::Address GPIO_BASE = 0x40000000U;
    constexpr efs::common::Address TIMER_BASE = 0x40001000U;
    constexpr efs::common::Address UART_BASE = 0x40003000U;

    efs::drivers::gpio::GPIO gpio(bus, GPIO_BASE);
    efs::drivers::timer::Timer timer(bus, TIMER_BASE);
    efs::drivers::uart::UART uart(bus, UART_BASE);

    // 2. Wrap Peripheral Drivers with Hardware Abstraction Layer (HAL)
    efs::hal::GPIOHAL gpioHAL(gpio);
    efs::hal::TimerHAL timerHAL(timer);
    efs::hal::UARTHAL uartHAL(uart);

    // 3. Instantiate FirmwareManager Component
    efs::firmware::FirmwareManager mgr;

    // 4. Create Concrete Firmware Applications (communicating ONLY via HAL)
    constexpr std::uint8_t BASIC_PIN = 1;
    constexpr std::uint8_t BLINK_PIN = 2;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    constexpr efs::common::Size TIMER_COMPARE = 3;
    constexpr std::uint32_t BAUD_RATE = 115200U;

    auto basicFw = std::make_shared<efs::firmware::BasicFirmware>(gpioHAL, BASIC_PIN, TOGGLE_INTERVAL);
    auto blinkFw = std::make_shared<efs::firmware::TimerBlinkFirmware>(gpioHAL, timerHAL, BLINK_PIN, TIMER_COMPARE);
    auto echoFw = std::make_shared<efs::firmware::UARTEchoFirmware>(uartHAL, BAUD_RATE);

    // 5. Register Firmware Applications with Manager
    std::cout << "\nRegistering firmware applications with FirmwareManager:\n";
    mgr.registerFirmware("BasicToggle", basicFw);
    mgr.registerFirmware("TimerBlink", blinkFw);
    mgr.registerFirmware("UARTEcho", echoFw);

    std::cout << "Registered applications (" << mgr.count() << "):\n";
    for (const auto& name : mgr.registeredNames()) {
        std::cout << "  - " << name << "\n";
    }

    // 6. Demonstrate Active Firmware Execution: BasicToggle
    std::cout << "\n--- Activating & Executing 'BasicToggle' ---\n";
    mgr.setActiveFirmware("BasicToggle");
    mgr.initialize();
    std::cout << "Active: " << mgr.activeFirmwareName() << "\n";

    for (efs::common::Size step = 1; step <= 4; ++step) {
        mgr.update();
        std::cout << "Step " << step << " | Pin 1 State: " << (gpioHAL.read(BASIC_PIN) ? "HIGH" : "LOW")
                  << " | Executions: " << basicFw->executionCount() << "\n";
    }

    // 7. Switch Active Firmware to TimerBlink
    std::cout << "\n--- Switching Active Firmware to 'TimerBlink' ---\n";
    mgr.setActiveFirmware("TimerBlink");
    mgr.initialize();
    std::cout << "Active: " << mgr.activeFirmwareName() << "\n";

    for (efs::common::Size cycle = 1; cycle <= 6; ++cycle) {
        timer.tick(); // Simulate hardware timer clocking
        mgr.update();
        std::cout << "Cycle " << cycle << " | Timer Count: " << timerHAL.counter()
                  << " | Pin 2 State: " << (gpioHAL.read(BLINK_PIN) ? "HIGH" : "LOW")
                  << " | Toggles: " << blinkFw->toggleCount() << "\n";
    }

    // 8. Switch Active Firmware to UARTEcho
    std::cout << "\n--- Switching Active Firmware to 'UARTEcho' ---\n";
    mgr.setActiveFirmware("UARTEcho");
    mgr.initialize();
    std::cout << "Active: " << mgr.activeFirmwareName() << "\n";

    std::cout << "Simulating incoming UART bytes 'E', 'C', 'H', 'O'...\n";
    uart.pushReceivedByte(static_cast<std::uint8_t>('E'));
    uart.pushReceivedByte(static_cast<std::uint8_t>('C'));
    uart.pushReceivedByte(static_cast<std::uint8_t>('H'));
    uart.pushReceivedByte(static_cast<std::uint8_t>('O'));

    mgr.update(); // Executes echo loop via HAL

    std::cout << "Echo Count: " << echoFw->echoCount() << "\n";
    std::cout << "Reading echoed bytes from TX FIFO via UARTHAL:\n";
    while (uartHAL.hasData() || uart.txFifoSize() > 0) {
        std::uint8_t txByte = uart.popTxByte();
        std::cout << "  Echoed byte: '" << static_cast<char>(txByte) << "'\n";
    }

    // 9. Shutdown & Reset
    std::cout << "\nShutting down active firmware...\n";
    mgr.shutdown();
    std::cout << "FirmwareManager Demo completed successfully.\n";

    return 0;
}
