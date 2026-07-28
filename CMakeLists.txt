cmake_minimum_required(VERSION 3.16)
project(EmbeddedFirmwareSimulator VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Static library target for the simulator
add_library(embedded_firmware_simulator STATIC
    src/common/logger.cpp
    src/memory/memory.cpp
    src/mmio/register.cpp
    src/mmio/mmio_bus.cpp
    src/drivers/gpio/gpio.cpp
    src/drivers/timer/timer.cpp
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

add_executable(mmio_tests tests/test_mmio.cpp)
target_link_libraries(mmio_tests PRIVATE embedded_firmware_simulator)

if(MSVC)
    target_compile_options(mmio_tests PRIVATE /W4)
else()
    target_compile_options(mmio_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME MMIOTests COMMAND mmio_tests)

add_executable(gpio_tests tests/test_gpio.cpp)
target_link_libraries(gpio_tests PRIVATE embedded_firmware_simulator)

if(MSVC)
    target_compile_options(gpio_tests PRIVATE /W4)
else()
    target_compile_options(gpio_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME GPIOTests COMMAND gpio_tests)

add_executable(timer_tests tests/test_timer.cpp)
target_link_libraries(timer_tests PRIVATE embedded_firmware_simulator)

if(MSVC)
    target_compile_options(timer_tests PRIVATE /W4)
else()
    target_compile_options(timer_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME TimerTests COMMAND timer_tests)

