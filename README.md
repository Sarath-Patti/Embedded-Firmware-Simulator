# Embedded Firmware Simulator

[![Version](https://img.shields.io/badge/version-v1.5.0-blue.svg)](include/common/version.hpp)
[![Standard](https://img.shields.io/badge/c%2B%2B-20-green.svg)](CMakeLists.txt)
[![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](LICENSE)

A production-quality, modular C++20 simulator architecture for embedded firmware development, hardware peripheral modeling, interrupt management, and interactive system debugging.

---

## Table of Contents

- [Overview](#overview)
- [Motivation](#motivation)
- [Key Features (v1.5)](#key-features-v15)
- [System Architecture](#system-architecture)
- [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
- [Firmware Development Model](#firmware-development-model)
- [Event Scheduler Architecture](#event-scheduler-architecture)
- [Simulation Clock Architecture](#simulation-clock-architecture)
- [System Bus Architecture](#system-bus-architecture)
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

## Key Features (v1.5)

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
  configureOutput()    start()        writeByte()
  configureInput()     stop()         readByte()
  write()              reset()        hasData()
  read()               counter()      setBaudRate()
  toggle()             setCompare()
```

- **`GPIOHAL`**: Wraps GPIO hardware pin configuration, reading, writing, and toggling.
- **`TimerHAL`**: Wraps hardware timer controls, counter querying, and compare match configuration.
- **`UARTHAL`**: Wraps serial byte transmission, reception, status polling, and baud rate configuration.

---

## Firmware Development Model

Firmware applications inherit from `efs::firmware::Firmware` and interact with peripherals **exclusively** through HAL abstractions:

```cpp
#include "hal/hal.hpp"
#include "firmware/firmware.hpp"

class UserFirmware : public efs::firmware::Firmware {
public:
    UserFirmware(efs::hal::GPIOHAL& gpio, efs::hal::UARTHAL& uart)
        : m_gpio(gpio), m_uart(uart) {}

    void initialize() override {
        m_gpio.configureOutput(13);
        m_uart.setBaudRate(115200);
        m_uart.enable();
    }

    void execute() override {
        m_gpio.toggle(13);
        m_uart.writeByte('A');
    }

    void shutdown() override {
        m_gpio.write(13, false);
    }

private:
    efs::hal::GPIOHAL& m_gpio;
    efs::hal::UARTHAL& m_uart;
};
```

---

## Event Scheduler Architecture

The `EventScheduler` (`efs::system::scheduler::EventScheduler`) provides a deterministic mechanism to schedule and execute callbacks at precise simulation cycles:

```text
               SystemBus / SimulationClock
                            │
                            ▼
                     EventScheduler
             ┌──────────────┼──────────────┐
             ▼              ▼              ▼
       Timer Compare  ADC Complete  Custom Callbacks
```

- **Scheduling & Cancellation**: `schedule(callback, cycle, description)` returns a unique `EventId`. Events can be dynamically canceled via `cancel(eventId)`.
- **FIFO Execution Order**: Multiple events scheduled for the same simulation cycle execute in strict FIFO order (sorted by insertion ID).
- **Timer Integration**: Hardware Timers schedule compare match callbacks with the `EventScheduler` instead of polling every cycle.

---

## Simulation Clock Architecture

The `SimulationClock` (`efs::system::clock::SimulationClock`) provides a deterministic, zero-overhead notion of simulated time for the entire system:

- **Configurable Frequency**: Default frequency of 1 MHz (configurable to any non-zero Hz).
- **Cycle & Time Tracking**: Maintains total cycle count and provides conversions for elapsed nanoseconds (`elapsedNanoseconds()`), microseconds (`elapsedMicroseconds()`), and milliseconds (`elapsedMilliseconds()`).
- **Peripheral Integration**: Peripherals calculate cycle deltas to update state deterministically without relying on host wall-clock time or sleeping threads.

---

## System Bus Architecture

The `SystemBus` (`efs::system::SystemBus`) acts as the primary interconnect in the simulator hierarchy. It decouples the CPU execution engine from individual memory and peripheral details:

- **Centralized Subsystem Management**: Provides high-cohesion accessors (`memory()`, `mmio()`, `interrupts()`, `clock()`, `scheduler()`) for primary platform services.
- **Peripheral Registration**: Allows hardware peripherals (`GPIO`, `Timer`, `UART`) to register cleanly without modifying peripheral implementations or public APIs.
- **Timer Ticking**: Centralizes cycle-based timer ticking (`tickTimers()`) invoked directly by the CPU execution loop.

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

---

## Repository Layout

```text
Embedded-Firmware-Simulator/
├── include/                  # Public C++ header interfaces
│   ├── common/               # Types, logger, and project versioning
│   ├── cpu/                  # CPU cycle engine & RegisterFile (cpu/registers/)
│   ├── drivers/              # Hardware peripheral drivers (gpio/, timer/, uart/)
│   ├── firmware/             # Firmware interface & BasicFirmware
│   ├── hal/                  # Hardware Abstraction Layer (gpio_hal, timer_hal, uart_hal)
│   ├── kernel/               # Priority Interrupt Controller
│   ├── memory/               # Byte-addressable Memory subsystem
│   ├── mmio/                 # Memory-Mapped I/O Register & MMIOBus
│   ├── monitor/              # Interactive Monitor & CLI debugger
│   └── system/               # SystemBus, SimulationClock (clock/), EventScheduler (scheduler/)
├── src/                      # Implementation files matching include/ structure
│   ├── common/
│   ├── cpu/
│   ├── drivers/
│   ├── firmware/
│   ├── hal/
│   ├── kernel/
│   ├── memory/
│   ├── mmio/
│   ├── monitor/
│   ├── system/
│   └── main.cpp              # Interactive simulator demo entry point
├── examples/                 # Sample applications (firmware_demo.cpp, uart_demo.cpp, event_scheduler_demo.cpp, hal_demo.cpp)
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
ctest --output-on-failure
```

---

## Interactive Monitor CLI

The `Monitor` subsystem provides a non-blocking interactive command-line interface to control simulation execution and inspect internal processor and peripheral state.

---

## Project Statistics

- **Core Subsystems**: HAL (`GPIOHAL`, `TimerHAL`, `UARTHAL`), Event Scheduler, Simulation Clock, System Bus, Memory, MMIO, GPIO, Timer, UART, Interrupt Controller, CPU, Register File, Firmware, Interactive Monitor
- **Peripherals Modeled**: 3 (`GPIO`, `Hardware Timer`, `UART Serial Interface`)
- **Unit Test Suites**: 14 (`MemoryTests`, `MMIOTests`, `GPIOTests`, `TimerTests`, `InterruptTests`, `CPUTests`, `FirmwareTests`, `RegisterTests`, `MonitorTests`, `UARTTests`, `SystemTests`, `ClockTests`, `SchedulerTests`, `HALTests`)
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

---

## Future Work

- **v1.6 – Firmware Binary Loader**: ELF and Intel HEX firmware binary image loader.
- **v1.7 – Advanced Monitor Debugging**: Breakpoints, watchpoints, and symbol table resolution.

---

## License

This project is licensed under the [MIT License](LICENSE).
