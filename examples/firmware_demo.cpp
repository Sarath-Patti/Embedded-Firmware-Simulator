#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "firmware/basic_firmware.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include <iostream>
#include <memory>

int main() {
    std::cout << "Starting Embedded Firmware Simulator Demo...\n";

    // 1. Create MMIO Bus
    efs::mmio::MMIOBus bus;

    // 2. Create GPIO Peripheral at 0x40000000
    efs::drivers::gpio::GPIO gpio(bus, 0x40000000);

    // 3. Create Timer Peripheral at 0x40001000
    efs::drivers::timer::Timer timer(bus, 0x40001000);

    // 4. Create Interrupt Controller at 0x40002000
    efs::kernel::InterruptController ic(bus, 0x40002000);

    // 5. Connect Timer to Interrupt Controller (Interrupt ID 0)
    ic.registerInterrupt(0);
    ic.enable(0);
    timer.attachInterruptController(&ic, 0);
    timer.setCompare(5);
    timer.start();

    ic.registerHandler(0, []() {
        std::cout << "  [ISR] Timer interrupt match handler executed!\n";
    });

    // 6. Create CPU Execution Engine
    efs::cpu::CPU cpu(&ic);
    cpu.attachTimer(&timer);

    // 7. Load BasicFirmware (toggles Pin 2 every 2 cycles)
    auto firmware = std::make_shared<efs::firmware::BasicFirmware>(gpio, 2, 2);
    cpu.loadFirmware(firmware);

    // 8. Run simulation cycles
    std::cout << "Running 10 simulation cycles...\n";
    cpu.start();
    for (int i = 1; i <= 10; ++i) {
        cpu.step();
        auto pinState = gpio.readPin(2);
        std::cout << "  Cycle " << cpu.cycleCount()
                  << " | GPIO Pin 2: " << (pinState == efs::drivers::gpio::PinState::High ? "HIGH" : "LOW")
                  << " | Timer Count: " << timer.counter() << "\n";
    }
    cpu.stop();

    std::cout << "Simulation Demo finished successfully.\n";
    return 0;
}
