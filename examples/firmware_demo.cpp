#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "firmware/basic_firmware.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    std::cout << "Starting Embedded Firmware Simulator Demo...\n";

    // 1. Create MMIO Bus
    efs::mmio::MMIOBus bus;

    // 2. Create GPIO Peripheral at 0x40000000
    constexpr efs::common::Address GPIO_BASE = 0x40000000U;
    efs::drivers::gpio::GPIO gpio(bus, GPIO_BASE);

    // 3. Create Timer Peripheral at 0x40001000
    constexpr efs::common::Address TIMER_BASE = 0x40001000U;
    efs::drivers::timer::Timer timer(bus, TIMER_BASE);

    // 4. Create Interrupt Controller at 0x40002000
    constexpr efs::common::Address IC_BASE = 0x40002000U;
    efs::kernel::InterruptController ic(bus, IC_BASE);

    // 5. Connect Timer to Interrupt Controller (Interrupt ID 0)
    constexpr std::uint8_t TIMER_INT_ID = 0;
    ic.registerInterrupt(TIMER_INT_ID);
    ic.enable(TIMER_INT_ID);
    timer.attachInterruptController(&ic, TIMER_INT_ID);

    constexpr efs::common::Size TIMER_COMPARE = 5;
    timer.setCompare(TIMER_COMPARE);
    timer.start();

    ic.registerHandler(TIMER_INT_ID, []() {
        std::cout << "  [ISR] Timer interrupt match handler executed!\n";
    });

    // 6. Create CPU Execution Engine
    efs::cpu::CPU cpu(&ic);
    cpu.attachTimer(&timer);

    // 7. Load BasicFirmware (toggles Pin 2 every 2 cycles)
    constexpr std::uint8_t LED_PIN = 2;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    auto firmware = std::make_shared<efs::firmware::BasicFirmware>(gpio, LED_PIN, TOGGLE_INTERVAL);
    cpu.loadFirmware(firmware);

    // 8. Run simulation cycles
    std::cout << "Running 10 simulation cycles...\n";
    cpu.start();
    for (efs::common::Size i = 1; i <= 10; ++i) {
        cpu.step();
        auto pinState = gpio.readPin(LED_PIN);
        std::cout << "  Cycle " << cpu.cycleCount()
                  << " | GPIO Pin 2: " << (pinState == efs::drivers::gpio::PinState::High ? "HIGH" : "LOW")
                  << " | Timer Count: " << timer.counter() << "\n";
    }
    cpu.stop();

    std::cout << "Simulation Demo finished successfully.\n";
    return 0;
}
