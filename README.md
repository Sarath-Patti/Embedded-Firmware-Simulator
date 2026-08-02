# Embedded Firmware Simulator (EFS)

![CI Status](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/cmake.yml/badge.svg)
![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

A modular, hardware-accurate C++17 simulator for bare-metal firmware, peripheral drivers, and Real-Time Operating Systems (RTOS).

---

## Project Overview

The **Embedded Firmware Simulator (EFS)** provides a full-featured, virtual hardware platform designed for developing, testing, and debugging embedded firmware applications without physical hardware.

Key benefits:
- Hardware-accurate 32-bit Memory-Mapped I/O (MMIO) bus decoding.
- Full suite of peripheral simulation models (GPIO, Timer, UART, SPI, I2C, PWM, ADC, DMA).
- Simulated Real-Time Operating System (RTOS) priority-preemptive task scheduler.
- Hardware Abstraction Layer (HAL) interfaces for portable application development.
- Zero external dependencies beyond standard C++17 compilers and CMake.

---

## Features

- **CPU Simulation Core**: Cycle-accurate cycle counting, step execution, and power state restrictions.
- **MMIO Bus Engine**: 32-bit flat address decoding with out-of-range safety checks.
- **Interrupt Controller (VIC)**: Vector Interrupt Controller with priority handling and nested ISR dispatches.
- **RTOS Scheduler**: Priority-based task scheduling with support for task suspension, resumption, yielding, and preemptive context switches.
- **Rich Peripheral Set**:
  - **GPIO**: Pin direction, state, and mode configuration.
  - **Timer**: 32-bit counter with auto-reset and compare match interrupts.
  - **UART**: Full-duplex serial communication with hardware FIFOs and baud rate control.
  - **SPI Master**: CPOL/CPHA mode selection, clock prescaling, and simulated slave device integration.
  - **I2C Master**: 7-bit addressing, START/STOP transactions, and ACK/NACK status handling.
  - **PWM**: Frequency and duty cycle generation (0–100%).
  - **ADC**: 12-bit analog sampling (0–4095) with multi-channel support.
  - **DMA**: Non-blocking memory-to-memory and peripheral DMA transfers.
- **System Monitor**: Interactive CLI interface for memory dumps, register inspection, and RTOS task tracking.

---

## Architecture Overview

```
+-------------------------------------------------------------------------+
|                              CPU Core                                   |
|  +-------------------+  +-------------------+  +---------------------+  |
|  |   RegisterFile    |  |  PowerController  |  |  FirmwareManager    |  |
|  +-------------------+  +-------------------+  +---------------------+  |
+-------------------------------------------------------------------------+
                                   |
                                   v
+-------------------------------------------------------------------------+
|                             SystemBus                                   |
|  +-------------------+  +-------------------+  +---------------------+  |
|  |     Memory        |  |     MMIOBus       |  | InterruptController |  |
|  +-------------------+  +-------------------+  +---------------------+  |
|  |  EventScheduler   |  |   DMAController   |  |   SimulationClock   |  |
|  +-------------------+  +-------------------+  +---------------------+  |
+-------------------------------------------------------------------------+
                                   |
            +----------------------+----------------------+
            |                      |                      |
            v                      v                      v
   +-----------------+    +-----------------+    +-----------------+
   | GPIO Peripheral |    | Timer Peripheral|    | UART Peripheral |
   +-----------------+    +-----------------+    +-----------------+
            |                      |                      |
            v                      v                      v
   +-----------------+    +-----------------+    +-----------------+
   |   SPI / I2C     |    |    PWM / ADC    |    |  RTOS Scheduler |
   +-----------------+    +-----------------+    +-----------------+
```

---

## Memory Map

| Peripheral | Base Address | Offset Range | Description |
| :--- | :--- | :--- | :--- |
| **System RAM** | `0x00000000` | `0x00000000` – `0x3FFFFFFF` | Configurable RAM |
| **GPIO** | `0x40000000` | `0x00` – `0x0C` | General Purpose I/O |
| **Timer** | `0x40001000` | `0x00` – `0x0C` | 32-bit Timer / Counter |
| **Interrupt Controller** | `0x40002000` | `0x00` – `0x0C` | Interrupt Controller |
| **UART** | `0x40003000` | `0x00` – `0x0C` | Serial Communication |
| **DMA** | `0x40004000` | `0x00` – `0x10` | Direct Memory Access |
| **SPI** | `0x40004800` | `0x00` – `0x0C` | SPI Master Controller |
| **I2C** | `0x40005000` | `0x00` – `0x0C` | I2C Master Controller |
| **PWM** | `0x40006000` | `0x00` – `0x0C` | Pulse Width Modulation |
| **ADC** | `0x40007000` | `0x00` – `0x0C` | Analog Digital Converter |

---

## Directory Structure

```
Embedded-Firmware-Simulator/
├── CMakeLists.txt              # CMake build specification
├── README.md                   # Main documentation
├── CONTRIBUTING.md             # Development and contribution guide
├── CHANGELOG.md                # Project release history
├── LICENSE                     # MIT License
├── docs/                       # Detailed subsystem documentation
│   ├── architecture.md
│   ├── firmware.md
│   ├── hal.md
│   ├── memory-map.md
│   ├── peripherals.md
│   ├── scheduler.md
│   └── monitor.md
├── include/                    # Public C++ header files
│   ├── common/
│   ├── cpu/
│   ├── drivers/
│   ├── firmware/
│   ├── hal/
│   ├── kernel/
│   ├── memory/
│   ├── mmio/
│   ├── monitor/
│   ├── rtos/
│   └── system/
├── src/                        # C++ source files
├── examples/                   # 12 Hardware demo applications
└── tests/                      # 22 CTest unit test suites
```

---

## Build Instructions

### Prerequisites
- C++17 compatible compiler (AppleClang 12+, GCC 9+, or MSVC 2019+)
- CMake 3.15 or higher

### Build Commands

```bash
# Configure and build Release mode with strict -Werror
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DEFS_ENABLE_WERROR=ON
cmake --build build

# Configure and build Debug mode
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DEFS_ENABLE_WERROR=ON
cmake --build build-debug
```

---

## Test Instructions

Run the 22 automated test suites via CTest:

```bash
# Run unit tests
ctest --test-dir build --output-on-failure
```

---

## Running Demos

The repository includes 12 executable hardware demo applications:

```bash
./build/simulator_demo
./build/firmware_demo
./build/uart_demo
./build/event_scheduler_demo
./build/hal_demo
./build/firmware_manager_demo
./build/power_demo
./build/dma_demo
./build/spi_demo
./build/i2c_demo
./build/pwm_demo
./build/rtos_demo
```

---

## CI Status

The GitHub Actions workflow validates every commit on:
- **Ubuntu Linux** (GCC 11+, Clang 14+)
- **macOS** (Apple Clang 14+)
- **Windows** (MSVC 2022)

---

## Future Roadmap

- [ ] RISC-V RV32I Instruction Set Simulator (ISS) decoder integration.
- [ ] GDB Remote Serial Protocol (RSP) stub interface for hardware step debugging.
- [ ] WebAssembly target port for web browser execution.
