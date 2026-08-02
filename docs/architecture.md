# Embedded Firmware Simulator Architecture

This document details the architectural layout, core subsystems, and communication flows of the **Embedded Firmware Simulator (EFS)** platform.

## Overview

The simulator is structured as a modular, hardware-accurate C++17 simulation platform for bare-metal and RTOS firmware testing without hardware dependencies.

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

## Subsystems

### 1. CPU Core (`efs::cpu::CPU`)
- Drives execution cycles (`step()`, `run(cycles)`).
- Interacts with `SystemBus` for memory, MMIO, and interrupt handling.
- Manages internal state (`RegisterFile`, cycle counters) and execution power state.

### 2. Memory-Mapped I/O Bus (`efs::mmio::MMIOBus`)
- Manages 32-bit address decoding for hardware registers.
- Dispatches read and write requests to registered `Register` instances.
- Raises out-of-range exceptions on invalid memory accesses.

### 3. Memory Subsystem (`efs::memory::Memory`)
- Simulates physical RAM storage.
- Provides byte, half-word, and word access methods with bounds checking.

### 4. Interrupt Controller (`efs::kernel::InterruptController`)
- Manages interrupt line registration, enablement, pending bits, and priorities.
- Dispatches registered Interrupt Service Routines (ISRs) to the CPU.

### 5. Event Scheduler (`efs::system::scheduler::EventScheduler`)
- Manages timed simulation callbacks using a priority queue.
- Executes callbacks deterministically based on simulation clock ticks.

### 6. Hardware Abstraction Layer (HAL)
- Wraps raw MMIO peripheral registers into high-level C++ interfaces (`GPIOHAL`, `TimerHAL`, `UARTHAL`, etc.).
- Allows application code to be decoupled from peripheral register maps.

### 7. RTOS Scheduler (`efs::rtos::RTOSScheduler`)
- Provides priority-based task scheduling (preemptive and cooperative modes).
- Manages Task Control Blocks (TCB), context switches, task suspension/resumption, and dynamic task creation.

### 8. System Monitor (`efs::monitor::Monitor`)
- Interactive CLI debugging and telemetry interface.
- Inspects memory regions, register states, peripheral stats, and RTOS task control blocks.
