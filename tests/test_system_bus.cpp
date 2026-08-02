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
#include <stdexcept>

using namespace efs::system;
using namespace efs::cpu;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::drivers::uart;
using namespace efs::kernel;
using namespace efs::memory;
using namespace efs::mmio;

void test_bus_construction() {
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);

    SystemBus bus(&mem, &mmioBus, &ic);

    assert(bus.memory() == &mem);
    assert(bus.mmio() == &mmioBus);
    assert(bus.interruptController() == &ic);
    assert(bus.gpio() == nullptr);
    assert(bus.uart() == nullptr);
    assert(bus.timers().empty());

    std::cout << "[PASS] test_bus_construction\n";
}

void test_subsystem_access() {
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);
    SystemBus bus;

    assert(bus.memory() == nullptr);
    assert(bus.mmio() == nullptr);
    assert(bus.interruptController() == nullptr);

    bus.setMemory(&mem);
    bus.setMMIO(&mmioBus);
    bus.setInterrupts(&ic);

    assert(bus.memory() == &mem);
    assert(bus.mmio() == &mmioBus);
    assert(bus.interruptController() == &ic);

    std::cout << "[PASS] test_subsystem_access\n";
}

void test_peripheral_registration() {
    MMIOBus mmioBus;
    GPIO gpio(mmioBus, 0x40000000);
    UART uart(mmioBus, 0x40001000);
    Timer timer1(mmioBus, 0x40002000);
    Timer timer2(mmioBus, 0x40003000);

    SystemBus bus;
    bus.attachGPIO(&gpio);
    bus.attachUART(&uart);

    bool attach1_ok = bus.attachTimer(&timer1);
    if (!attach1_ok) {
        throw std::runtime_error("First timer attachment failed");
    }
    assert(attach1_ok);

    bool attach2_ok = bus.attachTimer(&timer2);
    if (!attach2_ok) {
        throw std::runtime_error("Second timer attachment failed");
    }
    assert(attach2_ok);

    assert(bus.gpio() == &gpio);
    assert(bus.uart() == &uart);
    assert(bus.timers().size() == 2);

    // Ticking timers via bus
    timer1.setCompare(2);
    timer1.start();
    bus.tickTimers();
    assert(timer1.counter() == 1);

    // Detach timer
    bool detach1_ok = bus.detachTimer(&timer1);
    if (!detach1_ok) {
        throw std::runtime_error("First timer detachment failed");
    }
    assert(detach1_ok);
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
    bool null_attach = !bus.attachTimer(nullptr);
    if (!null_attach) {
        throw std::runtime_error("Null timer attach should fail");
    }
    assert(null_attach);

    // Detach null / non-attached timer
    MMIOBus mmioBus;
    Timer timer(mmioBus, 0x40001000);
    bool null_detach = !bus.detachTimer(nullptr);
    if (!null_detach) {
        throw std::runtime_error("Null timer detach should fail");
    }
    assert(null_detach);

    bool unattached_detach = !bus.detachTimer(&timer);
    if (!unattached_detach) {
        throw std::runtime_error("Unattached timer detach should fail");
    }
    assert(unattached_detach);

    // Duplicate attach timer
    bool attach_first = bus.attachTimer(&timer);
    if (!attach_first) {
        throw std::runtime_error("First timer attach failed");
    }
    assert(attach_first);

    bool dup_attach = !bus.attachTimer(&timer);
    if (!dup_attach) {
        throw std::runtime_error("Duplicate attach timer check failed");
    }
    assert(dup_attach);

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
