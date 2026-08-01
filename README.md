# Embedded Firmware Simulator

[![CI Status](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml/badge.svg)](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml)

A high-performance, modular **C++20 Embedded Firmware Simulator** designed for bare-metal firmware development, hardware abstraction layer (HAL) validation, hardware peripheral simulation, deterministic timing analysis, low-power state management, Direct Memory Access (DMA) transfers, and real-time interactive debugging without physical hardware.

---

## Overview

Developing and testing embedded firmware on physical hardware is often constrained by limited debugging visibility, slow flashing cycles, complex hardware availability, and non-deterministic timing environments.

The **Embedded Firmware Simulator** solves these challenges by providing a modular, C++20 simulated system architecture. Hardware peripherals expose standardized MMIO registers, the kernel dispatches priority interrupts, a Direct Memory Access (DMA) controller streams data between memory and peripherals without CPU intervention, the CPU coordinates cycle simulation steps, a cycle-driven Event Scheduler handles asynchronous peripheral events, a Hardware Abstraction Layer (HAL) isolates firmware from hardware details, a central FirmwareManager manages firmware application lifecycles, a Power Management & Reset Controller subsystem models system power states and reset behaviors, and an interactive monitor CLI gives developers complete visibility into system state in real time.

---

## Key Features (v1.8.0)

- **Direct Memory Access (DMA) Controller (`efs::drivers::dma::DMAController`)**: Cycle-accurate background data transfer between Memory and MMIO peripherals without CPU intervention. Supports Memory-to-Memory, Memory-to-MMIO, and MMIO-to-Memory transfers at 1 byte per simulation cycle, with completion interrupt generation.
- **Power Management Subsystem (`efs::system::power::PowerController`)**: Manages system power states (`ON`, `OFF`, `SLEEP`) and enforces CPU instruction execution restrictions during low-power modes.
- **Reset Controller (`efs::system::power::ResetController`)**: Granular reset orchestration supporting `resetCPU()`, `resetPeripherals()`, `resetFirmware()`, and `resetSystem()`.
- **Firmware Application Framework (`efs::firmware::FirmwareManager`)**: Centralized manager decoupling multiple firmware applications from hardware and CPU execution, providing active application selection and lifecycle orchestration.
- **Firmware Lifecycle Interface (`efs::firmware::Firmware`)**: Standardized lifecycle contract (`initialize`, `update`, `shutdown`, `reset`) operating strictly through Hardware Abstraction Layer (HAL) interfaces without direct peripheral coupling.
- **Concrete Firmware Applications**:
  - `BasicFirmware`: Simple GPIO toggling firmware operating via `GPIOHAL`.
  - `TimerBlinkFirmware`: Hardware timer-driven LED blinking firmware operating via `GPIOHAL` and `TimerHAL`.
  - `UARTEchoFirmware`: Automatic serial communication echo firmware operating via `UARTHAL`.
- **Continuous Integration Pipeline**: Automated multi-platform GitHub Actions CI matrix testing across Ubuntu, macOS, and Windows with strict compiler warning enforcement (`-Werror` / `/WX`).
- **Hardware Abstraction Layer (`efs::hal`)**: Provides clean, stable `GPIOHAL`, `TimerHAL`, and `UARTHAL` abstractions hiding raw peripheral implementation details from firmware.
- **Event Scheduler (`efs::system::scheduler::EventScheduler`)**: Deterministic cycle-driven event scheduler for executing callbacks at specific simulation times with strict FIFO ordering for simultaneous events.
- **Simulation Clock (`efs::system::clock::SimulationClock`)**: Centralized deterministic timing source maintaining simulated cycle counts and calculating elapsed nanoseconds, microseconds, and milliseconds.
- **System Bus Architecture (`efs::system::SystemBus`)**: Centralized communication layer interconnecting CPU, Memory, MMIO Bus, Interrupt Controller, Simulation Clock, Event Scheduler, Power Subsystem, DMA Controller, and Hardware Peripherals.
- **Memory Subsystem (`efs::memory::Memory`)**: Contiguous byte-addressable memory buffer with strict boundary checking, reset utilities, and error logging.
- **MMIO Bus Infrastructure (`efs::mmio::MMIOBus`, `efs::mmio::Register`)**: Memory-mapped register abstraction enabling peripheral integration and dynamic address dispatching.
- **GPIO Peripheral (`efs::drivers::gpio::GPIO`)**: 32-pin GPIO peripheral modeling Direction (DIR), Output (OUT), and Input (IN) registers.
- **Hardware Timer Peripheral (`efs::drivers::timer::Timer`)**: Configurable hardware timer modeling Control (CTRL), Counter (COUNT), Compare (COMPARE), and Status (STATUS) registers with match interrupts driven by scheduled events.
- **UART Peripheral (`efs::drivers::uart::UART`)**: Serial communication peripheral modeling DATA, STATUS, CONTROL, and BAUD registers with internal TX and RX FIFOs.
- **Interrupt Controller (`efs::kernel::InterruptController`)**: Priority interrupt manager supporting 32 IRQ sources, enabling/disabling, priority dispatching, and ISR registration.
- **CPU Execution Engine (`efs::cpu::CPU`)**: Cycle-based execution loop connected directly to the System Bus, driving the Simulation Clock, executing ready scheduled events, dispatching pending interrupts, stepping DMA transfers, and running loaded firmware via FirmwareManager.
- **CPU Register File (`efs::cpu::registers::RegisterFile`)**: Encapsulates processor state including 16 General Purpose Registers (R0–R15), Program Counter (PC), Stack Pointer (SP), and Status Register (SR).
- **Interactive Monitor CLI (`efs::monitor::Monitor`)**: Non-destructive command-line debugger allowing real-time step execution, memory dumps, register inspection, peripheral querying, clock timing observation, and pending event inspection.

