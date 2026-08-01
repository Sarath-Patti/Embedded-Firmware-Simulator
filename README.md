# Embedded Firmware Simulator

[![Version](https://img.shields.io/badge/version-v1.1.0-blue.svg)](include/common/version.hpp)
[![Standard](https://img.shields.io/badge/c%2B%2B-20-green.svg)](CMakeLists.txt)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

A production-quality, modular C++20 simulator architecture for embedded firmware development, hardware peripheral modeling, interrupt management, and interactive system debugging.

---

## Table of Contents

- [Overview](#overview)
- [Motivation](#motivation)
- [Key Features (v1.1)](#key-features-v11)
- [System Architecture](#system-architecture)
- [UART Peripheral Architecture](#uart-peripheral-architecture)
  - [Register Map](#register-map)
  - [FIFO Design](#fifo-design)
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

## Key Features (v1.1)

- **Memory Subsystem (`efs::memory::Memory`)**: Contiguous byte-addressable memory buffer with strict boundary checking, reset utilities, and error logging.
- **MMIO Bus Infrastructure (`efs::mmio::MMIOBus`, `efs::mmio::Register`)**: Memory-mapped register abstraction enabling peripheral integration and dynamic address dispatching.
- **GPIO Peripheral (`efs::drivers::gpio::GPIO`)**: 32-pin GPIO peripheral modeling Direction (DIR), Output (OUT), and Input (IN) registers.
- **Hardware Timer Peripheral (`efs::drivers::timer::Timer`)**: Configurable hardware timer modeling Control (CTRL), Counter (COUNT), Compare (COMPARE), and Status (STATUS) registers with match interrupts.
- **UART Peripheral (`efs::drivers::uart::UART`)**: Serial communication peripheral modeling DATA, STATUS, CONTROL, and BAUD registers with internal TX and RX FIFOs.
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
           ┌─────────────────┼─────────────────┐
           ▼                 ▼                 ▼
┌─────────────────────┐ ┌─────────┐ ┌─────────────────────┐
│   GPIO Peripheral   │ │  UART   │ │   Timer Peripheral  │
└─────────────────────┘ └─────────┘ └─────────────────────┘
```

---

## UART Peripheral Architecture

The UART (Universal Asynchronous Receiver-Transmitter) peripheral provides serial data transmission and reception capability mapped via the MMIO bus.

```text
                     MMIO Bus
                         │
                         ▼
                  UART Peripheral
         ┌────────────────────────────┐
         │ DATA Register    (0x00)    │
         │ STATUS Register  (0x04)    │
         │ CONTROL Register (0x08)    │
         │ BAUD Register    (0x0C)    │
         └────────────────────────────┘
                  │              │
              TX FIFO        RX FIFO
```

### Register Map

| Register | Offset | Access | Description | Bit Fields |
| :--- | :--- | :--- | :--- | :--- |
| `DATA` | `0x00` | R/W | Transmit & Receive Data Buffer | `[7:0] Data Byte` |
| `STATUS` | `0x04` | R | Peripheral Status Flags | `Bit 0: TX Empty`, `Bit 1: RX Available`, `Bit 2: Enabled` |
| `CONTROL`| `0x08` | R/W | Peripheral Control | `Bit 0: Enable` |
| `BAUD` | `0x0C` | R/W | Baud Rate Configuration | `[31:0] Baud Rate (e.g. 115200)` |

### FIFO Design

- **TX FIFO (`m_txFifo`)**: STL `std::queue` storing outgoing bytes enqueued via `writeByte()`. Setting `STATUS_TX_EMPTY_BIT` when empty.
- **RX FIFO (`m_rxFifo`)**: STL `std::queue` storing incoming bytes enqueued via `pushReceivedByte()` and dequeued via `readByte()`. Setting `STATUS_RX_AVAIL_BIT` when non-empty.

---

## Repository Layout

```text
Embedded-Firmware-Simulator/
├── include/                  # Public C++ header interfaces
│   ├── common/               # Types, logger, and project versioning
│   ├── cpu/                  # CPU cycle engine & RegisterFile (cpu/registers/)
│   ├── drivers/              # Hardware peripheral drivers (gpio/, timer/, uart/)
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
├── examples/                 # Sample applications (firmware_demo.cpp, uart_demo.cpp)
├── tests/                    # CTest unit test suite (10 test suites)
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

### Example Applications
Run standalone simulation demos showcasing firmware execution or UART serial communication:

```bash
./firmware_demo
./uart_demo
```

---

## Running Tests

Execute the full CTest unit test suite covering all 10 project test modules:

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
| `uart` | `uart` | Display UART peripheral state (enabled status, baud, FIFO queue sizes, status flags). |

### Example Session

```text
Embedded Firmware Simulator initialized.
Embedded Firmware Simulator Monitor (v1.0)
Type 'help' for a list of commands.
> uart
UART
----------------------
Enabled : YES

Baud : 115200

TX FIFO : 0 bytes

RX FIFO : 0 bytes

STATUS :

TX Empty : YES

RX Available : NO
> exit
Exiting monitor.
```

---

## Project Statistics

- **Core Subsystems**: Memory, MMIO, GPIO, Timer, UART, Interrupt Controller, CPU, Register File, Firmware, Interactive Monitor
- **Peripherals Modeled**: 3 (`GPIO`, `Hardware Timer`, `UART Serial Interface`)
- **Unit Test Suites**: 10 (`MemoryTests`, `MMIOTests`, `GPIOTests`, `TimerTests`, `InterruptTests`, `CPUTests`, `FirmwareTests`, `RegisterTests`, `MonitorTests`, `UARTTests`)
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

---

## Future Work

- **v1.2 – Instruction Set Architecture (ISA) & Decoder**: Lightweight opcode decoder and instruction pipeline.
- **v1.3 – Firmware Binary Loader**: ELF and Intel HEX firmware binary image loader.
- **v1.4 – Advanced Monitor Debugging**: Breakpoints, watchpoints, and symbol table resolution.

---

## License

This project is licensed under the [MIT License](LICENSE).
