# Embedded Firmware Simulator

Version: v0.3

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