---

## System Architecture

```text
                       ┌─────────────────────────┐
                       │   Interactive Monitor   │  (CLI Debugger)
                       └────────────┬────────────┘
                                    │
                                    ▼
                       ┌─────────────────────────┐
                       │   CPU Execution Engine  │
                       └─────┬─────────────┬─────┘
                             │             │
              ┌──────────────┘             └──────────────┐
              ▼                                           ▼
┌──────────────────────────┐                    ┌───────────────────┐
│ PowerController (ON/OFF) │                    │ Firmware Manager  │
└─────────────┬────────────┘                    └─────────┬─────────┘
              │                                           │
              ▼                                           ▼
┌──────────────────────────┐                   ┌────────────────────┐
│     ResetController      │                   │ Firmware App (HAL) │
└─────────────┬────────────┘                   └──────────┬─────────┘
              │                                           │
              └─────────────────────┬─────────────────────┘
                                    ▼
                       ┌─────────────────────────┐
                       │       System Bus        │
                       └─────┬───────────┬───────┘
                             │           │
           ┌─────────────────┼───────────┼─────────────────┐
           ▼                 ▼           ▼                 ▼
┌─────────────────────┐ ┌─────────┐ ┌─────────┐ ┌─────────────────────┐
│   Memory Subsystem  │ │   DMA   │ │  MMIO   │ │Interrupt Controller │
└─────────────────────┘ └─────────┘ └────┬────┘ └─────────────────────┘
                                         │
                     ┌───────────────────┼───────────────────┐
                     ▼                   ▼                   ▼
            ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
            │ GPIO Peripheral │ │ TimerPeripheral │ │ UART Peripheral │
            └─────────────────┘ └─────────────────┘ └─────────────────┘
```

---

## Direct Memory Access (DMA) Architecture & Transfer Flow

The DMA Controller (`efs::drivers::dma::DMAController`) performs autonomous data transfers between Memory and MMIO Peripherals at a rate of one byte per simulation cycle without consuming CPU execution instructions.

### DMA Architecture

```text
                 CPU Execution Loop
                         │
                         ▼
                  DMA Controller
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
       Memory        MMIO Bus      Peripherals
                                 ┌───┼───┐
                                 ▼   ▼   ▼
                               GPIO Timer UART
```

### Transfer Flow

```text
1. configure(src, dst, len) ──► 2. start() ──► 3. Cycle Step (1 Byte/Cycle) ──► 4. Completion Interrupt
```

1. **Configuration**: `configure(srcAddress, dstAddress, length)` sets up source/destination addresses and transfer byte count. Incrementing behavior is auto-detected for Memory (increment) vs MMIO registers (fixed address).
2. **Initiation**: `start()` validates address bounds and sets `busy() = true`.
3. **Cycle Execution**: On each simulation cycle, `step()` transfers 1 byte from source to destination without blocking the CPU.
4. **Completion**: Upon transferring the final byte, `busy() = false`, `completed() = true`, and a completion interrupt is triggered on the configured Interrupt Controller IRQ channel.

### Example Usage

