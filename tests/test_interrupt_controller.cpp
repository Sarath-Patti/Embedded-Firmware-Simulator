#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace efs::kernel;
using namespace efs::drivers::timer;
using namespace efs::mmio;

void test_interrupt_registration() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ1 = 1;
    assert(!ic.isRegistered(IRQ1));
    bool reg_ok = ic.registerInterrupt(IRQ1);
    if (!reg_ok) {
        throw std::runtime_error("Interrupt registration failed");
    }
    assert(reg_ok);
    assert(ic.isRegistered(IRQ1));

    // Duplicate registration fails
    bool dup_fail = !ic.registerInterrupt(IRQ1);
    if (!dup_fail) {
        throw std::runtime_error("Duplicate registration should fail");
    }
    assert(dup_fail);

    bool unreg_ok = ic.unregisterInterrupt(IRQ1);
    if (!unreg_ok) {
        throw std::runtime_error("Unregister interrupt failed");
    }
    assert(unreg_ok);
    assert(!ic.isRegistered(IRQ1));

    std::cout << "[PASS] test_interrupt_registration\n";
}

void test_enable_disable() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ2 = 2;
    ic.registerInterrupt(IRQ2);
    assert(!ic.enabled(IRQ2));

    ic.enable(IRQ2);
    assert(ic.enabled(IRQ2));

    ic.disable(IRQ2);
    assert(!ic.enabled(IRQ2));

    std::cout << "[PASS] test_enable_disable\n";
}

void test_trigger() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ3 = 3;
    ic.registerInterrupt(IRQ3);
    assert(!ic.pending(IRQ3));

    ic.trigger(IRQ3);
    assert(ic.pending(IRQ3));

    std::cout << "[PASS] test_trigger\n";
}

void test_clear() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ4 = 4;
    ic.registerInterrupt(IRQ4);
    ic.trigger(IRQ4);
    assert(ic.pending(IRQ4));

    ic.clear(IRQ4);
    assert(!ic.pending(IRQ4));

    std::cout << "[PASS] test_clear\n";
}

void test_pending_status() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ5 = 5;
    ic.registerInterrupt(IRQ5);
    assert(!ic.pending(IRQ5));
    assert((bus.read(ic.pendingAddress()) & (1U << IRQ5)) == 0);

    ic.trigger(IRQ5);
    assert(ic.pending(IRQ5));
    assert((bus.read(ic.pendingAddress()) & (1U << IRQ5)) != 0);

    ic.clear(IRQ5);
    assert(!ic.pending(IRQ5));
    assert((bus.read(ic.pendingAddress()) & (1U << IRQ5)) == 0);

    std::cout << "[PASS] test_pending_status\n";
}

void test_handler_registration() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ6 = 6;
    ic.registerInterrupt(IRQ6);

    bool called = false;
    bool handler_ok = ic.registerHandler(IRQ6, [&]() { called = true; });
    if (!handler_ok) {
        throw std::runtime_error("Handler registration failed");
    }
    assert(handler_ok);
    ic.enable(IRQ6);
    ic.trigger(IRQ6);

    bool dispatch_ok = ic.dispatch();
    if (!dispatch_ok) {
        throw std::runtime_error("Dispatch failed");
    }
    assert(dispatch_ok);
    if (!called) {
        throw std::runtime_error("Handler was not called");
    }
    assert(called);

    // Unregister handler
    bool unhandler_ok = ic.unregisterHandler(IRQ6);
    if (!unhandler_ok) {
        throw std::runtime_error("Unregister handler failed");
    }
    assert(unhandler_ok);

    std::cout << "[PASS] test_handler_registration\n";
}

void test_dispatch() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ7 = 7;
    ic.registerInterrupt(IRQ7);
    ic.enable(IRQ7);

    int count = 0;
    ic.registerHandler(IRQ7, [&]() { count++; });

    // No pending interrupt -> dispatch returns false
    assert(!ic.dispatch());

    ic.trigger(IRQ7);
    assert(ic.pending(IRQ7));

    bool dispatch_ok = ic.dispatch();
    if (!dispatch_ok) {
        throw std::runtime_error("Dispatch failed");
    }
    assert(dispatch_ok);
    assert(count == 1);
    assert(!ic.pending(IRQ7)); // Cleared after dispatch

    std::cout << "[PASS] test_dispatch\n";
}

void test_priority_selection() {
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t IRQ1 = 1;
    constexpr std::uint8_t IRQ2 = 2;

    ic.registerInterrupt(IRQ1);
    ic.registerInterrupt(IRQ2);

    ic.enable(IRQ1);
    ic.enable(IRQ2);

    // Set priority: ID 1 = 10, ID 2 = 5 (lower numerical value = higher priority)
    ic.setPriority(IRQ1, 10);
    ic.setPriority(IRQ2, 5);

    std::vector<int> execution_order;
    ic.registerHandler(IRQ1, [&]() { execution_order.push_back(1); });
    ic.registerHandler(IRQ2, [&]() { execution_order.push_back(2); });

    // Trigger both
    ic.trigger(IRQ1);
    ic.trigger(IRQ2);

    // First dispatch should pick ID 2 (higher priority 5)
    bool dispatch1_ok = ic.dispatch();
    if (!dispatch1_ok) {
        throw std::runtime_error("First dispatch failed");
    }
    assert(dispatch1_ok);
    assert(execution_order.size() == 1);
    assert(execution_order[0] == 2);

    // Second dispatch should pick ID 1 (priority 10)
    bool dispatch2_ok = ic.dispatch();
    if (!dispatch2_ok) {
        throw std::runtime_error("Second dispatch failed");
    }
    assert(dispatch2_ok);
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

    bool timer_dispatch_ok = ic.dispatch();
    if (!timer_dispatch_ok) {
        throw std::runtime_error("Timer dispatch failed");
    }
    assert(timer_dispatch_ok);
    if (!timer_isr_fired) {
        throw std::runtime_error("Timer ISR did not fire");
    }
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
