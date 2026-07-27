# Embedded Firmware Simulator

Version: v0.1

A modular, scalable C++20 simulator architecture for embedded firmware.

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

## License

This project is licensed under the [MIT License](LICENSE).