```cpp
// Initialize SystemBus and DMA Controller
efs::memory::Memory memory(2048);
efs::mmio::MMIOBus mmioBus;
efs::kernel::InterruptController ic(mmioBus, 0x40002000);
efs::system::SystemBus systemBus(&memory, &mmioBus, &ic);

efs::drivers::dma::DMAController dma(&memory, &mmioBus, &ic);
dma.attachInterruptController(&ic, 5 /* IRQ ID */);
systemBus.attachDMA(&dma);

efs::cpu::CPU cpu(&systemBus);

// Configure DMA for Memory-to-Memory copy (64 bytes)
dma.configure(0x0100, 0x0200, 64);
dma.start();

// Run CPU for 64 simulation cycles (DMA steps 1 byte per cycle in background)
cpu.run(64);

assert(dma.completed());
assert(ic.pending(5));
```

---

## Power Management & Reset Controller

The Power Management subsystem simulates bare-metal embedded power states and multi-tiered reset handling.

### Power State Diagram

```text
           ┌───────────────┐
           │     OFF       │
           └───────┬───────┘
                   │ powerOn()
                   ▼
  sleep()  ┌───────────────┐
 ┌─────────┤      ON       ├─────────┐
 │         └───────▲───────┘         │
 │                 │ wake()          │ powerOff()
 ▼                 │                 ▼
┌──────────────────┴────┐    ┌───────────────┐
│        SLEEP          │    │     OFF       │
└───────────────────────┘    └───────────────┘
```

- **ON**: Full active execution mode. CPU steps, advances clock cycles, dispatches interrupts, steps DMA transfers, and updates active firmware.
- **SLEEP**: Low-power standby mode. CPU halts instruction execution (`step()` and `run()` immediately return without advancing cycle counters).
- **OFF**: Powered down state. System execution is suspended.

---

## Firmware Application Framework

The Firmware Application Framework (`efs::firmware`) allows multiple firmware applications to coexist while remaining completely decoupled from the simulator implementation.

### Firmware Lifecycle

Every firmware application implements the standardized `efs::firmware::Firmware` interface:

1. `initialize()`: Invoked once when the application starts execution. Configures HAL peripherals.
2. `update()`: Invoked on every CPU simulation step to process input state and update outputs via HAL.
3. `shutdown()`: Invoked when active firmware is halted. Safely disables hardware timers, outputs, and channels.
4. `reset()`: Resets internal state to initial pre-execution defaults.

---

## Continuous Integration & Quality Gates

The project maintains a production-grade GitHub Actions CI pipeline (`.github/workflows/ci.yml`):

- **Cross-Platform Matrix**: Automated builds on Linux, macOS, and Windows.
- **Strict Warning Management**: Compiles with `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC). Passing `-DEFS_ENABLE_WERROR=ON` in CI treats warnings as errors without breaking local developer flexibility.
- **Immediate Failure Enforcement**: Tests run via `ctest --output-on-failure`, halting the build immediately if any assertion or test fails.

---

## Repository Layout

```text
Embedded-Firmware-Simulator/
├── .github/                  # GitHub Actions CI workflow definitions
│   └── workflows/
│       └── ci.yml            # CI build & test pipeline matrix
├── include/                  # Public C++ header interfaces
│   ├── drivers/
│   │   └── dma/              # DMAController header
│   └── system/
│       └── power/            # PowerController & ResetController headers
├── src/                      # Implementation files matching include/ structure
│   ├── drivers/
│   │   └── dma/              # DMAController implementation
│   └── system/
│       └── power/            # PowerController & ResetController implementations
├── examples/                 # Sample applications (firmware_demo, uart_demo, hal_demo, power_demo, dma_demo)
├── tests/                    # CTest unit test suite (17 test suites)
├── CMakeLists.txt            # CMake build system configuration
└── README.md                 # Project documentation
```

---

## Build Instructions

### Prerequisites
- C++20 compliant compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.16 or higher

### Build Steps

```bash
# Create build directory
mkdir build && cd build

# Configure CMake build system
cmake ..

