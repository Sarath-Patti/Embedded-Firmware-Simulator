#include "system/scheduler/event_scheduler.hpp"
#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace efs::system::scheduler;
using namespace efs::system;
using namespace efs::cpu;
using namespace efs::drivers::timer;
using namespace efs::kernel;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::monitor;

void test_event_scheduling_and_execution() {
    EventScheduler scheduler;
    bool executed = false;

    EventId id = scheduler.schedule([&executed]() {
        executed = true;
    }, 10, "Test Event");

    if (id == 0) {
        throw std::runtime_error("Event schedule failed");
    }
    assert(id > 0);
    assert(scheduler.pendingCount() == 1);

    // At cycle 5, not ready
    std::size_t count5 = scheduler.executeReadyEvents(5);
    if (count5 != 0) {
        throw std::runtime_error("No event should execute at cycle 5");
    }
    assert(count5 == 0);
    assert(!executed);

    // At cycle 10, ready
    std::size_t count10 = scheduler.executeReadyEvents(10);
    if (count10 != 1) {
        throw std::runtime_error("One event should execute at cycle 10");
    }
    assert(count10 == 1);
    if (!executed) {
        throw std::runtime_error("Event was not executed");
    }
    assert(executed);
    assert(scheduler.pendingCount() == 0);

    std::cout << "[PASS] test_event_scheduling_and_execution\n";
}

void test_fifo_ordering_and_simultaneous_events() {
    EventScheduler scheduler;
    std::vector<int> order;

    scheduler.schedule([&order]() { order.push_back(1); }, 10, "First at 10");
    scheduler.schedule([&order]() { order.push_back(2); }, 10, "Second at 10");
    scheduler.schedule([&order]() { order.push_back(3); }, 5, "First at 5");

    assert(scheduler.pendingCount() == 3);

    std::size_t count = scheduler.executeReadyEvents(10);
    if (count != 3) {
        throw std::runtime_error("3 events should execute");
    }
    assert(count == 3);

    assert(order.size() == 3);
    assert(order[0] == 3); // Cycle 5 executed first
    assert(order[1] == 1); // FIFO for cycle 10
    assert(order[2] == 2); // FIFO for cycle 10

    std::cout << "[PASS] test_fifo_ordering_and_simultaneous_events\n";
}

void test_cancellation_and_clearing() {
    EventScheduler scheduler;
    bool fired = false;

    EventId id1 = scheduler.schedule([&fired]() { fired = true; }, 10, "Event 1");
    EventId id2 = scheduler.schedule([]() {}, 20, "Event 2");
    if (id1 == 0 || id2 == 0) {
        throw std::runtime_error("Event scheduling failed");
    }

    assert(scheduler.pendingCount() == 2);
    bool cancelled = scheduler.cancel(id1);
    if (!cancelled) {
        throw std::runtime_error("Event cancellation failed");
    }
    assert(cancelled);
    assert(scheduler.pendingCount() == 1);

    std::size_t count = scheduler.executeReadyEvents(10);
    if (count != 0) {
        throw std::runtime_error("No ready event should execute");
    }
    assert(count == 0);
    if (fired) {
        throw std::runtime_error("Cancelled event fired unexpectedly");
    }
    assert(!fired);

    scheduler.clear();
    assert(scheduler.pendingCount() == 0);

    std::cout << "[PASS] test_cancellation_and_clearing\n";
}

void test_timer_integration_with_scheduler() {
    Memory mem(1024);
    MMIOBus mmioBus;
    InterruptController ic(mmioBus, 0x40002000);
    Timer timer(mmioBus, 0x40001000);

    ic.registerInterrupt(0);
    ic.enable(0);

    SystemBus systemBus(&mem, &mmioBus, &ic);
    systemBus.attachTimer(&timer);
    timer.attachInterruptController(&ic, 0);

    CPU cpu(&systemBus);

    timer.setCompare(5);
    timer.start();

    assert(systemBus.scheduler().pendingCount() == 1);

    cpu.run(5);

    assert(timer.hasMatch());
    assert(ic.pending(0));
    assert(systemBus.scheduler().pendingCount() == 0);

    std::cout << "[PASS] test_timer_integration_with_scheduler\n";
}

void test_monitor_events_output() {
    EventScheduler scheduler;
    scheduler.schedule([]() {}, 500, "Timer Compare");
    scheduler.schedule([]() {}, 900, "Firmware Callback");

    Monitor monitor(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &scheduler);
    std::ostringstream ss;

    bool exec_ok = monitor.executeCommand("events", ss);
    if (!exec_ok) {
        throw std::runtime_error("Monitor events command failed");
    }
    assert(exec_ok);
    std::string out = ss.str();

    assert(out.find("Pending Events") != std::string::npos);
    assert(out.find("Timer Compare") != std::string::npos);
    assert(out.find("Firmware Callback") != std::string::npos);
    assert(out.find("500") != std::string::npos);
    assert(out.find("900") != std::string::npos);

    std::cout << "[PASS] test_monitor_events_output\n";
}

int main() {
    std::cout << "Running Event Scheduler unit tests...\n";
    test_event_scheduling_and_execution();
    test_fifo_ordering_and_simultaneous_events();
    test_cancellation_and_clearing();
    test_timer_integration_with_scheduler();
    test_monitor_events_output();
    std::cout << "All Event Scheduler unit tests passed successfully.\n";
    return 0;
}
