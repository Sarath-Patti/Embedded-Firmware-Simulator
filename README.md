# Embedded Firmware Simulator

Version: v0.2

A modular, scalable C++20 simulator architecture for embedded firmware.

## Subsystems & Modules

### Memory Subsystem (`efs::memory::Memory`)
- **Contiguous Storage**: Encapsulated byte-addressable memory buffer configurable at initialization.
- **Bounds Checking**: Strictly validates memory address boundaries on `read` and `write` operations.
- **Error Reporting**: Logs out-of-bounds access via project logger utility and throws `std::out_of_range`.
- **Reset Utility**: Provides a `clear()` method to zero-initialize the memory region.

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

