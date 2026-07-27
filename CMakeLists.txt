cmake_minimum_required(VERSION 3.16)
project(EmbeddedFirmwareSimulator VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static library target for the simulator
add_library(embedded_firmware_simulator STATIC
    src/common/logger.cpp
    src/memory/memory.cpp
)

target_include_directories(embedded_firmware_simulator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compiler Warnings
if(MSVC)
    target_compile_options(embedded_firmware_simulator PRIVATE /W4)
else()
    target_compile_options(embedded_firmware_simulator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Demo executable target
add_executable(simulator_demo src/main.cpp)

target_link_libraries(simulator_demo PRIVATE embedded_firmware_simulator)

if(MSVC)
    target_compile_options(simulator_demo PRIVATE /W4)
else()
    target_compile_options(simulator_demo PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Testing Configuration
enable_testing()

add_executable(memory_tests tests/test_memory.cpp)
target_link_libraries(memory_tests PRIVATE embedded_firmware_simulator)

if(MSVC)
    target_compile_options(memory_tests PRIVATE /W4)
else()
    target_compile_options(memory_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME MemoryTests COMMAND memory_tests)

