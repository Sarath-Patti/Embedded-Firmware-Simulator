#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace efs::monitor;
using namespace efs::cpu;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::kernel;

void test_command_parsing() {
    CPU cpu;
    Memory memory(1024);
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    Timer timer(bus, 0x40001000);
    InterruptController ic(bus, 0x40002000);

    Monitor monitor(&cpu, &memory, &bus, &gpio, &timer, &ic);

    std::ostringstream ss;
    assert(monitor.executeCommand("help", ss));
    assert(ss.str().find("Supported commands:") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("regs", ss));
    assert(ss.str().find("=== CPU Registers ===") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("gpio", ss));
    assert(ss.str().find("=== GPIO Peripheral State ===") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("timer", ss));
    assert(ss.str().find("=== Timer Peripheral State ===") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("interrupts", ss));
    assert(ss.str().find("=== Interrupt Controller Status ===") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("mmio", ss));
    assert(ss.str().find("=== Registered MMIO Addresses") != std::string::npos);

    ss.str("");
    assert(!monitor.executeCommand("exit", ss));
    assert(ss.str().find("Exiting monitor.") != std::string::npos);

    std::cout << "[PASS] test_command_parsing\n";
}

void test_invalid_commands() {
    CPU cpu;
    Monitor monitor(&cpu);

    std::ostringstream ss;
    assert(monitor.executeCommand("invalidcmd", ss));
    assert(ss.str().find("Unknown command: 'invalidcmd'") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("foo bar baz", ss));
    assert(ss.str().find("Unknown command: 'foo'") != std::string::npos);

    std::cout << "[PASS] test_invalid_commands\n";
}

void test_numeric_argument_validation() {
    CPU cpu;
    Memory memory(1024);
    Monitor monitor(&cpu, &memory);

    std::ostringstream ss;
    assert(monitor.executeCommand("run invalid", ss));
    assert(ss.str().find("Invalid cycle count") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("run -5", ss));
    assert(ss.str().find("Invalid cycle count") != std::string::npos);

    ss.str("");
    assert(monitor.executeCommand("memory invalid_addr 10", ss));
    assert(ss.str().find("Invalid memory address format") != std::string::npos);

    std::cout << "[PASS] test_numeric_argument_validation\n";
}

void test_help_command() {
    Monitor monitor;
    std::ostringstream ss;

    assert(monitor.executeCommand("help", ss));
    std::string out = ss.str();
    assert(out.find("help") != std::string::npos);
    assert(out.find("exit") != std::string::npos);
    assert(out.find("reset") != std::string::npos);
    assert(out.find("step") != std::string::npos);
    assert(out.find("run") != std::string::npos);
    assert(out.find("regs") != std::string::npos);
    assert(out.find("memory") != std::string::npos);
    assert(out.find("mmio") != std::string::npos);

    std::cout << "[PASS] test_help_command\n";
}

void test_reset_command() {
    CPU cpu;
    cpu.step();
    cpu.step();
    assert(cpu.cycleCount() == 2);

    Monitor monitor(&cpu);
    std::ostringstream ss;
    assert(monitor.executeCommand("reset", ss));
    assert(cpu.cycleCount() == 0);
    assert(ss.str().find("Simulator reset.") != std::string::npos);

    std::cout << "[PASS] test_reset_command\n";
}

void test_run_command() {
    CPU cpu;
    Monitor monitor(&cpu);
    std::ostringstream ss;

    assert(monitor.executeCommand("run 15", ss));
    assert(cpu.cycleCount() == 15);
    assert(ss.str().find("Ran 15 cycles.") != std::string::npos);

    std::cout << "[PASS] test_run_command\n";
}

void test_step_command() {
    CPU cpu;
    Monitor monitor(&cpu);
    std::ostringstream ss;

    assert(cpu.cycleCount() == 0);
    assert(monitor.executeCommand("step", ss));
    assert(cpu.cycleCount() == 1);
    assert(ss.str().find("Stepped 1 cycle.") != std::string::npos);

    std::cout << "[PASS] test_step_command\n";
}

int main() {
    std::cout << "Running Monitor unit tests...\n";
    test_command_parsing();
    test_invalid_commands();
    test_numeric_argument_validation();
    test_help_command();
    test_reset_command();
    test_run_command();
    test_step_command();
    std::cout << "All Monitor unit tests passed successfully.\n";
    return 0;
}
