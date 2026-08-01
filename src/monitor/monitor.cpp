#include "monitor/monitor.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace efs::monitor {

namespace {

std::string_view trim(std::string_view s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.remove_prefix(1);
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.remove_suffix(1);
    }
    return s;
}

std::string toLower(std::string_view s) {
    std::string result(s);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

} // namespace

Monitor::Monitor(cpu::CPU* cpu,
                 memory::Memory* memory,
                 mmio::MMIOBus* mmioBus,
                 drivers::gpio::GPIO* gpio,
                 drivers::timer::Timer* timer,
                 kernel::InterruptController* interruptController,
                 drivers::uart::UART* uart,
                 system::clock::SimulationClock* clock)
    : m_cpu(cpu),
      m_memory(memory),
      m_mmioBus(mmioBus),
      m_gpio(gpio),
      m_timer(timer),
      m_interruptController(interruptController),
      m_uart(uart),
      m_clock(clock) {
}

void Monitor::setCPU(cpu::CPU* cpu) noexcept {
    m_cpu = cpu;
}

void Monitor::setMemory(memory::Memory* memory) noexcept {
    m_memory = memory;
}

void Monitor::setMMIOBus(mmio::MMIOBus* mmioBus) noexcept {
    m_mmioBus = mmioBus;
}

void Monitor::setGPIO(drivers::gpio::GPIO* gpio) noexcept {
    m_gpio = gpio;
}

void Monitor::setTimer(drivers::timer::Timer* timer) noexcept {
    m_timer = timer;
}

void Monitor::setInterruptController(kernel::InterruptController* interruptController) noexcept {
    m_interruptController = interruptController;
}

void Monitor::setUART(drivers::uart::UART* uart) noexcept {
    m_uart = uart;
}

void Monitor::setClock(system::clock::SimulationClock* clock) noexcept {
    m_clock = clock;
}

bool Monitor::executeCommand(std::string_view commandLine, std::ostream& output) {
    std::string_view trimmed = trim(commandLine);
    if (trimmed.empty()) {
        return true;
    }

    size_t spacePos = trimmed.find_first_of(" \t");
    std::string_view cmdWord = (spacePos == std::string_view::npos) ? trimmed : trimmed.substr(0, spacePos);
    std::string_view args = (spacePos == std::string_view::npos) ? "" : trim(trimmed.substr(spacePos));

    std::string cmd = toLower(cmdWord);

    if (cmd == "help") {
        printHelp(output);
        return true;
    }

    if (cmd == "exit" || cmd == "quit") {
        output << "Exiting monitor.\n";
        return false;
    }

    if (cmd == "reset") {
        if (m_cpu != nullptr) {
            m_cpu->reset();
            output << "Simulator reset.\n";
        } else {
            output << "No CPU attached to reset.\n";
        }
        return true;
    }

    if (cmd == "step") {
        if (m_cpu != nullptr) {
            m_cpu->step();
            output << "Stepped 1 cycle. Current cycle: " << m_cpu->cycleCount() << "\n";
        } else {
            output << "No CPU attached to step.\n";
        }
        return true;
    }

    if (cmd == "run") {
        handleRun(args, output);
        return true;
    }

    if (cmd == "regs") {
        printRegs(output);
        return true;
    }

    if (cmd == "gpio") {
        printGPIO(output);
        return true;
    }

    if (cmd == "timer") {
        printTimer(output);
        return true;
    }

    if (cmd == "interrupts") {
        printInterrupts(output);
        return true;
    }

    if (cmd == "memory") {
        printMemory(args, output);
        return true;
    }

    if (cmd == "mmio") {
        printMMIO(output);
        return true;
    }

    if (cmd == "uart") {
        printUART(output);
        return true;
    }

    if (cmd == "clock") {
        printClock(output);
        return true;
    }

    output << "Unknown command: '" << cmdWord << "'. Type 'help' for supported commands.\n";
    return true;
}

bool Monitor::processCommand(std::string_view commandLine, std::ostream& output) {
    return executeCommand(commandLine, output);
}

void Monitor::runInteractiveSession(std::istream& input, std::ostream& output) {
    output << "Embedded Firmware Simulator Monitor (v1.0)\n";
    output << "Type 'help' for a list of commands.\n";

    std::string line;
    output << "> ";
    while (std::getline(input, line)) {
        if (!executeCommand(line, output)) {
            break;
        }
        output << "> ";
    }
}

