# Embedded Firmware Simulator

[![CI Status](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml/badge.svg)](https://github.com/Sarath-Patti/Embedded-Firmware-Simulator/actions/workflows/ci.yml)

A high-performance, modular **C++20 Embedded Firmware Simulator** designed for bare-metal firmware development, real-time operating system (RTOS) task scheduling, hardware abstraction layer (HAL) validation, hardware peripheral simulation, deterministic timing analysis, low-power state management, Direct Memory Access (DMA) transfers, Serial Peripheral Interface (SPI) communication, Inter-Integrated Circuit (I²C) master communication, Pulse Width Modulation (PWM) signal generation, Analog-to-Digital Converter (ADC) multi-channel sampling, and real-time interactive debugging without physical hardware.

---

## Overview

Developing and testing embedded firmware on physical hardware is often constrained by limited debugging visibility, slow flashing cycles, complex hardware availability, and non-deterministic timing environments.

The **Embedded Firmware Simulator** solves these challenges by providing a modular, C++20 simulated system architecture. Hardware peripherals expose standardized MMIO registers, the kernel dispatches priority interrupts, a Direct Memory Access (DMA) controller streams data between memory and peripherals, SPI, I²C, PWM, and ADC controllers manage peripheral communication and signal sampling, a simulated Real-Time Operating System (RTOS) Task Scheduler coordinates multi-task execution lifecycles, the CPU coordinates cycle simulation steps, a cycle-driven Event Scheduler handles asynchronous peripheral events, a Hardware Abstraction Layer (HAL) isolates firmware from hardware details, a central FirmwareManager manages firmware application lifecycles, a Power Management & Reset Controller subsystem models system power states and reset behaviors, and an interactive monitor CLI gives developers complete visibility into system state in real time.

---

## Key Features (v2.4.0)

- **Simulated RTOS Task Scheduler (`efs::rtos::Scheduler`)**: Preemptive priority-based multi-task scheduler supporting round-robin scheduling among equal priorities, tick delay queues, suspended task management, and an automatic Idle Task.
- **Task Control Block (`efs::rtos::TaskControlBlock`, `efs::rtos::Task`)**: Standardized task lifecycle model tracking task state (`READY`, `RUNNING`, `BLOCKED`, `SUSPENDED`, `TERMINATED`), priority levels (0–255), stack allocation, and execution metrics.
- **Analog-to-Digital Converter (ADC) Controller (`efs::drivers::adc::ADCController`)**: Multi-channel ADC peripheral supporting configurable resolution (8-bit, 10-bit, 12-bit), reference voltage ($V_{\text{ref}}$), input voltage sampling with saturation handling, and MMIO register integration.
- **ADC Hardware Abstraction Layer (`efs::hal::ADCHAL`)**: Clean firmware HAL abstraction isolating firmware applications from raw ADC MMIO registers via `read()`, `enable()`, `disable()`, and `setReferenceVoltage()`.
- **Pulse Width Modulation (PWM) Controller (`efs::drivers::pwm::PWMController`)**: Signal generator driven internally by `efs::drivers::timer::Timer`, supporting configurable signal frequency (Hz), duty cycle percentage (0–100%), output pin state evaluation, and MMIO register integration.
- **PWM Hardware Abstraction Layer (`efs::hal::PWMHAL`)**: Clean firmware HAL abstraction isolating firmware applications from raw PWM registers via `setDutyCycle()`, `setFrequency()`, `enable()`, `disable()`, and `outputState()`.
- **Inter-Integrated Circuit (I²C) Controller (`efs::drivers::i2c::I2CController`)**: Master-driven I²C bus controller supporting 7-bit slave addressing, START/STOP condition generation, ACK/NACK verification, multi-device attachment, and MMIO register integration.
- **I²C Hardware Abstraction Layer (`efs::hal::I2CHAL`)**: Firmware abstraction supporting Arduino/Wire-style transactions (`beginTransmission()`, `endTransmission()`, `writeByte()`, `readByte()`, `requestFrom()`, `available()`).
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
- **Hardware Abstraction Layer (`efs::hal`)**: Provides clean, stable `GPIOHAL`, `TimerHAL`, `UARTHAL`, `SPIHAL`, `I2CHAL`, `PWMHAL`, and `ADCHAL` abstractions hiding raw peripheral implementation details from firmware.
- **Event Scheduler (`efs::system::scheduler::EventScheduler`)**: Deterministic cycle-driven event scheduler for executing callbacks at specific simulation times with strict FIFO ordering for simultaneous events.
- **Simulation Clock (`efs::system::clock::SimulationClock`)**: Centralized deterministic timing source maintaining simulated cycle counts and calculating elapsed nanoseconds, microseconds, and milliseconds.
- **System Bus Architecture (`efs::system::SystemBus`)**: Centralized communication layer interconnecting CPU, Memory, MMIO Bus, Interrupt Controller, Simulation Clock, Event Scheduler, Power Subsystem, DMA Controller, and Hardware Peripherals.
- **Memory Subsystem (`efs::memory::Memory`)**: Contiguous byte-addressable memory buffer with strict boundary checking, reset utilities, and error logging.
- **MMIO Bus Infrastructure (`efs::mmio::MMIOBus`, `efs::mmio::Register`)**: Memory-mapped register abstraction enabling peripheral integration and dynamic address dispatching.
- **GPIO Peripheral (`efs::drivers::gpio::GPIO`)**: 32-pin GPIO peripheral modeling Direction (DIR), Output (OUT), and Input (IN) registers.
- **Hardware Timer Peripheral (`efs::drivers::timer::Timer`)**: Configurable hardware timer modeling Control (CTRL), Counter (COUNT), Compare (COMPARE), and Status (STATUS) registers with match interrupts driven by scheduled events.
- **UART Peripheral (`efs::drivers::uart::UART`)**: Serial communication peripheral modeling DATA, STATUS, CONTROL, and BAUD registers with internal TX and RX FIFOs.
- **Interrupt Controller (`efs::kernel::InterruptController`)**: Priority interrupt manager supporting 32 IRQ sources, enabling/disabling, priority dispatching, and ISR registration.
- **CPU Execution Engine (`efs::cpu::CPU`)**: Cycle-based execution loop connected directly to the System Bus, driving the Simulation Clock, executing ready scheduled events, dispatching pending interrupts, stepping DMA transfers, running RTOS tasks, and managing active firmware.
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
│ PowerController (ON/OFF) │                    │  RTOS Scheduler   │
└─────────────┬────────────┘                    └─────────┬─────────┘
              │                                           │
              ▼                                           ▼
┌──────────────────────────┐                   ┌────────────────────┐
│     ResetController      │                   │   Firmware Tasks   │
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
       ┌────────────────┬────────────────┼──────────────┬────────────────┐
       ▼                ▼                ▼              ▼                ▼
┌──────────────┐ ┌──────────────┐   ┌─────────┐  ┌──────────────┐ ┌──────────────┐
│ GPIO Periph  │ │ Timer Periph │   │  UART   │  │ SPI Controlr │ │ ADC Controlr │
└──────────────┘ └──────────────┘   └─────────┘  └──────────────┘ └──────────────┘
```

---

## RTOS Task Scheduler Architecture

The RTOS Task Scheduler (`efs::rtos::Scheduler`) provides lightweight simulated multi-tasking without operating system host threads.

### Task Lifecycle & States

```text
  [createTask] ───►  READY  ◄─────── [delay expired / resumeTask]
                       │  ▲
           [schedule]  │  │ [yield / round-robin]
                       ▼  │
                    RUNNING
                     │   │
     [delay(ticks)]  │   └──► [suspendTask] ──► SUSPENDED
                     ▼                              │
                  BLOCKED ──────────────────────────┘
```

- **`READY`**: Eligible for execution in the ready queue.
- **`RUNNING`**: Currently executing task slice.
- **`BLOCKED`**: Suspended in the delay queue awaiting a target tick count (`delay(ticks)`).
- **`SUSPENDED`**: Manually paused task (`suspendTask()`).
- **`TERMINATED`**: Deleted or completed task.

### Scheduling Policy

1. **Strict Priority Preemption**: Highest numeric priority (255 down to 1) READY task is always selected.
2. **Equal-Priority Round-Robin**: Tasks sharing the same priority level take turns in FIFO sequence.
3. **Idle Task**: Priority 0 task automatically executed when no user tasks are ready.

### Example Usage

```cpp
efs::system::SystemBus systemBus;
efs::cpu::CPU cpu(&systemBus);
efs::rtos::Scheduler rtos(&systemBus);

cpu.attachRTOSScheduler(&rtos);

// Create Sensor Sampling Task (High Priority 10)
rtos.createTask("SensorTask", [&rtos]() {
    // Perform sensor read...
    rtos.delay(5); // Block for 5 simulation ticks
}, 10);

// Create Heartbeat LED Task (Medium Priority 5)
rtos.createTask("HeartbeatTask", []() {
    // Toggle LED pin...
}, 5);

// Start CPU execution -> runs RTOS Task Scheduler
cpu.start();
for (int i = 0; i < 20; ++i) {
    cpu.step();
}
cpu.stop();
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
