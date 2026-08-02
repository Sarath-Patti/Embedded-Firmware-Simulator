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
#include <stdexcept>
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
    bool ok1 = monitor.executeCommand("help", ss);
    if (!ok1) {
        throw std::runtime_error("Help command failed");
    }
    assert(ok1);
    assert(ss.str().find("Supported commands:") != std::string::npos);

    ss.str("");
    bool ok2 = monitor.executeCommand("regs", ss);
    if (!ok2) {
        throw std::runtime_error("Regs command failed");
    }
    assert(ok2);
    assert(ss.str().find("=== CPU Registers ===") != std::string::npos);

    ss.str("");
    bool ok3 = monitor.executeCommand("gpio", ss);
    if (!ok3) {
        throw std::runtime_error("GPIO command failed");
    }
    assert(ok3);
    assert(ss.str().find("=== GPIO Peripheral State ===") != std::string::npos);

    ss.str("");
    bool ok4 = monitor.executeCommand("timer", ss);
    if (!ok4) {
        throw std::runtime_error("Timer command failed");
    }
    assert(ok4);
    assert(ss.str().find("=== Timer Peripheral State ===") != std::string::npos);

    ss.str("");
    bool ok5 = monitor.executeCommand("interrupts", ss);
    if (!ok5) {
        throw std::runtime_error("Interrupts command failed");
    }
    assert(ok5);
    assert(ss.str().find("=== Interrupt Controller Status ===") != std::string::npos);

    ss.str("");
    bool ok6 = monitor.executeCommand("mmio", ss);
    if (!ok6) {
        throw std::runtime_error("MMIO command failed");
    }
    assert(ok6);
    assert(ss.str().find("=== Registered MMIO Addresses") != std::string::npos);

    ss.str("");
    bool ok7 = !monitor.executeCommand("exit", ss);
    if (!ok7) {
        throw std::runtime_error("Exit command failed");
    }
    assert(ok7);
    assert(ss.str().find("Exiting monitor.") != std::string::npos);

    std::cout << "[PASS] test_command_parsing\n";
}

void test_invalid_commands() {
    CPU cpu;
    Monitor monitor(&cpu);

    std::ostringstream ss;
    bool ok1 = monitor.executeCommand("invalidcmd", ss);
    if (!ok1) {
        throw std::runtime_error("Invalid cmd execution check failed");
    }
    assert(ok1);
    assert(ss.str().find("Unknown command: 'invalidcmd'") != std::string::npos);

    ss.str("");
    bool ok2 = monitor.executeCommand("foo bar baz", ss);
    if (!ok2) {
        throw std::runtime_error("Unknown command execution check failed");
    }
    assert(ok2);
    assert(ss.str().find("Unknown command: 'foo'") != std::string::npos);

    std::cout << "[PASS] test_invalid_commands\n";
}

void test_numeric_argument_validation() {
    CPU cpu;
    Memory memory(1024);
    Monitor monitor(&cpu, &memory);

    std::ostringstream ss;
    bool ok1 = monitor.executeCommand("run invalid", ss);
    if (!ok1) {
        throw std::runtime_error("Run invalid command failed");
    }
    assert(ok1);
    assert(ss.str().find("Invalid cycle count") != std::string::npos);

    ss.str("");
    bool ok2 = monitor.executeCommand("run -5", ss);
    if (!ok2) {
        throw std::runtime_error("Run negative command failed");
    }
    assert(ok2);
    assert(ss.str().find("Invalid cycle count") != std::string::npos);

    ss.str("");
    bool ok3 = monitor.executeCommand("memory invalid_addr 10", ss);
    if (!ok3) {
        throw std::runtime_error("Memory invalid command failed");
    }
    assert(ok3);
    assert(ss.str().find("Invalid memory address format") != std::string::npos);

    std::cout << "[PASS] test_numeric_argument_validation\n";
}

void test_help_command() {
    Monitor monitor;
    std::ostringstream ss;

    bool ok = monitor.executeCommand("help", ss);
    if (!ok) {
        throw std::runtime_error("Help command execution failed");
    }
    assert(ok);
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
    bool ok = monitor.executeCommand("reset", ss);
    if (!ok) {
        throw std::runtime_error("Reset command failed");
    }
    assert(ok);
    assert(cpu.cycleCount() == 0);
    assert(ss.str().find("Simulator reset.") != std::string::npos);

    std::cout << "[PASS] test_reset_command\n";
}

void test_run_command() {
    CPU cpu;
    Monitor monitor(&cpu);
    std::ostringstream ss;

    bool ok = monitor.executeCommand("run 15", ss);
    if (!ok) {
        throw std::runtime_error("Run command failed");
    }
    assert(ok);
    assert(cpu.cycleCount() == 15);
    assert(ss.str().find("Ran 15 cycles.") != std::string::npos);

    std::cout << "[PASS] test_run_command\n";
}

void test_step_command() {
    CPU cpu;
    Monitor monitor(&cpu);
    std::ostringstream ss;

    assert(cpu.cycleCount() == 0);
    bool ok = monitor.executeCommand("step", ss);
    if (!ok) {
        throw std::runtime_error("Step command failed");
    }
    assert(ok);
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
