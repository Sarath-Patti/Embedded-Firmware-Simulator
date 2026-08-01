# Embedded Firmware Simulator

[![Build Status](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml/badge.svg)](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions)
[![Version](https://img.shields.io/badge/version-v1.5.1-blue.svg)](include/common/version.hpp)
[![Standard](https://img.shields.io/badge/c%2B%2B-20-green.svg)](CMakeLists.txt)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

A production-quality, modular C++20 simulator architecture for embedded firmware development, hardware peripheral modeling, interrupt management, and interactive system debugging.

---

## Table of Contents

- [Overview](#overview)
- [Motivation](#motivation)
- [Key Features (v1.5.1)](#key-features-v151)
- [System Architecture](#system-architecture)
- [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
- [Firmware Development Model](#firmware-development-model)
- [Event Scheduler Architecture](#event-scheduler-architecture)
- [Simulation Clock Architecture](#simulation-clock-architecture)
- [System Bus Architecture](#system-bus-architecture)
- [UART Peripheral Architecture](#uart-peripheral-architecture)
  - [Register Map](#register-map)
  - [FIFO Design](#fifo-design)
- [Continuous Integration & Quality Gates](#continuous-integration--quality-gates)
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

## Key Features (v1.5.1)

- **Continuous Integration Pipeline**: Automated multi-platform GitHub Actions CI matrix testing across Ubuntu, macOS, and Windows with strict compiler warning enforcement (`-Werror` / `/WX`).
- **Hardware Abstraction Layer (`efs::hal`)**: Provides clean, stable `GPIOHAL`, `TimerHAL`, and `UARTHAL` abstractions hiding raw peripheral implementation details from firmware.
- **Firmware Development Model**: Standardized application lifecycle (`initialize`, `execute`, `shutdown`) operating exclusively via HAL interfaces without direct peripheral coupling.
- **Event Scheduler (`efs::system::scheduler::EventScheduler`)**: Deterministic cycle-driven event scheduler for executing callbacks at specific simulation times with strict FIFO ordering for simultaneous events.
- **Simulation Clock (`efs::system::clock::SimulationClock`)**: Centralized deterministic timing source maintaining simulated cycle counts and calculating elapsed nanoseconds, microseconds, and milliseconds.
- **System Bus Architecture (`efs::system::SystemBus`)**: Centralized communication layer interconnecting CPU, Memory, MMIO Bus, Interrupt Controller, Simulation Clock, Event Scheduler, and Hardware Peripherals.
- **Memory Subsystem (`efs::memory::Memory`)**: Contiguous byte-addressable memory buffer with strict boundary checking, reset utilities, and error logging.
- **MMIO Bus Infrastructure (`efs::mmio::MMIOBus`, `efs::mmio::Register`)**: Memory-mapped register abstraction enabling peripheral integration and dynamic address dispatching.
- **GPIO Peripheral (`efs::drivers::gpio::GPIO`)**: 32-pin GPIO peripheral modeling Direction (DIR), Output (OUT), and Input (IN) registers.
- **Hardware Timer Peripheral (`efs::drivers::timer::Timer`)**: Configurable hardware timer modeling Control (CTRL), Counter (COUNT), Compare (COMPARE), and Status (STATUS) registers with match interrupts driven by scheduled events.
- **UART Peripheral (`efs::drivers::uart::UART`)**: Serial communication peripheral modeling DATA, STATUS, CONTROL, and BAUD registers with internal TX and RX FIFOs.
- **Interrupt Controller (`efs::kernel::InterruptController`)**: Priority interrupt manager supporting 32 IRQ sources, enabling/disabling, priority dispatching, and ISR registration.
- **CPU Execution Engine (`efs::cpu::CPU`)**: Cycle-based execution loop connected directly to the System Bus, driving the Simulation Clock, executing ready scheduled events, dispatching pending interrupts, and running loaded firmware.
- **Firmware Abstraction Layer (`efs::firmware::Firmware`, `efs::firmware::BasicFirmware`)**: Standardized lifecycle interface (`initialize`, `execute`, `shutdown`) and concrete sample firmware implementations.
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
                       └────────────┬────────────┘
                                    │
                                    ▼
                       ┌─────────────────────────┐
                       │        Firmware         │
                       └────────────┬────────────┘
                                    │
                                    ▼
                       ┌─────────────────────────┐
                       │Hardware Abstraction Layer│ (GPIOHAL, TimerHAL, UARTHAL)
                       └────────────┬────────────┘
                                    │
                                    ▼
                       ┌─────────────────────────┐
                       │       System Bus        │
                       └─────┬───────────┬───────┘
                             │           │
           ┌─────────────────┼───────────┴─────────────────┐
           ▼                 ▼                             ▼
┌─────────────────────┐ ┌─────────┐             ┌─────────────────────┐
│   Memory Subsystem  │ │  MMIO   │             │Interrupt Controller │
└─────────────────────┘ └────┬────┘             └─────────────────────┘
                             │
         ┌───────────────────┼───────────────────┐
         ▼                   ▼                   ▼
┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐
│ GPIO Peripheral │ │ TimerPeripheral │ │ UART Peripheral │
└─────────────────┘ └─────────────────┘ └─────────────────┘
```

---

## Continuous Integration & Quality Gates

The project maintains a production-grade GitHub Actions CI pipeline (`.github/workflows/ci.yml`):

```text
               GitHub Push / Pull Request
                           │
                           ▼
          ┌────────────────────────────────┐
          │  Multi-Platform CI Matrix Loop │
          ├────────────────────────────────┤
          │ - Ubuntu Latest (GCC/Clang)    │
          │ - macOS Latest (Apple Clang)   │
          │ - Windows Latest (MSVC)        │
          └────────────────┬───────────────┘
                           │
                           ▼
          ┌────────────────────────────────┐
          │   Quality Gate Checks (-Werror)│
          ├────────────────────────────────┤
          │ 1. CMake Configuration         │
          │ 2. Parallel Multi-Target Build │
          │ 3. All Demo Executables Built  │
          │ 4. CTest Suite (14 Test Suites)│
          └────────────────────────────────┘
```

- **Cross-Platform Matrix**: Automated builds on Linux, macOS, and Windows.
- **Strict Warning Management**: Compiles with `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4` (MSVC). Passing `-DEFS_ENABLE_WERROR=ON` in CI treats warnings as errors without breaking local developer flexibility.
- **Immediate Failure Enforcement**: Tests run via `ctest --output-on-failure`, halting the build immediately if any assertion or test fails.

---

## Hardware Abstraction Layer (HAL)

The Hardware Abstraction Layer (`efs::hal`) provides a high-level, production-grade interface between firmware and hardware peripherals:

```text
                Firmware Application
                         │
                         ▼
             Hardware Abstraction Layer
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
     GPIOHAL          TimerHAL         UARTHAL
```

---

## Repository Layout

```text
Embedded-Firmware-Simulator/
├── .github/                  # GitHub Actions CI workflow definitions
│   └── workflows/
│       └── ci.yml            # CI build & test pipeline matrix
├── include/                  # Public C++ header interfaces
├── src/                      # Implementation files matching include/ structure
├── examples/                 # Sample applications (firmware_demo, uart_demo, event_scheduler_demo, hal_demo)
├── tests/                    # CTest unit test suite (14 test suites)
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
```

---

## Running Tests

Execute the full CTest unit test suite covering all 14 project test modules:

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

- **Core Subsystems**: HAL (`GPIOHAL`, `TimerHAL`, `UARTHAL`), Event Scheduler, Simulation Clock, System Bus, Memory, MMIO, GPIO, Timer, UART, Interrupt Controller, CPU, Register File, Firmware, Interactive Monitor
- **Peripherals Modeled**: 3 (`GPIO`, `Hardware Timer`, `UART Serial Interface`)
- **Unit Test Suites**: 14 (`MemoryTests`, `MMIOTests`, `GPIOTests`, `TimerTests`, `InterruptTests`, `CPUTests`, `FirmwareTests`, `RegisterTests`, `MonitorTests`, `UARTTests`, `SystemTests`, `ClockTests`, `SchedulerTests`, `HALTests`)
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

---

## Future Work

- **v1.6 – Firmware Binary Loader**: ELF and Intel HEX firmware binary image loader.
- **v1.7 – Advanced Monitor Debugging**: Breakpoints, watchpoints, and symbol table resolution.

---

## License

This project is licensed under the [MIT License](LICENSE).
