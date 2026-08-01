#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>

using namespace efs::cpu;
using namespace efs::drivers::timer;
using namespace efs::kernel;
using namespace efs::mmio;

void test_cpu_start_stop() {
    CPU cpu;
    assert(!cpu.running());

    cpu.start();
    assert(cpu.running());

    cpu.stop();
    assert(!cpu.running());

    std::cout << "[PASS] test_cpu_start_stop\n";
}

void test_cpu_reset() {
    CPU cpu;
    cpu.step();
    cpu.step();
    cpu.step();
    assert(cpu.cycleCount() == 3);

    cpu.reset();
    assert(cpu.cycleCount() == 0);

    std::cout << "[PASS] test_cpu_reset\n";
}

void test_cpu_single_step() {
    CPU cpu;
    assert(cpu.cycleCount() == 0);

    cpu.step();
    assert(cpu.cycleCount() == 1);

    std::cout << "[PASS] test_cpu_single_step\n";
}

void test_cpu_multiple_steps() {
    CPU cpu;
    cpu.run(10);
    assert(cpu.cycleCount() == 10);

    std::cout << "[PASS] test_cpu_multiple_steps\n";
}

void test_cpu_cycle_counting() {
    CPU cpu;
    cpu.run(5);
    assert(cpu.cycleCount() == 5);

    cpu.run(15);
    assert(cpu.cycleCount() == 20);

    std::cout << "[PASS] test_cpu_cycle_counting\n";
}

void test_cpu_timer_ticking() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);
    CPU cpu;

    assert(cpu.attachTimer(&timer));
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

    assert(cpu.attachTimer(&timer));

    constexpr std::uint8_t TIMER_INT_ID = 0;
    ic.registerInterrupt(TIMER_INT_ID);
    ic.enable(TIMER_INT_ID);
    timer.attachInterruptController(&ic, TIMER_INT_ID);

    bool isr_called = false;
    ic.registerHandler(TIMER_INT_ID, [&]() { isr_called = true; });

    timer.setCompare(3);
    timer.start();

    cpu.run(3);
    assert(isr_called);
    (void)isr_called;
    assert(cpu.cycleCount() == 3);

    std::cout << "[PASS] test_cpu_interrupt_dispatch\n";
}

void test_cpu_timer_attachment_removal() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);
    CPU cpu;

    assert(cpu.attachTimer(&timer));
    // Duplicate attachment fails
    assert(!cpu.attachTimer(&timer));

    assert(cpu.detachTimer(&timer));
    // Detaching unattached fails
    assert(!cpu.detachTimer(&timer));

    std::cout << "[PASS] test_cpu_timer_attachment_removal\n";
}

int main() {
    std::cout << "Running CPU unit tests...\n";
    test_cpu_start_stop();
    test_cpu_reset();
    test_cpu_single_step();
    test_cpu_multiple_steps();
    test_cpu_cycle_counting();
    test_cpu_timer_ticking();
    test_cpu_interrupt_dispatch();
    test_cpu_timer_attachment_removal();
    std::cout << "All CPU unit tests passed successfully.\n";
    return 0;
}
