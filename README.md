# Embedded Firmware Simulator

Version: v0.9

A modular, scalable C++20 simulator architecture for embedded firmware.

## Subsystems & Modules

### Memory Subsystem (`efs::memory::Memory`)
- **Contiguous Storage**: Encapsulated byte-addressable memory buffer configurable at initialization.
- **Bounds Checking**: Strictly validates memory address boundaries on `read` and `write` operations.
- **Error Reporting**: Logs out-of-bounds access via project logger utility and throws `std::out_of_range`.
- **Reset Utility**: Provides a `clear()` method to zero-initialize the memory region.

### MMIO Subsystem (`efs::mmio::Register`, `efs::mmio::MMIOBus`)
- **Register Abstraction**: Encapsulates single memory-mapped register storage with `address()`, `read()`, and `write(value)`.
- **MMIO Bus Dispatch**: Manages dynamic address-to-register mappings (`registerRegister`, `unregisterRegister`, `contains`).
- **Access Safety**: Rejects duplicate address registrations and throws `std::out_of_range` on unmapped address access attempts.
- **Decoupled Architecture**: Operates independently of specific peripheral implementations.

### GPIO Peripheral (`efs::drivers::gpio::GPIO`)
- **MMIO Register Mapping**: Integrates DIR (0x00), OUT (0x04), and IN (0x08) registers into the MMIO Bus.
- **Pin Operations**: Supports `configurePin`, `writePin`, `readPin`, and `togglePin` for 32 pins.
- **Safety & Validation**: Enforces pin index validation and verifies direction prior to write/toggle operations.
- **State Reflection**: Bi-directionally synchronizes pin state mutations with MMIO register values.

### Timer Peripheral (`efs::drivers::timer::Timer`)
- **MMIO Register Mapping**: Integrates CTRL (0x00), COUNT (0x04), COMPARE (0x08), and STATUS (0x0C) registers into the MMIO Bus.
- **Timer Operations**: Supports `start()`, `stop()`, `reset()`, `tick()`, `setCompare(value)`, `counter()`, and `running()`.
- **Compare Matching**: Increments counter while enabled; sets STATUS match flag and automatically halts execution upon reaching the target compare value.
- **Synchronized State**: Seamlessly reflects state transitions between direct C++ API methods and MMIO register operations.

### Interrupt Controller (`efs::kernel::InterruptController`)
- **MMIO Register Interface**: Exposes ENABLE (0x00), PENDING (0x04), and PRIORITY (0x08) registers via MMIO Bus.
- **Interrupt Lifecycle**: Supports registration (`registerInterrupt`), enabling/disabling (`enable`/`disable`), manual and hardware triggering (`trigger`), and status query (`pending`/`enabled`).
- **Priority Dispatch**: Evaluates pending enabled interrupts and dispatches the highest-priority handler callback (`dispatch()`), clearing the pending flag upon invocation.
- **Peripheral Integration**: Connects with peripherals like `Timer` to automatically trigger interrupts on hardware events (e.g. COMPARE match).

### CPU Execution Engine (`efs::cpu::CPU`)
- **Simulation Cycle Engine**: Coordinates cycle-based simulation execution (`start`, `stop`, `reset`, `step`, `run`).
- **Peripheral Coordination**: Drives attached timers via `Timer.tick()` during each simulation step.
- **Interrupt Orchestration**: Triggers `InterruptController.dispatch()` per cycle to execute ISR callbacks.
- **Cycle Tracking**: Maintains 64-bit cycle count (`cycleCount()`) and manages execution state (`running()`).

### Firmware Execution Layer (`efs::firmware::Firmware`, `efs::firmware::BasicFirmware`)
- **Firmware Interface**: Abstract lifecycle interface exposing `initialize()`, `execute()`, and `shutdown()`.
- **CPU Integration**: CPU manages firmware lifecycle through `loadFirmware()`, `unloadFirmware()`, and `firmwareLoaded()`. Safely handles absence of loaded firmware.
- **BasicFirmware Implementation**: Example firmware driving GPIO pin toggling at configurable cycle intervals during CPU simulation steps.

### CPU Register File (`efs::cpu::registers::RegisterFile`)
- **Processor State Abstraction**: Encapsulates 16 General Purpose Registers (R0–R15), Program Counter (PC), Stack Pointer (SP), and Status Register (SR).
- **Access Safety & Validation**: Validates GPR register indices (`readRegister`, `writeRegister`), throwing `std::out_of_range` on invalid indices.
- **Special Registers**: Direct accessors for PC (`readPC`/`writePC`), SP (`readSP`/`writeSP`), and Status (`readStatus`/`writeStatus`).
- **CPU Ownership**: Directly owned and managed by `CPU`, resetting alongside simulator cycle reset (`CPU::reset()`).

## Project Structure

- `include/`: Public header files (`cpu`, `kernel`, `memory`, `drivers`, `debugger`, `common`).
- `src/`: Core implementation files.
- `examples/`: Code examples and demo applications.
- `tests/`: Unit and integration test suites.
- `benchmarks/`: Performance benchmarks.
- `docs/`: Project documentation.
- `scripts/`: Development and utility scripts.

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running Tests

```bash
ctest --output-on-failure
```

## License

This project is licensed under the [MIT License](LICENSE).

