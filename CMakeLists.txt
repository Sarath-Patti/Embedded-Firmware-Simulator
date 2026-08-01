cmake_minimum_required(VERSION 3.16)
project(EmbeddedFirmwareSimulator VERSION 1.5.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to treat compiler warnings as errors (default OFF for local development, ON in CI)
option(EFS_ENABLE_WERROR "Treat warnings as errors" OFF)

set(EFS_WARNING_FLAGS)
if(MSVC)
    set(EFS_WARNING_FLAGS /W4)
    if(EFS_ENABLE_WERROR)
        list(APPEND EFS_WARNING_FLAGS /WX)
    endif()
else()
    set(EFS_WARNING_FLAGS -Wall -Wextra -Wpedantic)
    if(EFS_ENABLE_WERROR)
        list(APPEND EFS_WARNING_FLAGS -Werror)
    endif()
endif()

# Static library target for the simulator
add_library(embedded_firmware_simulator STATIC
    src/common/logger.cpp
    src/memory/memory.cpp
    src/mmio/register.cpp
    src/mmio/mmio_bus.cpp
    src/drivers/gpio/gpio.cpp
    src/drivers/timer/timer.cpp
    src/drivers/uart/uart.cpp
    src/hal/gpio_hal.cpp
    src/hal/timer_hal.cpp
    src/hal/uart_hal.cpp
    src/kernel/interrupt_controller.cpp
    src/system/system_bus.cpp
    src/system/clock/simulation_clock.cpp
    src/system/scheduler/event_scheduler.cpp
    src/cpu/cpu.cpp
    src/cpu/registers/register_file.cpp
    src/firmware/firmware.cpp
    src/firmware/basic_firmware.cpp
    src/monitor/monitor.cpp
)

target_include_directories(embedded_firmware_simulator PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(embedded_firmware_simulator PRIVATE ${EFS_WARNING_FLAGS})

# Executable targets
add_executable(simulator_demo src/main.cpp)
target_link_libraries(simulator_demo PRIVATE embedded_firmware_simulator)
target_compile_options(simulator_demo PRIVATE ${EFS_WARNING_FLAGS})

add_executable(firmware_demo examples/firmware_demo.cpp)
target_link_libraries(firmware_demo PRIVATE embedded_firmware_simulator)
target_compile_options(firmware_demo PRIVATE ${EFS_WARNING_FLAGS})

add_executable(uart_demo examples/uart_demo.cpp)
target_link_libraries(uart_demo PRIVATE embedded_firmware_simulator)
target_compile_options(uart_demo PRIVATE ${EFS_WARNING_FLAGS})

add_executable(event_scheduler_demo examples/event_scheduler_demo.cpp)
target_link_libraries(event_scheduler_demo PRIVATE embedded_firmware_simulator)
target_compile_options(event_scheduler_demo PRIVATE ${EFS_WARNING_FLAGS})

add_executable(hal_demo examples/hal_demo.cpp)
target_link_libraries(hal_demo PRIVATE embedded_firmware_simulator)
target_compile_options(hal_demo PRIVATE ${EFS_WARNING_FLAGS})

# Testing Configuration
enable_testing()

function(efs_add_test TARGET_NAME SOURCE_FILE TEST_NAME)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE embedded_firmware_simulator)
    target_compile_options(${TARGET_NAME} PRIVATE ${EFS_WARNING_FLAGS})
    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})
endfunction()

efs_add_test(memory_tests tests/test_memory.cpp MemoryTests)
efs_add_test(mmio_tests tests/test_mmio.cpp MMIOTests)
efs_add_test(gpio_tests tests/test_gpio.cpp GPIOTests)
efs_add_test(timer_tests tests/test_timer.cpp TimerTests)
efs_add_test(interrupt_tests tests/test_interrupt_controller.cpp InterruptTests)
efs_add_test(cpu_tests tests/test_cpu.cpp CPUTests)
efs_add_test(firmware_tests tests/test_firmware.cpp FirmwareTests)
efs_add_test(register_tests tests/test_register_file.cpp RegisterTests)
efs_add_test(monitor_tests tests/test_monitor.cpp MonitorTests)
efs_add_test(uart_tests tests/test_uart.cpp UARTTests)
efs_add_test(system_tests tests/test_system_bus.cpp SystemTests)
efs_add_test(clock_tests tests/test_clock.cpp ClockTests)
efs_add_test(scheduler_tests tests/test_scheduler.cpp SchedulerTests)
efs_add_test(hal_tests tests/test_hal.cpp HALTests)
