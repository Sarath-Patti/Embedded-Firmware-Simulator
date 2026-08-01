#include "system/scheduler/event_scheduler.hpp"
#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <iostream>
#include <vector>

int main() {
    std::cout << "Starting Event Scheduler Demo...\n";

    // 1. Setup simulator subsystems
    efs::memory::Memory memory(1024);
    efs::mmio::MMIOBus bus;
    efs::system::SystemBus systemBus(&memory, &bus, nullptr);
    efs::cpu::CPU cpu(&systemBus);

    std::vector<std::string> executionLog;

    // 2. Schedule three events
    std::cout << "Scheduling three events:\n";
    std::cout << "  - Event 1: scheduled for Cycle 500 (\"Timer Compare\")\n";
    std::cout << "  - Event 2: scheduled for Cycle 300 (\"ADC Conversion Complete\")\n";
    std::cout << "  - Event 3: scheduled for Cycle 900 (\"Firmware Callback\")\n";

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 1 fired at Cycle 500\n";
        executionLog.push_back("Event 1 (Cycle 500)");
    }, 500, "Timer Compare");

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 2 fired at Cycle 300\n";
        executionLog.push_back("Event 2 (Cycle 300)");
    }, 300, "ADC Conversion Complete");

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 3 fired at Cycle 900\n";
        executionLog.push_back("Event 3 (Cycle 900)");
    }, 900, "Firmware Callback");

    // 3. Inspect pending events via Monitor CLI
    std::cout << "\n--- Monitor 'events' command output before execution ---\n";
    efs::monitor::Monitor monitor(&cpu, &memory, &bus, nullptr, nullptr, nullptr, nullptr, &systemBus.clock(), &systemBus.scheduler());
    monitor.executeCommand("events");

    // 4. Advance CPU simulation steps
    std::cout << "\nStepping CPU simulation to cycle 1000...\n";
    cpu.run(1000);

    // 5. Verify execution order
    std::cout << "\nExecution Log Order:\n";
    for (size_t i = 0; i < executionLog.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << executionLog[i] << "\n";
    }

    std::cout << "\nEvent Scheduler Demo completed successfully.\n";
    return 0;
}
