#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace efs::cpu;
using namespace efs::drivers::timer;
using namespace efs::kernel;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::system;

void test_cpu_initialization() {
    CPU cpu;
    assert(!cpu.running());
    assert(cpu.cycleCount() == 0);
    assert(cpu.interruptController() == nullptr);
    assert(cpu.systemBus() == nullptr);

    std::cout << "[PASS] test_cpu_initialization\n";
}

void test_cpu_bus_binding() {
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);
    SystemBus systemBus(&mem, &mmioBus, &ic);

    CPU cpu(&systemBus);
    assert(cpu.systemBus() == &systemBus);
    assert(cpu.interruptController() == &ic);

    std::cout << "[PASS] test_cpu_bus_binding\n";
}

void test_cpu_start_stop() {
    CPU cpu;
    assert(!cpu.running());

    cpu.start();
    assert(cpu.running());

    cpu.stop();
    assert(!cpu.running());

    std::cout << "[PASS] test_cpu_start_stop\n";
}

void test_cpu_cycle_counting() {
    CPU cpu;
    cpu.start();

    cpu.step();
    assert(cpu.cycleCount() == 1);

    cpu.run(4);
    assert(cpu.cycleCount() == 5);

    cpu.run(15);
    assert(cpu.cycleCount() == 20);

    std::cout << "[PASS] test_cpu_cycle_counting\n";
}

void test_cpu_timer_ticking() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);
    CPU cpu;

    bool attach_ok = cpu.attachTimer(&timer);
    if (!attach_ok) {
        throw std::runtime_error("Timer attachment failed");
    }
    assert(attach_ok);

    timer.setCompare(100);
    timer.start();

    cpu.run(5);
    assert(timer.counter() == 5);
    assert(cpu.cycleCount() == 5);

    std::cout << "[PASS] test_cpu_timer_ticking\n";
}

void test_cpu_interrupt_dispatch() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);
    Timer timer(bus, 0x40001000);
    CPU cpu(&ic);

    bool attach_ok = cpu.attachTimer(&timer);
    if (!attach_ok) {
        throw std::runtime_error("Timer attachment failed");
    }
    assert(attach_ok);

    constexpr std::uint8_t TIMER_INT_ID = 0;
    ic.registerInterrupt(TIMER_INT_ID);
    ic.enable(TIMER_INT_ID);
    timer.attachInterruptController(&ic, TIMER_INT_ID);

    bool isr_called = false;
    ic.registerHandler(TIMER_INT_ID, [&]() { isr_called = true; });

    timer.setCompare(3);
    timer.start();

    cpu.run(3);
    if (!isr_called) {
        throw std::runtime_error("ISR was not called");
    }
    assert(isr_called);
    assert(cpu.cycleCount() == 3);

    std::cout << "[PASS] test_cpu_interrupt_dispatch\n";
}

void test_cpu_timer_attachment_removal() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);
    CPU cpu;

    bool attach1 = cpu.attachTimer(&timer);
    if (!attach1) {
        throw std::runtime_error("First timer attachment failed");
    }
    assert(attach1);

    // Duplicate attachment fails
    bool attach2 = !cpu.attachTimer(&timer);
    if (!attach2) {
        throw std::runtime_error("Duplicate timer attachment should fail");
    }
    assert(attach2);

    bool detach1 = cpu.detachTimer(&timer);
    if (!detach1) {
        throw std::runtime_error("First timer detachment failed");
    }
    assert(detach1);

    // Detaching unattached fails
    bool detach2 = !cpu.detachTimer(&timer);
    if (!detach2) {
        throw std::runtime_error("Detaching unattached timer should fail");
    }
    assert(detach2);

    std::cout << "[PASS] test_cpu_timer_attachment_removal\n";
}

int main() {
    std::cout << "Running CPU unit tests...\n";
    test_cpu_initialization();
    test_cpu_bus_binding();
    test_cpu_start_stop();
    test_cpu_cycle_counting();
    test_cpu_timer_ticking();
    test_cpu_interrupt_dispatch();
    test_cpu_timer_attachment_removal();
    std::cout << "All CPU unit tests passed successfully.\n";
    return 0;
}
