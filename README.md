# Embedded Firmware Simulator

[![CI Status](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml/badge.svg)](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml)

A high-performance, modular **C++20 Embedded Firmware Simulator** designed for bare-metal firmware development, hardware abstraction layer (HAL) validation, hardware peripheral simulation, deterministic timing analysis, low-power state management, Direct Memory Access (DMA) transfers, Serial Peripheral Interface (SPI) communication, and real-time interactive debugging without physical hardware.

---

## Overview

Developing and testing embedded firmware on physical hardware is often constrained by limited debugging visibility, slow flashing cycles, complex hardware availability, and non-deterministic timing environments.

The **Embedded Firmware Simulator** solves these challenges by providing a modular, C++20 simulated system architecture. Hardware peripherals expose standardized MMIO registers, the kernel dispatches priority interrupts, a Direct Memory Access (DMA) controller streams data between memory and peripherals, a Serial Peripheral Interface (SPI) master controller coordinates full-duplex communication with simulated SPI slave devices, the CPU coordinates cycle simulation steps, a cycle-driven Event Scheduler handles asynchronous peripheral events, a Hardware Abstraction Layer (HAL) isolates firmware from hardware details, a central FirmwareManager manages firmware application lifecycles, a Power Management & Reset Controller subsystem models system power states and reset behaviors, and an interactive monitor CLI gives developers complete visibility into system state in real time.

---

## Key Features (v1.9.0)

- **Serial Peripheral Interface (SPI) Controller (`efs::drivers::spi::SPIController`)**: Synchronous full-duplex SPI master controller supporting configurable clock dividers, SPI modes (Mode 0, 1, 2, 3), and integration with simulated SPI slave devices (`efs::drivers::spi::SPIDevice`).
- **SPI Hardware Abstraction Layer (`efs::hal::SPIHAL`)**: Clean firmware HAL abstraction isolating firmware applications from underlying raw SPI peripheral MMIO registers via `writeByte()`, `readByte()`, `transfer()`, and `configure()`.
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
- **Hardware Abstraction Layer (`efs::hal`)**: Provides clean, stable `GPIOHAL`, `TimerHAL`, `UARTHAL`, and `SPIHAL` abstractions hiding raw peripheral implementation details from firmware.
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
                    ┌────────────────────┼────────────────────┐
                    ▼                    ▼                    ▼
           ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
           │ GPIO Peripheral │  │ TimerPeripheral │  │ UART Peripheral │
           └─────────────────┘  └─────────────────┘  └────────┬────────┘
                                                              │
                                                              ▼
                                                     ┌─────────────────┐
                                                     │ SPI Controller  │
                                                     └────────┬────────┘
                                                              │
                                                              ▼
                                                     ┌─────────────────┐
                                                     │ Simulated SPI   │
                                                     │ Slave Device    │
                                                     └─────────────────┘
```

---

## SPI Architecture & Transfer Flow

The Serial Peripheral Interface (`efs::drivers::spi::SPIController`) manages full-duplex synchronous byte transfers between master controller and attached simulated SPI slave devices (`efs::drivers::spi::SPIDevice`). Firmware interacts with SPI via `efs::hal::SPIHAL`.

### Architecture

```text
                 Firmware Application
                          │
                          ▼
                       SPI HAL
                          │
                          ▼
                    SPI Controller (MMIO 0x40004000)
                          │
                     System Bus
                          │
                     MMIO Bus
                          │
                 Simulated SPI Device
```

### Register Layout

| Offset | Register Name | Description |
|---|---|---|
| `0x00` | DATA | Transmit/Receive data register (8-bit / 32-bit) |
| `0x04` | STATUS | Status bits: Enable (bit 0), Busy (bit 1), RX Available (bit 2) |
| `0x08` | CONTROL | Control bits: Enable (bit 0), Mode 0..3 (bits 1..2) |
| `0x0C` | CLOCK_DIV | Prescaler clock divider (default: 2) |

### Transfer Flow

```text
1. spiHAL.configure(mode, div) ──► 2. spiHAL.enable() ──► 3. spiHAL.transfer(txByte) ──► 4. rxByte returned
```

1. **Configuration**: `configure(mode, clockDivider)` sets SPI mode (0..3) and clock prescaler divider.
2. **Enable**: `enable()` sets `CTRL_ENABLE_BIT` in the control register.
3. **Full-Duplex Transfer**: `transfer(txByte)` sends `txByte` to the attached SPI slave device, receives the slave's response byte, updates the DATA register, and stores the response in the RX buffer.
4. **Firmware Retrieval**: `readByte()` returns enqueued received bytes from the RX buffer.

### Example Usage

```cpp
efs::mmio::MMIOBus mmioBus;
efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

// Instantiate SPI controller at MMIO base 0x40004000
efs::drivers::spi::SPIController spi(mmioBus, 0x40004000);
systemBus.attachSPI(&spi);

// Attach simulated SPI slave device
efs::drivers::spi::SimulatedSPIDevice slave(0x55 /* default response */);
spi.attachSlave(&slave);

// Use SPI HAL in firmware
efs::hal::SPIHAL spiHAL(&spi);
spiHAL.enable();
spiHAL.configure(0 /* Mode 0 */, 4 /* Divider 4 */);

// Perform full-duplex transfer
std::uint8_t rx = spiHAL.transfer(0xA5);
assert(rx == 0x55);
assert(slave.lastReceivedByte() == 0xA5);
```

---

## Building and Running

### Build Prerequisites

- C++20 compliant compiler (GCC 10+, Clang 11+, or MSVC 2019+)
- CMake 3.16+
- Make or Ninja

### Local Build Instructions

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DEFS_ENABLE_WERROR=ON
cmake --build .
ctest --output-on-failure
```

---

## License

This project is licensed under the MIT License. See `LICENSE` for details.