void Monitor::printHelp(std::ostream& output) const {
    output << "Supported commands:\n"
           << "  help                  Display all supported commands.\n"
           << "  exit                  Exit the monitor.\n"
           << "  reset                 Reset the simulator.\n"
           << "  step                  Execute exactly one CPU cycle.\n"
           << "  run <cycles>          Execute N CPU cycles.\n"
           << "  regs                  Display PC, SP, Status, and R0-R15.\n"
           << "  gpio                  Display GPIO peripheral state.\n"
           << "  timer                 Display timer peripheral state.\n"
           << "  interrupts            Display interrupt controller status.\n"
           << "  memory <addr> <cnt>   Display memory bytes in hex.\n"
           << "  mmio                  Display all registered MMIO addresses.\n"
           << "  uart                  Display UART peripheral state.\n"
           << "  clock                 Display simulation clock state.\n";
}

void Monitor::printRegs(std::ostream& output) const {
    if (m_cpu == nullptr) {
        output << "No CPU attached.\n";
        return;
    }
    const auto& rf = m_cpu->registerFile();
    output << "=== CPU Registers ===\n";
    output << "PC:     0x" << std::hex << std::setw(8) << std::setfill('0') << rf.readPC() << "\n";
    output << "SP:     0x" << std::hex << std::setw(8) << std::setfill('0') << rf.readSP() << "\n";
    output << "Status: 0x" << std::hex << std::setw(8) << std::setfill('0') << rf.readStatus() << "\n";
    for (std::size_t i = 0; i < cpu::registers::NUM_GPRS; ++i) {
        output << "R" << std::dec << i << (i < 10 ? ":  " : ": ")
               << "0x" << std::hex << std::setw(8) << std::setfill('0') << rf.readRegister(i)
               << ((i % 4 == 3) ? "\n" : "\t");
    }
    output << std::dec << std::setfill(' ');
}

void Monitor::printGPIO(std::ostream& output) const {
    if (m_gpio == nullptr) {
        output << "No GPIO attached.\n";
        return;
    }
    output << "=== GPIO Peripheral State ===\n"
           << "Base Address: 0x" << std::hex << m_gpio->baseAddress() << std::dec << "\n";
    for (std::uint8_t i = 0; i < 8; ++i) {
        auto state = m_gpio->readPin(i);
        output << "  Pin " << static_cast<int>(i) << ": "
               << (state == drivers::gpio::PinState::High ? "HIGH" : "LOW") << "\n";
    }
}

void Monitor::printTimer(std::ostream& output) const {
    if (m_timer == nullptr) {
        output << "No Timer attached.\n";
        return;
    }
    output << "=== Timer Peripheral State ===\n"
           << "Base Address: 0x" << std::hex << m_timer->baseAddress() << std::dec << "\n"
           << "Running:      " << (m_timer->running() ? "YES" : "NO") << "\n"
           << "Counter:      " << m_timer->counter() << "\n"
           << "Compare:      " << m_timer->compare() << "\n"
           << "Match Flag:   " << (m_timer->hasMatch() ? "SET" : "CLEARED") << "\n";
}

void Monitor::printInterrupts(std::ostream& output) const {
    if (m_interruptController == nullptr) {
        output << "No Interrupt Controller attached.\n";
        return;
    }
    output << "=== Interrupt Controller Status ===\n"
           << "Base Address: 0x" << std::hex << m_interruptController->baseAddress() << std::dec << "\n";
    for (std::uint8_t id = 0; id < kernel::MAX_INTERRUPTS; ++id) {
        if (m_interruptController->isRegistered(id)) {
            output << "  IRQ " << static_cast<int>(id)
                   << " | Enabled: " << (m_interruptController->enabled(id) ? "YES" : "NO")
                   << " | Pending: " << (m_interruptController->pending(id) ? "YES" : "NO")
                   << " | Priority: " << static_cast<int>(m_interruptController->getPriority(id)) << "\n";
        }
    }
}

