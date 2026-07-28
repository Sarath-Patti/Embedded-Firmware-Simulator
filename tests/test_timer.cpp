#include "drivers/timer/timer.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>

using namespace efs::drivers::timer;
using namespace efs::mmio;

void test_timer_register_creation() {
    MMIOBus bus;
    {
        Timer timer(bus, 0x40001000);
        assert(bus.contains(0x40001000)); // CTRL
        assert(bus.contains(0x40001004)); // COUNT
        assert(bus.contains(0x40001008)); // COMPARE
        assert(bus.contains(0x4000100C)); // STATUS
    }
    // Destructor RAII unregisters all
    assert(!bus.contains(0x40001000));
    assert(!bus.contains(0x40001004));
    assert(!bus.contains(0x40001008));
    assert(!bus.contains(0x4000100C));

    std::cout << "[PASS] test_timer_register_creation\n";
}

void test_timer_start_stop() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    assert(!timer.running());
    assert(bus.read(0x40001000) == 0);

    timer.start();
    assert(timer.running());
    assert(bus.read(0x40001000) == Timer::CTRL_ENABLE_BIT);

    timer.stop();
    assert(!timer.running());
    assert(bus.read(0x40001000) == 0);

    std::cout << "[PASS] test_timer_start_stop\n";
}

void test_timer_reset() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    timer.setCompare(10);
    timer.start();
    timer.tick();
    timer.tick();
    assert(timer.counter() == 2);

    timer.reset();
    assert(timer.counter() == 0);
    assert(!timer.hasMatch());
    assert(bus.read(0x40001004) == 0); // COUNT
    assert(bus.read(0x4000100C) == 0); // STATUS

    std::cout << "[PASS] test_timer_reset\n";
}

void test_timer_ticking() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    timer.setCompare(100);

    // Ticks while stopped should not increment counter
    timer.tick();
    assert(timer.counter() == 0);

    // Ticks while running should increment counter
    timer.start();
    timer.tick();
    timer.tick();
    timer.tick();
    assert(timer.counter() == 3);

    std::cout << "[PASS] test_timer_ticking\n";
}

void test_timer_compare_match() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    timer.setCompare(3);
    timer.start();

    timer.tick(); // 1
    assert(timer.running());
    assert(!timer.hasMatch());

    timer.tick(); // 2
    assert(timer.running());
    assert(!timer.hasMatch());

    timer.tick(); // 3 -> Match!
    assert(!timer.running()); // Stopped on compare match
    assert(timer.hasMatch());
    assert(timer.counter() == 3);
    assert((bus.read(0x4000100C) & Timer::STATUS_MATCH_BIT) != 0);

    std::cout << "[PASS] test_timer_compare_match\n";
}

void test_timer_status_update() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    timer.setCompare(2);
    timer.start();

    timer.tick();
    timer.tick(); // Compare match

    assert(timer.hasMatch());
    assert(bus.read(0x4000100C) == Timer::STATUS_MATCH_BIT);

    // Reset clears STATUS and COUNT
    timer.reset();
    assert(!timer.hasMatch());
    assert(bus.read(0x4000100C) == 0);

    std::cout << "[PASS] test_timer_status_update\n";
}

void test_timer_mmio_register_interaction() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    // Write COMPARE register directly via MMIO bus
    bus.write(0x40001008, 5);
    assert(timer.compare() == 5);

    // Write CTRL register directly via MMIO bus (set Enable bit)
    bus.write(0x40001000, Timer::CTRL_ENABLE_BIT);
    assert(timer.running());

    // Tick until compare match
    for (int i = 0; i < 5; ++i) {
        timer.tick();
    }

    assert(!timer.running());
    assert(timer.counter() == 5);
    assert(timer.hasMatch());
    assert(bus.read(0x4000100C) == Timer::STATUS_MATCH_BIT);

    std::cout << "[PASS] test_timer_mmio_register_interaction\n";
}

int main() {
    std::cout << "Running Timer unit tests...\n";
    test_timer_register_creation();
    test_timer_start_stop();
    test_timer_reset();
    test_timer_ticking();
    test_timer_compare_match();
    test_timer_status_update();
    test_timer_mmio_register_interaction();
    std::cout << "All Timer unit tests passed successfully.\n";
    return 0;
}
