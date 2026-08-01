#include "common/logger.hpp"
#include "common/version.hpp"
#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "firmware/basic_firmware.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include "system/system_bus.hpp"
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    std::cout << "Embedded Firmware Simulator initialized.\n";

    constexpr efs::common::Size MEMORY_SIZE = 65536;
    constexpr efs::common::Address MMIO_IC_BASE = 0x40002000U;
    constexpr efs::common::Address MMIO_GPIO_BASE = 0x40000000U;
    constexpr efs::common::Address MMIO_TIMER_BASE = 0x40001000U;
    constexpr efs::common::Address MMIO_UART_BASE = 0x40003000U;

    efs::memory::Memory memory(MEMORY_SIZE);
    efs::mmio::MMIOBus bus;
    efs::kernel::InterruptController ic(bus, MMIO_IC_BASE);

    efs::system::SystemBus systemBus(&memory, &bus, &ic);

    efs::drivers::gpio::GPIO gpio(bus, MMIO_GPIO_BASE);
    efs::drivers::timer::Timer timer(bus, MMIO_TIMER_BASE);
    efs::drivers::uart::UART uart(bus, MMIO_UART_BASE);

    systemBus.attachGPIO(&gpio);
    systemBus.attachTimer(&timer);
    systemBus.attachUART(&uart);

    uart.enable();

    constexpr std::uint8_t TIMER_INT_ID = 0;
    ic.registerInterrupt(TIMER_INT_ID);
    ic.enable(TIMER_INT_ID);
    timer.attachInterruptController(&ic, TIMER_INT_ID);
    
    constexpr efs::common::DWord TIMER_COMPARE = 10;
    timer.setCompare(TIMER_COMPARE);
    timer.start();

    efs::cpu::CPU cpu(&systemBus);

    constexpr std::uint8_t FIRMWARE_PIN = 1;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    auto firmware = std::make_shared<efs::firmware::BasicFirmware>(gpio, FIRMWARE_PIN, TOGGLE_INTERVAL);
    cpu.loadFirmware(firmware);

    efs::monitor::Monitor monitor(&cpu, systemBus.memory(), systemBus.mmio(), systemBus.gpio(), &timer, systemBus.interrupts(), systemBus.uart());
    monitor.runInteractiveSession(std::cin, std::cout);

    return 0;
}
