#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "firmware/basic_firmware.hpp"
#include "firmware/firmware_manager.hpp"
#include "hal/hal.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/power/power_controller.hpp"
#include "system/power/reset_controller.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>

using namespace efs::system::power;
using namespace efs::cpu;
using namespace efs::system;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::drivers::uart;
using namespace efs::kernel;
using namespace efs::mmio;
using namespace efs::firmware;
using namespace efs::hal;

void test_power_state_transitions() {
    PowerController ctrl;
    assert(ctrl.state() == PowerState::ON);
    assert(ctrl.isPowerOn());
    assert(!ctrl.isSleep());
    assert(!ctrl.isPowerOff());

    ctrl.sleep();
    assert(ctrl.state() == PowerState::SLEEP);
    assert(ctrl.isSleep());
    assert(!ctrl.isPowerOn());

    ctrl.wake();
    assert(ctrl.state() == PowerState::ON);
    assert(ctrl.isPowerOn());

    ctrl.powerOff();
    assert(ctrl.state() == PowerState::OFF);
    assert(ctrl.isPowerOff());

    ctrl.powerOn();
    assert(ctrl.state() == PowerState::ON);

    std::cout << "[PASS] test_power_state_transitions\n";
}

void test_sleep_wake_behavior() {
    PowerController ctrl;
    
    // Cannot sleep if OFF
    ctrl.powerOff();
    ctrl.sleep();
    assert(ctrl.isPowerOff()); // remains OFF

    // Power ON then sleep
    ctrl.powerOn();
    ctrl.sleep();
    assert(ctrl.isSleep());

    // Cannot sleep if already in SLEEP
    ctrl.sleep();
    assert(ctrl.isSleep());

    // Wake from SLEEP -> ON
    ctrl.wake();
    assert(ctrl.isPowerOn());

    // Cannot wake if already ON
    ctrl.wake();
    assert(ctrl.isPowerOn());

    std::cout << "[PASS] test_sleep_wake_behavior\n";
}

void test_cpu_execution_restrictions() {
    CPU cpu;
    PowerController powerCtrl;
    cpu.setPowerController(&powerCtrl);

    cpu.start();
    cpu.step();
    cpu.step();
    assert(cpu.cycleCount() == 2);

    // Enter SLEEP -> CPU execution halted
    powerCtrl.sleep();
    cpu.step();
    cpu.step();
    cpu.run(5);
    assert(cpu.cycleCount() == 2); // No cycle progression during SLEEP

    // Power OFF -> CPU execution halted
    powerCtrl.powerOff();
    cpu.step();
    assert(cpu.cycleCount() == 2);

    // Power ON -> CPU execution resumes
    powerCtrl.powerOn();
    cpu.step();
    assert(cpu.cycleCount() == 3);

    std::cout << "[PASS] test_cpu_execution_restrictions\n";
}

void test_firmware_reset() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    GPIOHAL gpioHAL(gpio);

    CPU cpu;
    constexpr std::uint8_t PIN = 2;
    constexpr efs::common::Size INTERVAL = 2;
    auto basicFw = std::make_shared<BasicFirmware>(gpioHAL, PIN, INTERVAL);

    cpu.loadFirmware(basicFw, "Basic");
    cpu.start();

    cpu.step();
    cpu.step(); // Toggles pin HIGH
    assert(gpioHAL.read(PIN));

    ResetController resetCtrl(&cpu, nullptr, &cpu.firmwareManager());
    resetCtrl.resetFirmware();

    // Resetting firmware re-initialized it, forcing pin back LOW
    assert(basicFw->isInitialized());

    std::cout << "[PASS] test_firmware_reset\n";
}

void test_peripheral_reset() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    Timer timer(bus, 0x40001000);
    UART uart(bus, 0x40003000);
    InterruptController ic(bus, 0x40002000);

    SystemBus systemBus;
    systemBus.setMMIO(&bus);
    systemBus.attachGPIO(&gpio);
    systemBus.attachTimer(&timer);
    systemBus.attachUART(&uart);
    systemBus.setInterrupts(&ic);

    // Configure peripheral states
    gpio.configurePin(1, PinDirection::Output);
    gpio.writePin(1, PinState::High);

    constexpr efs::common::DWord COMPARE_VAL = 5;
    timer.setCompare(COMPARE_VAL);
    timer.start();
    timer.tick();
    assert(timer.counter() == 1);

    uart.enable();
    uart.writeByte('X');
    assert(uart.txFifoSize() == 1);

    ic.registerInterrupt(0);
    ic.enable(0);
    ic.trigger(0);
    assert(ic.pending(0));

    // Execute peripheral reset via ResetController
    ResetController resetCtrl(nullptr, &systemBus, nullptr);
    resetCtrl.resetPeripherals();

    // Verify peripherals are cleanly reset
    assert(gpio.readPin(1) == PinState::Low);
    assert(timer.counter() == 0);
    assert(!timer.running());
    assert(uart.txFifoSize() == 0);
    assert(!uart.enabled());
    assert(!ic.pending(0));
    assert(!ic.enabled(0));

    std::cout << "[PASS] test_peripheral_reset\n";
}

void test_cpu_reset() {
    CPU cpu;
    cpu.start();
    cpu.run(10);
    assert(cpu.cycleCount() == 10);

    ResetController resetCtrl(&cpu, nullptr, nullptr);
    resetCtrl.resetCPU();

    assert(cpu.cycleCount() == 0);

    std::cout << "[PASS] test_cpu_reset\n";
}

void test_full_system_reset() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    GPIOHAL gpioHAL(gpio);

    SystemBus systemBus;
    systemBus.setMMIO(&bus);
    systemBus.attachGPIO(&gpio);

    CPU cpu(&systemBus);
    constexpr std::uint8_t PIN = 3;
    constexpr efs::common::Size INTERVAL = 2;
    auto basicFw = std::make_shared<BasicFirmware>(gpioHAL, PIN, INTERVAL);

    cpu.loadFirmware(basicFw, "Basic");
    cpu.start();
    cpu.run(6);

    assert(cpu.cycleCount() == 6);

    ResetController resetCtrl(&cpu, &systemBus, &cpu.firmwareManager());
    resetCtrl.resetSystem();

    assert(cpu.cycleCount() == 0);
    assert(gpioHAL.read(PIN) == false);
    assert(basicFw->isInitialized());

    std::cout << "[PASS] test_full_system_reset\n";
}

void test_edge_cases() {
    // Null pointers in ResetController
    ResetController resetCtrl(nullptr, nullptr, nullptr);
    resetCtrl.resetCPU();
    resetCtrl.resetPeripherals();
    resetCtrl.resetFirmware();
    resetCtrl.resetSystem();

    // Dynamic pointer updates
    CPU cpu;
    SystemBus systemBus;
    FirmwareManager mgr;

    resetCtrl.setCPU(&cpu);
    resetCtrl.setSystemBus(&systemBus);
    resetCtrl.setFirmwareManager(&mgr);

    resetCtrl.resetSystem();

    std::cout << "[PASS] test_edge_cases\n";
}

int main() {
    std::cout << "Running Power Management unit tests...\n";
    test_power_state_transitions();
    test_sleep_wake_behavior();
    test_cpu_execution_restrictions();
    test_firmware_reset();
    test_peripheral_reset();
    test_cpu_reset();
    test_full_system_reset();
    test_edge_cases();
    std::cout << "All Power Management unit tests passed successfully.\n";
    return 0;
}
