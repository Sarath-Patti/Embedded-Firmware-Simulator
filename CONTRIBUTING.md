# Contributing to Embedded Firmware Simulator

Thank you for your interest in contributing to the **Embedded Firmware Simulator (EFS)** project!

## Code Standards

- **Language Standard**: Modern C++17.
- **Compiler Compliance**: Code must compile cleanly with `-Wall -Wextra -Wpedantic -Werror` under Apple Clang, GCC, and MSVC (`/W4 /WX`).
- **Memory Safety**: Prefer standard smart pointers (`std::shared_ptr`, `std::unique_ptr`) and RAII patterns.
- **Documentation**: All public headers must use Doxygen-style docstrings (`///`).

## Build & Verification

Before submitting pull requests, run all four build configurations:

```bash
# 1. Apple Clang Release
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DEFS_ENABLE_WERROR=ON
cmake --build build && ctest --test-dir build --output-on-failure

# 2. Apple Clang Debug
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DEFS_ENABLE_WERROR=ON
cmake --build build-debug && ctest --test-dir build-debug --output-on-failure

# 3. GCC 16 Release
CC=/opt/homebrew/bin/gcc-16 CXX=/opt/homebrew/bin/g++-16 cmake -S . -B build-gcc -DCMAKE_BUILD_TYPE=Release -DEFS_ENABLE_WERROR=ON
cmake --build build-gcc && ctest --test-dir build-gcc --output-on-failure

# 4. GCC 16 Debug
CC=/opt/homebrew/bin/gcc-16 CXX=/opt/homebrew/bin/g++-16 cmake -S . -B build-gcc-debug -DCMAKE_BUILD_TYPE=Debug -DEFS_ENABLE_WERROR=ON
cmake --build build-gcc-debug && ctest --test-dir build-gcc-debug --output-on-failure
```
