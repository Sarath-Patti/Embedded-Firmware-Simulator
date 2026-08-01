#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>

using namespace efs::system;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::kernel;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::drivers::uart;
using namespace efs::cpu;

void test_bus_construction() {
    // Default construction
    SystemBus bus1;
    assert(bus1.memory() == nullptr);
    assert(bus1.mmio() == nullptr);
    assert(bus1.interrupts() == nullptr);
    assert(bus1.gpio() == nullptr);
    assert(bus1.uart() == nullptr);
    assert(bus1.timers().empty());

    // Parameterized construction
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);

    SystemBus bus2(&mem, &mmioBus, &ic);
    assert(bus2.memory() == &mem);
    assert(bus2.mmio() == &mmioBus);
    assert(bus2.interrupts() == &ic);

    std::cout << "[PASS] test_bus_construction\n";
}

void test_subsystem_access() {
    SystemBus bus;
    Memory mem(2048);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);

    bus.setMemory(&mem);
    bus.setMMIO(&mmioBus);
    bus.setInterrupts(&ic);

    assert(bus.memory() == &mem);
    assert(bus.mmio() == &mmioBus);
    assert(bus.interrupts() == &ic);

    assert(bus.memory()->size() == 2048);

    std::cout << "[PASS] test_subsystem_access\n";
}

void test_peripheral_registration() {
    MMIOBus mmioBus;
    GPIO gpio(mmioBus, 0x40000000);
    Timer timer1(mmioBus, 0x40001000);
    Timer timer2(mmioBus, 0x40001100);
    UART uart(mmioBus, 0x40003000);

    SystemBus bus;
    bus.attachGPIO(&gpio);
    bus.attachUART(&uart);
    assert(bus.attachTimer(&timer1));
    assert(bus.attachTimer(&timer2));

    assert(bus.gpio() == &gpio);
    assert(bus.uart() == &uart);
    assert(bus.timers().size() == 2);

    // Ticking timers via bus
    timer1.setCompare(2);
    timer1.start();
    bus.tickTimers();
    assert(timer1.counter() == 1);

    // Detach timer
    assert(bus.detachTimer(&timer1));
    assert(bus.timers().size() == 1);

    std::cout << "[PASS] test_peripheral_registration\n";
}

void test_cpu_integration() {
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);
    Timer timer(mmioBus, 0x40001000);

    SystemBus bus(&mem, &mmioBus, &ic);
    bus.attachTimer(&timer);

    CPU cpu(&bus);
    assert(cpu.systemBus() == &bus);
    assert(cpu.interruptController() == &ic);

    timer.setCompare(5);
    timer.start();

    cpu.step();
    assert(timer.counter() == 1);
    assert(cpu.cycleCount() == 1);

    std::cout << "[PASS] test_cpu_integration\n";
}

void test_invalid_access() {
    SystemBus bus;

    // Attach null timer
    assert(!bus.attachTimer(nullptr));

    // Detach null / non-attached timer
    MMIOBus mmioBus;
    Timer timer(mmioBus, 0x40001000);
    assert(!bus.detachTimer(nullptr));
    assert(!bus.detachTimer(&timer));

    // Duplicate attach timer
    assert(bus.attachTimer(&timer));
    assert(!bus.attachTimer(&timer));

    // Tick timers when empty
    bus.tickTimers();

    std::cout << "[PASS] test_invalid_access\n";
}

int main() {
    std::cout << "Running System Bus unit tests...\n";
    test_bus_construction();
    test_subsystem_access();
    test_peripheral_registration();
    test_cpu_integration();
    test_invalid_access();
    std::cout << "All System Bus unit tests passed successfully.\n";
    return 0;
}
