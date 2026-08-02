# Hardware Abstraction Layer (HAL)

This document details the Hardware Abstraction Layer (HAL) modules available in the **Embedded Firmware Simulator (EFS)**.

## Architecture

The HAL decouples firmware logic from raw Memory-Mapped I/O (MMIO) register addresses by presenting clean, object-oriented C++ interfaces.

```
+-------------------------------------------------------------+
|                     Firmware Application                    |
+-------------------------------------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                  Hardware Abstraction Layer                 |
|  +-----------+  +----------+  +----------+  +------------+  |
|  |  GPIOHAL  |  | TimerHAL |  | UARTHAL  |  |   SPIHAL   |  |
|  +-----------+  +----------+  +----------+  +------------+  |
|  |  I2CHAL   |  |  PWMHAL  |  | ADCHAL   |  |            |  |
|  +-----------+  +----------+  +----------+  +------------+  |
+-------------------------------------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                     Peripheral Drivers                      |
|  +-----------+  +----------+  +----------+  +------------+  |
|  |   GPIO    |  |  Timer   |  |   UART   |  |    SPI     |  |
|  +-----------+  +----------+  +----------+  +------------+  |
+-------------------------------------------------------------+
```

---

## Available HAL Interfaces

### 1. `GPIOHAL` (`hal/gpio_hal.hpp`)
- `pinMode(pin, direction)`: Set pin input/output mode.
- `write(pin, value)`: Write boolean digital output state.
- `read(pin)`: Read digital pin state.
- `toggle(pin)`: Invert pin state.

### 2. `TimerHAL` (`hal/timer_hal.hpp`)
- `start()`, `stop()`, `reset()`: Control timer execution.
- `setPeriod(ticks)`: Configure comparison match value.
- `counter()`: Read current counter value.

### 3. `UARTHAL` (`hal/uart_hal.hpp`)
- `write(byte)` / `write(string)`: Send data over serial bus.
- `read()`: Read received byte from RX FIFO buffer.
- `available()`: Check if unread bytes exist in RX buffer.

### 4. `SPIHAL` (`hal/spi_hal.hpp`)
- `transfer(byte)`: Full-duplex SPI single-byte transfer.
- `transfer(txBuf, rxBuf, length)`: Buffer SPI transfer.

### 5. `I2CHAL` (`hal/i2c_hal.hpp`)
- `beginTransmission(address)` / `endTransmission()`: Manage I2C write transactions.
- `requestFrom(address, length)`: Initiate I2C read transactions.

### 6. `PWMHAL` (`hal/pwm_hal.hpp`)
- `setFrequency(hz)`: Set PWM carrier frequency.
- `setDutyCycle(percent)`: Adjust duty cycle (0–100%).

### 7. `ADCHAL` (`hal/adc_hal.hpp`)
- `readChannel(channel)`: Sample 12-bit ADC channel value.
- `startConversion()`: Trigger continuous or single-shot ADC conversions.
