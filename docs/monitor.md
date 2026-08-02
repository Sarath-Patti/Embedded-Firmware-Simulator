# System Monitor CLI Reference

This document describes the CLI system debugging and monitoring subsystem in the **Embedded Firmware Simulator (EFS)** platform.

## Overview

The System Monitor (`efs::monitor::Monitor`) provides interactive system state inspection and runtime telemetry commands.

---

## Monitor Commands

### 1. `help`
- Displays list of available monitor commands and command syntax.

### 2. `cpu`
- Displays CPU cycle count, power state, attached buses, and register values.

### 3. `memory <address> <length>`
- Dumps hex and ASCII representation of memory regions.

### 4. `mmio`
- Lists registered MMIO addresses and their current 32-bit values.

### 5. `gpio`
- Displays GPIO pin states, directions, and modes.

### 6. `timer`
- Displays timer counter value, compare match setting, and status bits.

### 7. `uart`
- Displays UART baud rate, FIFO status, and enable state.

### 8. `tasks` / `rtos`
- Dumps RTOS task list, task states, priorities, and TCB metrics.
