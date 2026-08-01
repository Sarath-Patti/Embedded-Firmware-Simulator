#include "system/scheduler/event_scheduler.hpp"
#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include "drivers/timer/timer.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    std::cout << "Starting Event Scheduler Demo...\n";

    // 1. Setup simulator subsystems
    constexpr efs::common::Size MEMORY_SIZE = 1024;
    efs::memory::Memory memory(MEMORY_SIZE);
    efs::mmio::MMIOBus bus;
    efs::system::SystemBus systemBus(&memory, &bus, nullptr);
    efs::cpu::CPU cpu(&systemBus);

    std::vector<std::string> executionLog;

    // 2. Schedule three events
    std::cout << "Scheduling three events:\n";
    std::cout << "  - Event 1: scheduled for Cycle 500 (\"Timer Compare\")\n";
    std::cout << "  - Event 2: scheduled for Cycle 300 (\"ADC Conversion Complete\")\n";
    std::cout << "  - Event 3: scheduled for Cycle 900 (\"Firmware Callback\")\n";

    constexpr efs::common::Size DELAY_500 = 500;
    constexpr efs::common::Size DELAY_300 = 300;
    constexpr efs::common::Size DELAY_900 = 900;

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 1 fired at Cycle 500\n";
        executionLog.push_back("Event 1 (Cycle 500)");
    }, DELAY_500, "Timer Compare");

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 2 fired at Cycle 300\n";
        executionLog.push_back("Event 2 (Cycle 300)");
    }, DELAY_300, "ADC Conversion Complete");

    systemBus.scheduler().schedule([&executionLog]() {
        std::cout << "[EXECUTED] Event 3 fired at Cycle 900\n";
        executionLog.push_back("Event 3 (Cycle 900)");
    }, DELAY_900, "Firmware Callback");

    // 3. Inspect pending events via Monitor CLI
    std::cout << "\n--- Monitor 'events' command output before execution ---\n";
    efs::monitor::Monitor monitor(&cpu, &memory, &bus, nullptr, nullptr, nullptr, nullptr, &systemBus.clock(), &systemBus.scheduler());
    monitor.executeCommand("events");

    // 4. Advance CPU simulation steps
    std::cout << "\nStepping CPU simulation to cycle 1000...\n";
    constexpr efs::common::Size STEPS_1000 = 1000;
    cpu.run(STEPS_1000);

    // 5. Verify execution order
    std::cout << "\nExecution Log Order:\n";
    for (std::size_t i = 0; i < executionLog.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << executionLog[i] << "\n";
    }

    std::cout << "\nEvent Scheduler Demo completed successfully.\n";
    return 0;
}