# Build static library, main simulator demo, examples, and test executables
cmake --build .
```

### Enabling Warnings as Errors (Strict Quality Mode)

To match CI strict compiler warning checks locally, pass `-DEFS_ENABLE_WERROR=ON` during CMake configuration:

```bash
cmake -B build -DEFS_ENABLE_WERROR=ON
cmake --build build
```

---

## Running the Simulator

### Interactive Simulator Monitor Demo

```bash
./simulator_demo
```

### Example Demos

```bash
./firmware_demo
./uart_demo
./event_scheduler_demo
./hal_demo
./firmware_manager_demo
./power_demo
./dma_demo
```

---

## Running Tests

Execute the full CTest unit test suite covering all 17 project test modules:

```bash
# Navigate to build directory and run tests
cd build
ctest --output-on-failure
```

Alternatively, from the repository root:

```bash
ctest --test-dir build --output-on-failure
```

---

## Project Statistics

- **Core Subsystems**: DMA Controller (`DMAController`), Power Subsystem (`PowerController`, `ResetController`), Firmware Manager, HAL (`GPIOHAL`, `TimerHAL`, `UARTHAL`), Event Scheduler, Simulation Clock, System Bus, Memory, MMIO, GPIO, Timer, UART, Interrupt Controller, CPU, Register File, Firmware Framework, Interactive Monitor
- **Peripherals Modeled**: 3 (`GPIO`, `Hardware Timer`, `UART Serial Interface`)
- **Unit Test Suites**: 17 (`MemoryTests`, `MMIOTests`, `GPIOTests`, `TimerTests`, `InterruptTests`, `CPUTests`, `FirmwareTests`, `FirmwareManagerTests`, `PowerTests`, `RegisterTests`, `MonitorTests`, `UARTTests`, `SystemTests`, `ClockTests`, `SchedulerTests`, `HALTests`, `DMATests`)
- **CI Test Platforms**: Ubuntu, macOS, Windows
- **Language Standard**: Modern C++20

---

## Milestone Roadmap

- [x] **v0.1 – Project Foundation**: Directory layout, CMake build system, core logger, and version definitions.
- [x] **v0.2 – Memory Subsystem**: Encapsulated byte-addressable memory buffer with bounds checking and error handling.
- [x] **v0.3 – MMIO Bus Infrastructure**: Register abstraction and dynamic MMIO address bus mapping.
- [x] **v0.4 – GPIO Peripheral**: 32-pin GPIO peripheral modeling DIR, OUT, and IN registers.
- [x] **v0.5 – Timer Peripheral**: Hardware timer modeling CTRL, COUNT, COMPARE, and STATUS registers with compare match interrupts.
- [x] **v0.6 – Interrupt Controller**: 32-channel priority interrupt controller supporting ISR registration and hardware triggering.
- [x] **v0.7 – CPU Execution Engine**: Cycle-driven CPU coordinator for timers, interrupts, and simulation steps.
- [x] **v0.8 – Firmware Execution Layer**: Abstract firmware interface (`initialize`, `execute`, `shutdown`) and `BasicFirmware` example.
- [x] **v0.9 – CPU Register File**: 16 GPRs (R0–R15), Program Counter (PC), Stack Pointer (SP), and Status Register (SR).
- [x] **v1.0 – Interactive Debugger & Monitor**: Non-blocking CLI monitor enabling step execution, memory dumps, and register inspection.
- [x] **v1.1 – UART Serial Peripheral**: Polling-mode UART with TX/RX FIFOs, baud rate configuration, and MMIO registers.
- [x] **v1.2 – System Bus**: Central communication bus interconnecting CPU, Memory, MMIO, Interrupt Controller, and peripherals.
- [x] **v1.3 – Simulation Clock**: Centralized deterministic timing source for simulation cycles and timing calculations.
- [x] **v1.4 – Event Scheduler**: Deterministic cycle-driven event scheduler for callback execution and peripheral timing.
- [x] **v1.5 – Hardware Abstraction Layer**: Clean, stable `GPIOHAL`, `TimerHAL`, and `UARTHAL` abstractions for firmware peripheral control.
- [x] **v1.5.1 – Continuous Integration & Quality Gates**: Cross-platform GitHub Actions CI matrix pipeline, `-Werror` quality gates, and build status reporting.
- [x] **v1.6 – Firmware Application Framework**: `FirmwareManager`, lifecycle interface (`initialize`, `update`, `shutdown`, `reset`), `TimerBlinkFirmware`, and `UARTEchoFirmware`.
- [x] **v1.7 – Power Management & Reset Controller**: `PowerController` (`ON`, `OFF`, `SLEEP`), `ResetController` (`resetCPU`, `resetPeripherals`, `resetFirmware`, `resetSystem`), and CPU power execution constraints.
- [x] **v1.8 – Direct Memory Access (DMA) Controller**: `DMAController` background byte transfers (Memory-to-Memory, Memory-to-MMIO, MMIO-to-Memory), completion interrupts, and SystemBus integration.

---

## Future Work

- [ ] **v1.9 – Firmware Binary Loader**: ELF and Intel HEX firmware binary image loader.
- [ ] **v2.0 – Advanced Monitor Debugging**: Breakpoints, watchpoints, and symbol table resolution.

---

## License

MIT License. See `LICENSE` for details.
