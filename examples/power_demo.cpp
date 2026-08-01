#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "firmware/basic_firmware.hpp"
#include "hal/hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/power/power_controller.hpp"
#include "system/power/reset_controller.hpp"
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    std::cout << "Starting Power Management & Reset Controller Demo...\n";

    // 1. Initialize Subsystems and Hardware Peripherals
    efs::mmio::MMIOBus bus;
    constexpr efs::common::Address GPIO_BASE = 0x40000000U;
    constexpr efs::common::Address TIMER_BASE = 0x40001000U;
    constexpr efs::common::Address UART_BASE = 0x40003000U;

    efs::drivers::gpio::GPIO gpio(bus, GPIO_BASE);
    efs::drivers::timer::Timer timer(bus, TIMER_BASE);
    efs::drivers::uart::UART uart(bus, UART_BASE);

    efs::hal::GPIOHAL gpioHAL(gpio);
    efs::hal::TimerHAL timerHAL(timer);
    efs::hal::UARTHAL uartHAL(uart);

    efs::system::SystemBus systemBus;
    systemBus.setMMIO(&bus);
    systemBus.attachGPIO(&gpio);
    systemBus.attachTimer(&timer);
    systemBus.attachUART(&uart);

    // 2. Initialize CPU and Power/Reset Subsystems
    efs::cpu::CPU cpu(&systemBus);
    efs::system::power::PowerController powerCtrl;
    efs::system::power::ResetController resetCtrl(&cpu, &systemBus, &cpu.firmwareManager());
    cpu.setPowerController(&powerCtrl);

    // 3. Load Firmware Application
    constexpr std::uint8_t PIN = 1;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    auto firmware = std::make_shared<efs::firmware::BasicFirmware>(gpioHAL, PIN, TOGGLE_INTERVAL);
    cpu.loadFirmware(firmware, "BasicToggle");

    // 4. Demonstrate Power State Transitions & CPU Execution Restrictions
    std::cout << "\n--- Initial Power State: ON ---\n";
    powerCtrl.powerOn();
    cpu.start();
    std::cout << "Is Power ON? " << (powerCtrl.isPowerOn() ? "Yes" : "No") << "\n";

    cpu.step();
    cpu.step();
    std::cout << "Executed 2 steps | CPU Cycle Count: " << cpu.cycleCount()
              << " | Pin 1 State: " << (gpioHAL.read(PIN) ? "HIGH" : "LOW") << "\n";

    // 5. Demonstrate SLEEP Mode
    std::cout << "\n--- Entering SLEEP Mode ---\n";
    powerCtrl.sleep();
    std::cout << "Is Power SLEEP? " << (powerCtrl.isSleep() ? "Yes" : "No") << "\n";

    std::cout << "Attempting 5 steps while in SLEEP mode...\n";
    for (int i = 0; i < 5; ++i) {
        cpu.step();
    }
    std::cout << "CPU Cycle Count after SLEEP steps: " << cpu.cycleCount() << " (Execution paused as expected)\n";

    // 6. Demonstrate Wakeup from SLEEP Mode
    std::cout << "\n--- Waking up from SLEEP Mode ---\n";
    powerCtrl.wake();
    std::cout << "Is Power ON? " << (powerCtrl.isPowerOn() ? "Yes" : "No") << "\n";

    cpu.step();
    cpu.step();
    std::cout << "Executed 2 steps post-wake | CPU Cycle Count: " << cpu.cycleCount()
              << " | Pin 1 State: " << (gpioHAL.read(PIN) ? "HIGH" : "LOW") << "\n";

    // 7. Demonstrate Reset Operations
    std::cout << "\n--- Testing ResetController Operations ---\n";
    
    std::cout << "Triggering resetCPU()...\n";
    resetCtrl.resetCPU();
    std::cout << "CPU Cycle Count after resetCPU(): " << cpu.cycleCount() << "\n";

    std::cout << "Triggering resetPeripherals()...\n";
    resetCtrl.resetPeripherals();
    std::cout << "Pin 1 State after resetPeripherals(): " << (gpioHAL.read(PIN) ? "HIGH" : "LOW") << "\n";

    std::cout << "Triggering resetSystem()...\n";
    resetCtrl.resetSystem();
    std::cout << "CPU Cycle Count after resetSystem(): " << cpu.cycleCount() << "\n";

    // 8. Power OFF
    std::cout << "\n--- Powering OFF System ---\n";
    powerCtrl.powerOff();
    std::cout << "Is Power OFF? " << (powerCtrl.isPowerOff() ? "Yes" : "No") << "\n";

    std::cout << "Power Management & Reset Controller Demo completed successfully.\n";
    return 0;
}