void Monitor::printMemory(std::string_view args, std::ostream& output) const {
    if (m_memory == nullptr) {
        output << "No Memory attached.\n";
        return;
    }

    if (args.empty()) {
        output << "Usage: memory <address> <count>\n";
        return;
    }

    std::istringstream iss{std::string(args)};
    std::string addrStr;
    std::size_t count = 0;

    if (!(iss >> addrStr >> count) || count == 0) {
        output << "Usage: memory <address> <count>\n";
        return;
    }

    common::Address startAddr = 0;
    try {
        startAddr = std::stoull(addrStr, nullptr, 0);
    } catch (...) {
        output << "Invalid memory address format: '" << addrStr << "'\n";
        return;
    }

    output << "=== Memory Dump (0x" << std::hex << startAddr << " - " << count << " bytes) ===\n";
    for (std::size_t i = 0; i < count; ++i) {
        common::Address addr = startAddr + i;
        if (!m_memory->isValidAddress(addr)) {
            output << "\nMemory read out of bounds at address 0x" << std::hex << addr << "\n";
            break;
        }
        if (i % 16 == 0) {
            if (i > 0) output << "\n";
            output << "0x" << std::hex << std::setw(8) << std::setfill('0') << addr << ": ";
        }
        output << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(m_memory->read(addr)) << " ";
    }
    output << std::dec << std::setfill(' ') << "\n";
}

void Monitor::printMMIO(std::ostream& output) const {
    if (m_mmioBus == nullptr) {
        output << "No MMIO Bus attached.\n";
        return;
    }

    auto addrs = m_mmioBus->registeredAddresses();
    output << "=== Registered MMIO Addresses (" << addrs.size() << " entries) ===\n";
    for (auto addr : addrs) {
        output << "  0x" << std::hex << std::setw(8) << std::setfill('0') << addr
               << " (Value: 0x" << std::setw(8) << m_mmioBus->read(addr) << ")\n";
    }
    output << std::dec << std::setfill(' ');
}

void Monitor::printUART(std::ostream& output) const {
    if (m_uart == nullptr) {
        output << "No UART attached.\n";
        return;
    }
    output << "UART\n"
           << "----------------------\n"
           << "Enabled : " << (m_uart->enabled() ? "YES" : "NO") << "\n\n"
           << "Baud : " << m_uart->baudRate() << "\n\n"
           << "TX FIFO : " << m_uart->txFifoSize() << (m_uart->txFifoSize() == 1 ? " byte" : " bytes") << "\n\n"
           << "RX FIFO : " << m_uart->rxFifoSize() << (m_uart->rxFifoSize() == 1 ? " byte" : " bytes") << "\n\n"
           << "STATUS :\n\n"
           << "TX Empty : " << (m_uart->txEmpty() ? "YES" : "NO") << "\n\n"
           << "RX Available : " << (m_uart->hasReceivedData() ? "YES" : "NO") << "\n";
}

void Monitor::printClock(std::ostream& output) const {
    const system::clock::SimulationClock* clockPtr = m_clock;
    if (clockPtr == nullptr && m_cpu != nullptr && m_cpu->systemBus() != nullptr) {
        clockPtr = &m_cpu->systemBus()->clock();
    }
    if (clockPtr == nullptr) {
        output << "No Simulation Clock attached.\n";
        return;
    }

    std::string freqStr;
    auto freq = clockPtr->frequency();
    if (freq % 1'000'000 == 0) {
        freqStr = std::to_string(freq / 1'000'000) + " MHz";
    } else if (freq % 1'000 == 0) {
        freqStr = std::to_string(freq / 1'000) + " kHz";
    } else {
        freqStr = std::to_string(freq) + " Hz";
    }

    output << "Clock\n"
           << "---------------------\n\n"
           << "Frequency : " << freqStr << "\n\n"
           << "Cycles : " << clockPtr->cycles() << "\n\n"
           << "Elapsed : " << clockPtr->elapsedMicroseconds() << " us\n";
}

void Monitor::handleRun(std::string_view args, std::ostream& output) {
    if (m_cpu == nullptr) {
        output << "No CPU attached to run.\n";
        return;
    }

    if (args.empty()) {
        output << "Usage: run <cycles>\n";
        return;
    }

    std::string argStr(args);
    size_t firstNonSpace = argStr.find_first_not_of(" \t");
    if (firstNonSpace == std::string::npos || !std::isdigit(static_cast<unsigned char>(argStr[firstNonSpace]))) {
        output << "Invalid cycle count: '" << args << "'. Must be a positive integer.\n";
        return;
    }

    common::QWord cycles = 0;
    try {
        std::size_t pos = 0;
        cycles = std::stoull(argStr, &pos);
        if (pos != argStr.length() || cycles == 0) {
            output << "Invalid cycle count: '" << args << "'. Must be a positive integer.\n";
            return;
        }
    } catch (...) {
        output << "Invalid cycle count: '" << args << "'. Must be a positive integer.\n";
        return;
    }

    m_cpu->run(cycles);
    output << "Ran " << cycles << " cycles. Total cycle count: " << m_cpu->cycleCount() << "\n";
}

} // namespace efs::monitor
