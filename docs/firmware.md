# Firmware Interface & Firmware Manager

This document covers the Firmware framework and execution lifecycle in the **Embedded Firmware Simulator (EFS)**.

## Overview

The simulator supports executing object-oriented firmware modules on top of the simulated hardware stack.

```
       +----------------------------+
       |   IFirmware / Firmware     |
       +----------------------------+
         ^            ^           ^
         |            |           |
+------------------+  |  +------------------+
|  BasicFirmware   |  |  | TimerBlinkFw     |
+------------------+  |  +------------------+
                      |
           +--------------------+
           |  UARTEchoFirmware  |
           +--------------------+
```

---

## Firmware Lifecycle

Every firmware module inherits from `efs::firmware::Firmware` and implements three core methods:

1. **`initialize()`**: Executed once when the firmware is loaded or when the system resets. Initializes peripheral directions, interrupts, and default configurations.
2. **`update()`**: Executed periodically on every CPU step while the CPU is powered ON and running. Contains the primary application loop logic.
3. **`shutdown()`**: Executed when the firmware is stopped or unloaded to safely disable peripherals.

---

## Standard Firmware Modules

### `BasicFirmware`
- Demonstrates GPIO HAL control and interval-based pin toggling.

### `TimerBlinkFirmware`
- Demonstrates hardware timer interrupt driven LED blinking via `TimerHAL` and `GPIOHAL`.

### `UARTEchoFirmware`
- Demonstrates full-duplex serial communication by echoing characters received on `UARTHAL`.

---

## Firmware Manager (`efs::firmware::FirmwareManager`)

The `FirmwareManager` manages multiple firmware applications:
- **`registerFirmware(shared_ptr<Firmware>, name)`**: Registers a firmware module.
- **`loadFirmware(name)`**: Loads and initializes the active firmware module.
- **`update()`**: Drives execution for the currently loaded firmware application.
