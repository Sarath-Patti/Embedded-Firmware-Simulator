#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace efs::kernel;
using namespace efs::drivers::timer;
using namespace efs::mmio;

void test_interrupt_registration() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    assert(!ic.isRegistered(1));
    assert(ic.registerInterrupt(1));
    assert(ic.isRegistered(1));

    // Duplicate registration fails
    assert(!ic.registerInterrupt(1));

    assert(ic.unregisterInterrupt(1));
    assert(!ic.isRegistered(1));

    std::cout << "[PASS] test_interrupt_registration\n";
}

void test_enable_disable() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(2);
    assert(!ic.enabled(2));

    ic.enable(2);
    assert(ic.enabled(2));
    assert((bus.read(0x40002000) & (1U << 2)) != 0);

    ic.disable(2);
    assert(!ic.enabled(2));
    assert((bus.read(0x40002000) & (1U << 2)) == 0);

    std::cout << "[PASS] test_enable_disable\n";
}

void test_trigger() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(3);
    assert(!ic.pending(3));

    ic.trigger(3);
    assert(ic.pending(3));
    assert((bus.read(0x40002004) & (1U << 3)) != 0);

    std::cout << "[PASS] test_trigger\n";
}

void test_clear() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(4);
    ic.trigger(4);
    assert(ic.pending(4));

    ic.clear(4);
    assert(!ic.pending(4));
    assert((bus.read(0x40002004) & (1U << 4)) == 0);

    std::cout << "[PASS] test_clear\n";
}

void test_pending_status() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(5);
    assert(!ic.pending(5));
    ic.trigger(5);
    assert(ic.pending(5));
    ic.clear(5);
    assert(!ic.pending(5));

    std::cout << "[PASS] test_pending_status\n";
}

void test_handler_registration() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(6);
    bool called = false;

    assert(ic.registerHandler(6, [&]() { called = true; }));
    ic.enable(6);
    ic.trigger(6);

    assert(ic.dispatch());
    assert(called);

    // Unregister handler
    assert(ic.unregisterHandler(6));

    std::cout << "[PASS] test_handler_registration\n";
}

void test_dispatch() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(7);
    ic.enable(7);

    int count = 0;
    ic.registerHandler(7, [&]() { count++; });

    // No pending interrupt -> dispatch returns false
    assert(!ic.dispatch());

    ic.trigger(7);
    assert(ic.pending(7));

    assert(ic.dispatch());
    assert(count == 1);
    assert(!ic.pending(7)); // Cleared after dispatch

    std::cout << "[PASS] test_dispatch\n";
}

void test_priority_selection() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    ic.registerInterrupt(1);
    ic.registerInterrupt(2);

    ic.enable(1);
    ic.enable(2);

    // Set priority: ID 1 = 10, ID 2 = 5 (lower numerical value = higher priority)
    ic.setPriority(1, 10);
    ic.setPriority(2, 5);

    std::vector<int> execution_order;
    ic.registerHandler(1, [&]() { execution_order.push_back(1); });
    ic.registerHandler(2, [&]() { execution_order.push_back(2); });

    // Trigger both
    ic.trigger(1);
    ic.trigger(2);

    // First dispatch should pick ID 2 (higher priority 5)
    assert(ic.dispatch());
    assert(execution_order.size() == 1);
    assert(execution_order[0] == 2);

    // Second dispatch should pick ID 1 (priority 10)
    assert(ic.dispatch());
    assert(execution_order.size() == 2);
    assert(execution_order[1] == 1);

    std::cout << "[PASS] test_priority_selection\n";
}

void test_timer_generated_interrupt() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);
    Timer timer(bus, 0x40001000);

    constexpr std::uint8_t TIMER_INT_ID = 0;
    ic.registerInterrupt(TIMER_INT_ID);
    ic.enable(TIMER_INT_ID);

    bool timer_isr_fired = false;
    ic.registerHandler(TIMER_INT_ID, [&]() { timer_isr_fired = true; });

    timer.attachInterruptController(&ic, TIMER_INT_ID);
    timer.setCompare(3);
    timer.start();

    // Tick timer until compare match
    timer.tick(); // 1
    timer.tick(); // 2
    assert(!ic.pending(TIMER_INT_ID));

    timer.tick(); // 3 -> COMPARE match occurs, timer triggers interrupt ID 0
    assert(ic.pending(TIMER_INT_ID));
    assert(!timer_isr_fired);

    assert(ic.dispatch());
    assert(timer_isr_fired);
    assert(!ic.pending(TIMER_INT_ID));

    std::cout << "[PASS] test_timer_generated_interrupt\n";
}

int main() {
    std::cout << "Running InterruptController unit tests...\n";
    test_interrupt_registration();
    test_enable_disable();
    test_trigger();
    test_clear();
    test_pending_status();
    test_handler_registration();
    test_dispatch();
    test_priority_selection();
    test_timer_generated_interrupt();
    std::cout << "All InterruptController unit tests passed successfully.\n";
    return 0;
}
