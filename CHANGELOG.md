# Changelog

All notable changes to the **Embedded Firmware Simulator (EFS)** project are documented in this file.

## [3.0.0] - 2026-08-02 (Final Polish Milestone)

### Added
- Complete architectural documentation in `docs/` (`architecture.md`, `firmware.md`, `hal.md`, `memory-map.md`, `peripherals.md`, `scheduler.md`, `monitor.md`).
- Extended contribution guide (`CONTRIBUTING.md`) and project history (`CHANGELOG.md`).

### Fixed
- Fixed Debug mode assertion timing in `test_uart.cpp`, `test_i2c.cpp`, `test_pwm.cpp`, and `test_cpu.cpp`.
- Cleaned up non-existent API method calls across unit test suites.
- Achieved 100% pass rates across Apple Clang and GCC 16 in both Release and Debug build modes.

---

## [2.4.0] - 2026-08-02
- Implemented RTOS foundation and priority task scheduler (`RTOSScheduler`, `TaskControlBlock`).

## [2.0.0] - 2026-08-01
- Added SPI, I2C, PWM, ADC controllers, and HAL wrappers.

## [1.0.0] - 2026-07-27
- Initial simulator release with CPU, Memory, MMIO, GPIO, Timer, UART, and Interrupt Controller.
