#include "system/clock/simulation_clock.hpp"
#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace efs::system::clock;
using namespace efs::system;
using namespace efs::cpu;
using namespace efs::drivers::timer;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::monitor;

void test_default_construction_and_frequency() {
    SimulationClock clock;
    assert(clock.frequency() == 1'000'000);
    assert(clock.cycles() == 0);
    assert(clock.elapsedNanoseconds() == 0);
    assert(clock.elapsedMicroseconds() == 0);
    assert(clock.elapsedMilliseconds() == 0);

    std::cout << "[PASS] test_default_construction_and_frequency\n";
}

void test_frequency_configuration() {
    SimulationClock clock(2'000'000);
    assert(clock.frequency() == 2'000'000);

    clock.setFrequency(500'000);
    assert(clock.frequency() == 500'000);

    bool threw_zero = false;
    try {
        clock.setFrequency(0);
    } catch (const std::invalid_argument&) {
        threw_zero = true;
    }
    assert(threw_zero);
    (void)threw_zero;

    std::cout << "[PASS] test_frequency_configuration\n";
}

void test_ticking_and_time_conversions() {
    SimulationClock clock(1'000'000); // 1 MHz

    clock.tick(1024);
    assert(clock.cycles() == 1024);
    assert(clock.elapsedMicroseconds() == 1024);
    assert(clock.elapsedNanoseconds() == 1'024'000);
    assert(clock.elapsedMilliseconds() == 1);

    clock.reset();
    assert(clock.cycles() == 0);
    assert(clock.elapsedNanoseconds() == 0);

    std::cout << "[PASS] test_ticking_and_time_conversions\n";
}

void test_cpu_and_timer_integration() {
    Memory mem(1024);
    MMIOBus bus;
    Timer timer(bus, 0x40001000);

    SystemBus systemBus(&mem, &bus, nullptr);
    systemBus.attachTimer(&timer);
    systemBus.clock().setFrequency(1'000'000);

    CPU cpu(&systemBus);

    timer.setCompare(10);
    timer.start();

    cpu.run(5);

    assert(systemBus.clock().cycles() == 5);
    assert(timer.counter() == 5);
    assert(!timer.hasMatch());

    cpu.run(5);
    assert(systemBus.clock().cycles() == 10);
    assert(timer.counter() == 10);
    assert(timer.hasMatch());

    std::cout << "[PASS] test_cpu_and_timer_integration\n";
}

void test_monitor_clock_output() {
    SimulationClock clock(1'000'000);
    clock.tick(1024);

    Monitor monitor(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &clock);
    std::ostringstream ss;

    assert(monitor.executeCommand("clock", ss));
    std::string out = ss.str();

    assert(out.find("Clock") != std::string::npos);
    assert(out.find("Frequency : 1 MHz") != std::string::npos);
    assert(out.find("Cycles : 1024") != std::string::npos);
    assert(out.find("Elapsed : 1024 us") != std::string::npos);

    std::cout << "[PASS] test_monitor_clock_output\n";
}

int main() {
    std::cout << "Running Simulation Clock unit tests...\n";
    test_default_construction_and_frequency();
    test_frequency_configuration();
    test_ticking_and_time_conversions();
    test_cpu_and_timer_integration();
    test_monitor_clock_output();
    std::cout << "All Simulation Clock unit tests passed successfully.\n";
    return 0;
}
