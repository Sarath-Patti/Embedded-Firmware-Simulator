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

    assert(id > 0);
    assert(scheduler.pendingCount() == 1);

    // At cycle 5, not ready
    assert(scheduler.executeReadyEvents(5) == 0);
    assert(!executed);

    // At cycle 10, ready
    assert(scheduler.executeReadyEvents(10) == 1);
    assert(executed);
    (void)executed;
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

    scheduler.executeReadyEvents(10);

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
    (void)id2;

    assert(scheduler.pendingCount() == 2);
    assert(scheduler.cancel(id1));
    assert(scheduler.pendingCount() == 1);

    scheduler.executeReadyEvents(10);
    assert(!fired);
    (void)fired;

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

    assert(monitor.executeCommand("events", ss));
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
