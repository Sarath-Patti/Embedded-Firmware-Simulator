# Peripheral Drivers Reference

This document describes the design and usage of peripheral drivers within the **Embedded Firmware Simulator (EFS)** platform.

## Peripherals Overview

All peripheral drivers in the simulator inherit from or interact with `efs::mmio::MMIOBus` to register 32-bit hardware registers.

---

## Driver Capabilities

### 1. GPIO (`efs::drivers::gpio::GPIO`)
- Dynamic pin direction configuration (`Input`, `Output`).
- Digital pin state reading (`High`, `Low`) and writing.
- Configurable pin modes (`PushPull`, `OpenDrain`, `PullUp`, `PullDown`).

### 2. Timer (`efs::drivers::timer::Timer`)
- 32-bit up-counter with configurable auto-reset.
- Match interrupt generation connected to `InterruptController`.
- Simulation clock integration (`tick()`).

### 3. UART (`efs::drivers::uart::UART`)
- Simulation of full-duplex UART serial hardware.
- Configurable baud rates (e.g. 9600, 115200 baud).
- Integrated transmit and receive FIFO buffers.

### 4. DMA Controller (`efs::drivers::dma::DMAController`)
- Memory-to-Memory, Memory-to-Peripheral, and Peripheral-to-Memory transfers.
- Non-blocking transaction execution.
- Transfer complete interrupt callbacks.

### 5. SPI Master Controller (`efs::drivers::spi::SPIController`)
- Support for SPI mode selection (CPOL/CPHA) and clock prescalers.
- Simulated external `SPIDevice` slave attachment.

### 6. I2C Master Controller (`efs::drivers::i2c::I2CController`)
- Standard (100 kHz) and Fast Mode (400 kHz) I2C master simulation.
- Multi-device slave addressing and ACK/NACK signaling.

### 7. PWM Controller (`efs::drivers::pwm::PWMController`)
- Programmable pulse frequency and duty cycle (0–100%).
- Integrated timer-driven square wave generator.

### 8. Analog-to-Digital Converter (`efs::drivers::adc::ADCController`)
- 12-bit analog sampling resolution (0–4095 range).
- 8-channel analog input selection.
- Continuous and single-shot conversion modes.
