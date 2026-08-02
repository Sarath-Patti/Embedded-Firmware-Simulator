# Memory & MMIO Register Map

This document specifies the system memory layout and Memory-Mapped I/O (MMIO) register addresses for the **Embedded Firmware Simulator (EFS)**.

## Address Space Overview

The simulator operates on a 32-bit flat memory address space:

```
+------------------------+ 0x00000000
|      System RAM        |
|  (Size: Configurable)  |
+------------------------+ 0x3FFFFFFF
|  Peripheral MMIO Region|
|  0x40000000-0x40007FFF |
+------------------------+ 0x40008000
|       Reserved         |
+------------------------+ 0xFFFFFFFF
```

---

## MMIO Base Addresses

| Peripheral | Base Address | Offset Range | Description |
| :--- | :--- | :--- | :--- |
| **GPIO** | `0x40000000` | `0x00` – `0x0C` | General Purpose Input/Output |
| **Timer** | `0x40001000` | `0x00` – `0x0C` | 32-bit Timer / Counter |
| **Interrupt Controller** | `0x40002000` | `0x00` – `0x0C` | Interrupt Management (VIC) |
| **UART** | `0x40003000` | `0x00` – `0x0C` | Serial Communication Interface |
| **DMA** | `0x40004000` | `0x00` – `0x10` | Direct Memory Access Controller |
| **SPI** | `0x40004800` | `0x00` – `0x0C` | Serial Peripheral Interface |
| **I2C** | `0x40005000` | `0x00` – `0x0C` | Inter-Integrated Circuit |
| **PWM** | `0x40006000` | `0x00` – `0x0C` | Pulse Width Modulation Controller |
| **ADC** | `0x40007000` | `0x00` – `0x0C` | Analog-to-Digital Converter |

---

## Detailed Register Offsets

### GPIO (`0x40000000`)
- `0x00`: `DATA` – Pin input/output state bitmask.
- `0x04`: `DIR` – Pin direction bitmask (0 = Input, 1 = Output).
- `0x08`: `MODE` – Pin mode configuration.

### Timer (`0x40001000`)
- `0x00`: `COUNTER` – Current counter value.
- `0x04`: `COMPARE` – Counter match value.
- `0x08`: `CONTROL` – Bit 0: Enable, Bit 1: Auto-reset, Bit 2: Interrupt Enable.

### Interrupt Controller (`0x40002000`)
- `0x00`: `ENABLE` – Interrupt enable bitmask.
- `0x04`: `PENDING` – Pending interrupt lines bitmask.
- `0x08`: `PRIORITY` – Priority register configuration.

### UART (`0x40003000`)
- `0x00`: `DATA` – Transmit/Receive data register.
- `0x04`: `STATUS` – Bit 0: TX Empty, Bit 1: RX Available.
- `0x08`: `CONTROL` – Bit 0: UART Enable.
- `0x0C`: `BAUD` – Baud rate register.

### DMA (`0x40004000`)
- `0x00`: `SRC_ADDR` – Transfer source address.
- `0x04`: `DEST_ADDR` – Transfer destination address.
- `0x08`: `TRANSFER_SIZE` – Size in bytes.
- `0x0C`: `CONTROL` – Bit 0: Start, Bit 1: Busy, Bit 2: Done.

### SPI (`0x40004800`)
- `0x00`: `DATA` – Data register.
- `0x04`: `STATUS` – Bit 0: TX Empty, Bit 1: RX Available, Bit 2: Busy.
- `0x08`: `CONTROL` – Bit 0: Enable, Bit 1: Mode.
- `0x0C`: `CLOCK_DIV` – Clock divider value.

### I2C (`0x40005000`)
- `0x00`: `DATA` – Data buffer register.
- `0x04`: `STATUS` – Bit 0: Busy, Bit 1: ACK Received, Bit 2: Error.
- `0x08`: `CONTROL` – Bit 0: Enable, Bit 1: Start, Bit 2: Stop.
- `0x0C`: `SLAVE_ADDR` – Target slave device 7-bit address.

### PWM (`0x40006000`)
- `0x00`: `CONTROL` – Bit 0: Enable.
- `0x04`: `FREQ` – Carrier frequency in Hz.
- `0x08`: `DUTY` – Duty cycle percentage (0–100%).
- `0x0C`: `STATUS` – Output active status bitmask.

### ADC (`0x40007000`)
- `0x00`: `DATA` – 12-bit conversion value register.
- `0x04`: `CONTROL` – Bit 0: Enable, Bit 1: Start Conversion.
- `0x08`: `CHANNEL` – Active channel selector (0–7).
- `0x0C`: `STATUS` – Bit 0: Conversion Done.
