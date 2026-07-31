# Embedded Firmware Simulator

[![Version](https://img.shields.io/badge/version-v1.0.0-blue.svg)](include/common/version.hpp)
[![Standard](https://img.shields.io/badge/c%2B%2B-20-green.svg)](CMakeLists.txt)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

A production-quality, modular C++20 simulator architecture for embedded firmware development, hardware peripheral modeling, interrupt management, and interactive system debugging.

---

## Table of Contents

- [Overview](#overview)
- [Motivation](#motivation)
- [Key Features (v1.0)](#key-features-v10)
- [System Architecture](#system-architecture)
- [Repository Layout](#repository-layout)
- [Build Instructions](#build-instructions)
- [Running the Simulator](#running-the-simulator)
- [Running Tests](#running-tests)
- [Interactive Monitor CLI](#interactive-monitor-cli)
  - [Supported Commands](#supported-commands)
  - [Example Session](#example-session)
- [Project Statistics](#project-statistics)
- [Milestone Roadmap](#milestone-roadmap)
- [Future Work](#future-work)
- [License](#license)

---

## Overview

The **Embedded Firmware Simulator** provides a lightweight, highly extensible C++20 framework designed to simulate an embedded microcontroller environment on host workstations. The platform decouples physical hardware dependencies from firmware design, enabling hardware driver development, interrupt handling verification, cycle-accurate simulation, and memory/register observation with zero external hardware requirements.

---

## Motivation

Developing embedded software typically requires physical microcontroller targets, hardware debug probes (JTAG/SWD), and board support packages. Hardware availability bottlenecks, slow flashing workflows, and limited observability often hinder rapid firmware iteration.

The **Embedded Firmware Simulator** solves these challenges by providing a modular, C++20 simulated system architecture. Hardware peripherals expose standardized MMIO registers, the kernel dispatches priority interrupts, the CPU coordinates cycle simulation steps, and an interactive monitor CLI gives developers complete visibility into system state in real time.

---

## Key Features (v1.0)

- **Memory Subsystem (`efs::memory::Memory`)**: Contiguous byte-addressable memory buffer with strict boundary checking, reset utilities, and error logging.
- **MMIO Bus Infrastructure (`efs::mmio::MMIOBus`, `efs::mmio::Register`)**: Memory-mapped register abstraction enabling peripheral integration and dynamic address dispatching.
- **GPIO Peripheral (`efs::drivers::gpio::GPIO`)**: 32-pin GPIO peripheral modeling Direction (DIR), Output (OUT), and Input (IN) registers.
- **Hardware Timer Peripheral (`efs::drivers::timer::Timer`)**: Configurable hardware timer modeling Control (CTRL), Counter (COUNT), Compare (COMPARE), and Status (STATUS) registers with match interrupts.
- **Interrupt Controller (`efs::kernel::InterruptController`)**: Priority interrupt manager supporting 32 IRQ sources, enabling/disabling, priority dispatching, and ISR registration.
- **CPU Execution Engine (`efs::cpu::CPU`)**: Cycle-based execution loop driving attached peripheral timers, dispatching pending interrupts, and running loaded firmware.
- **Firmware Abstraction Layer (`efs::firmware::Firmware`, `efs::firmware::BasicFirmware`)**: Standardized lifecycle interface (`initialize`, `execute`, `shutdown`) and concrete sample firmware implementations.
- **CPU Register File (`efs::cpu::registers::RegisterFile`)**: Encapsulates processor state including 16 General Purpose Registers (R0–R15), Program Counter (PC), Stack Pointer (SP), and Status Register (SR).
- **Interactive Monitor CLI (`efs::monitor::Monitor`)**: Non-destructive command-line debugger allowing real-time step execution, memory dumps, register inspection, and peripheral status querying.

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
                       └─────┬───────────────┬───┘
                             │               │
                             ▼               ▼
                       ┌───────────┐   ┌───────────┐
                       │ Register  │   │ Firmware  │
                       │   File    │   │ Subsystem │
                       └───────────┘   └───────────┘
                             │
                             ▼
                       ┌───────────┐
                       │ Interrupt │
                       │Controller │
                       └─────┬─────┘
                             │
                             ▼
                       ┌───────────┐
                       │ Memory /  │
                       │ MMIO Bus  │
                       └─────┬─────┘
           ┌─────────────────┴─────────────────┐
           ▼                                   ▼
┌─────────────────────┐             ┌─────────────────────┐
│   GPIO Peripheral   │             │   Timer Peripheral  │
└─────────────────────┘             └─────────────────────┘
```

---

## Repository Layout

```text
Embedded-Firmware-Simulator/
├── include/                  # Public C++ header interfaces
│   ├── common/               # Types, logger, and project versioning
│   ├── cpu/                  # CPU cycle engine & RegisterFile (cpu/registers/)
│   ├── drivers/              # Hardware peripheral drivers (gpio/, timer/)
│   ├── firmware/             # Firmware interface & BasicFirmware
│   ├── kernel/               # Priority Interrupt Controller
│   ├── memory/               # Byte-addressable Memory subsystem
│   ├── mmio/                 # Memory-Mapped I/O Register & MMIOBus
│   └── monitor/              # Interactive Monitor & CLI debugger
├── src/                      # Implementation files matching include/ structure
│   ├── common/
│   ├── cpu/
│   ├── drivers/
│   ├── firmware/
│   ├── kernel/
│   ├── memory/
│   ├── mmio/
│   ├── monitor/
│   └── main.cpp              # Interactive simulator demo entry point
├── examples/                 # Sample applications (firmware_demo.cpp)
├── tests/                    # CTest unit test suite (9 test suites)
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

---

## Running the Simulator

### Interactive Simulator Monitor Demo
Run the main interactive monitor executable to control and observe the simulator in real time:

```bash
./simulator_demo
```

### Firmware Example Application
Run the standalone simulation demo showcasing GPIO pin toggling and timer compare interrupts:

```bash
./firmware_demo
```

---

## Running Tests

Execute the full CTest unit test suite covering all 9 project modules:

```bash
ctest --output-on-failure
```

---

## Interactive Monitor CLI

The `Monitor` subsystem provides a non-blocking interactive command-line interface to control simulation execution and inspect internal processor and peripheral state.

### Supported Commands

| Command | Syntax | Description |
| :--- | :--- | :--- |
| `help` | `help` | Display all supported commands. |
| `exit` | `exit` | Exit the interactive monitor session. |
| `reset` | `reset` | Reset the CPU simulation cycle counter and register file. |
| `step` | `step` | Execute exactly one simulation cycle. |
| `run` | `run <cycles>` | Execute `N` simulation cycles (where `N` is a positive integer). |
| `regs` | `regs` | Display PC, SP, Status Register, and R0–R15 values. |
| `gpio` | `gpio` | Display GPIO pin directions and output/input states. |
| `timer` | `timer` | Display timer state (running status, counter, compare, match flag). |
| `interrupts` | `interrupts` | Display interrupt controller status (enabled, pending, priority levels). |
| `memory` | `memory <addr> <cnt>` | Display `<cnt>` memory bytes starting at `<addr>` in hex. |
| `mmio` | `mmio` | Display all registered MMIO addresses and their current values. |

### Example Session

```text
Embedded Firmware Simulator initialized.
Embedded Firmware Simulator Monitor (v1.0)
Type 'help' for a list of commands.
> step
Stepped 1 cycle. Current cycle: 1
> regs
=== CPU Registers ===
PC:     0x00000000
SP:     0x00000000
Status: 0x00000000
R0:     0x00000000	R1:     0x00000000	R2:     0x00000000	R3:     0x00000000
R4:     0x00000000	R5:     0x00000000	R6:     0x00000000	R7:     0x00000000
R8:     0x00000000	R9:     0x00000000	R10:    0x00000000	R11:    0x00000000
R12:    0x00000000	R13:    0x00000000	R14:    0x00000000	R15:    0x00000000
> run 5
Ran 5 cycles. Total cycle count: 6
> gpio
=== GPIO Peripheral State ===
Base Address: 0x40000000
  Pin 0: LOW
  Pin 1: HIGH
  Pin 2: LOW
  ...
> memory 0x40000000 12
=== Memory Dump (0x40000000 - 12 bytes) ===
0x40000000: 02 00 00 00 02 00 00 00 02 00 00 00 
> exit
Exiting monitor.
```

---

## Project Statistics

- **Core Subsystems**: Memory, MMIO, GPIO, Timer, Interrupt Controller, CPU, Register File, Firmware, Interactive Monitor
- **Peripherals Modeled**: 2 (`GPIO`, `Hardware Timer`)
- **Unit Test Suites**: 9 (`MemoryTests`, `MMIOTests`, `GPIOTests`, `TimerTests`, `InterruptTests`, `CPUTests`, `FirmwareTests`, `RegisterTests`, `MonitorTests`)
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

---

## Future Work

- **v1.1 – UART Serial Interface**: Universal Asynchronous Receiver-Transmitter peripheral with RX/TX FIFOs.
- **v1.2 – Instruction Set Architecture (ISA) & Decoder**: Lightweight opcode decoder and instruction pipeline.
- **v1.3 – Firmware Binary Loader**: ELF and Intel HEX firmware binary image loader.
- **v1.4 – Advanced Monitor Debugging**: Breakpoints, watchpoints, and symbol table resolution.

---

## License

This project is licensed under the [MIT License](LICENSE).
