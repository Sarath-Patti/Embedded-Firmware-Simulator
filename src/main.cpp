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
#include <iostream>
#include <memory>

int main() {
    std::cout << "Embedded Firmware Simulator initialized.\n";

    efs::memory::Memory memory(65536);
    efs::mmio::MMIOBus bus;
    efs::kernel::InterruptController ic(bus, 0x40002000);

    efs::system::SystemBus systemBus(&memory, &bus, &ic);

    efs::drivers::gpio::GPIO gpio(bus, 0x40000000);
    efs::drivers::timer::Timer timer(bus, 0x40001000);
    efs::drivers::uart::UART uart(bus, 0x40003000);

    systemBus.attachGPIO(&gpio);
    systemBus.attachTimer(&timer);
    systemBus.attachUART(&uart);

    uart.enable();

    ic.registerInterrupt(0);
    ic.enable(0);
    timer.attachInterruptController(&ic, 0);
    timer.setCompare(10);
    timer.start();

    efs::cpu::CPU cpu(&systemBus);

    auto firmware = std::make_shared<efs::firmware::BasicFirmware>(gpio, 1, 2);
    cpu.loadFirmware(firmware);

    efs::monitor::Monitor monitor(&cpu, systemBus.memory(), systemBus.mmio(), systemBus.gpio(), &timer, systemBus.interrupts(), systemBus.uart());
    monitor.runInteractiveSession(std::cin, std::cout);

    return 0;
}
