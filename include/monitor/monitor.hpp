#ifndef EFS_MONITOR_MONITOR_HPP
#define EFS_MONITOR_MONITOR_HPP

#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include <iostream>
#include <string>
#include <string_view>

namespace efs::monitor {

class Monitor {
public:
    Monitor(cpu::CPU* cpu = nullptr,
            memory::Memory* memory = nullptr,
            mmio::MMIOBus* mmioBus = nullptr,
            drivers::gpio::GPIO* gpio = nullptr,
            drivers::timer::Timer* timer = nullptr,
            kernel::InterruptController* interruptController = nullptr);
    ~Monitor() = default;

    Monitor(const Monitor&) = delete;
    Monitor& operator=(const Monitor&) = delete;
    Monitor(Monitor&&) = delete;
    Monitor& operator=(Monitor&&) = delete;

    void setCPU(cpu::CPU* cpu) noexcept;
    void setMemory(memory::Memory* memory) noexcept;
    void setMMIOBus(mmio::MMIOBus* mmioBus) noexcept;
    void setGPIO(drivers::gpio::GPIO* gpio) noexcept;
    void setTimer(drivers::timer::Timer* timer) noexcept;
    void setInterruptController(kernel::InterruptController* interruptController) noexcept;

    // Executes a single command string non-blockingly. Returns false if 'exit' command is issued.
    bool executeCommand(std::string_view commandLine, std::ostream& output = std::cout);

    // Non-blocking command processor. Alias for executeCommand.
    bool processCommand(std::string_view commandLine, std::ostream& output = std::cout);

    // Interactive monitor session loop forwarding lines from input stream to executeCommand.
    void runInteractiveSession(std::istream& input = std::cin, std::ostream& output = std::cout);

private:
    void printHelp(std::ostream& output) const;
    void printRegs(std::ostream& output) const;
    void printGPIO(std::ostream& output) const;
    void printTimer(std::ostream& output) const;
    void printInterrupts(std::ostream& output) const;
    void printMemory(std::string_view args, std::ostream& output) const;
    void printMMIO(std::ostream& output) const;
    void handleRun(std::string_view args, std::ostream& output);

    cpu::CPU* m_cpu{nullptr};
    memory::Memory* m_memory{nullptr};
    mmio::MMIOBus* m_mmioBus{nullptr};
    drivers::gpio::GPIO* m_gpio{nullptr};
    drivers::timer::Timer* m_timer{nullptr};
    kernel::InterruptController* m_interruptController{nullptr};
};

} // namespace efs::monitor

#endif // EFS_MONITOR_MONITOR_HPP
