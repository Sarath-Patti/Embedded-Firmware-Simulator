#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/uart/uart.hpp"
#include "hal/gpio_hal.hpp"
#include "hal/uart_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "rtos/rtos_scheduler.hpp"
#include "system/system_bus.hpp"
#include <iomanip>
#include <iostream>

int main() {
    std::cout << "Starting RTOS Task Scheduler Demo...\n";

    // 1. Initialize MMIO Bus and SystemBus
    efs::mmio::MMIOBus mmioBus;
    efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

    // 2. Initialize Peripherals (GPIO at 0x40000000, UART at 0x40002000)
    efs::drivers::gpio::GPIO gpio(mmioBus, 0x40000000U);
    efs::drivers::uart::UART uart(mmioBus, 0x40002000U);

    systemBus.attachGPIO(&gpio);
    systemBus.attachUART(&uart);

    // 3. Setup HAL Abstractions
    efs::hal::GPIOHAL gpioHAL(&gpio);
    efs::hal::UARTHAL uartHAL(&uart);

    gpioHAL.configureOutput(0); // Output pin 0 for Heartbeat LED

    // 4. Initialize CPU and RTOS Scheduler
    efs::cpu::CPU cpu(&systemBus);
    efs::rtos::Scheduler rtos(&systemBus);
    cpu.attachRTOSScheduler(&rtos);

    std::cout << "RTOS Scheduler initialized.\n";
    std::cout << "  System Clock Frequency: " << systemBus.clock().cycles() << " cycles\n\n";

    // 5. Create RTOS Tasks
    std::size_t heartbeatCount = 0;
    std::size_t sensorCount = 0;
    std::size_t commCount = 0;

    // Task 1: Sensor Sampling Task (Priority 10 - High)
    rtos.createTask("SensorTask", [&rtos, &sensorCount]() {
        sensorCount++;
        std::cout << "  [SensorTask (High Priority 10)] Sampling sensors... Execution #" << sensorCount << "\n";
        rtos.delay(2); // Delay for 2 ticks
    }, 10);

    // Task 2: Heartbeat LED Task (Priority 5 - Medium)
    rtos.createTask("HeartbeatTask", [&gpioHAL, &heartbeatCount]() {
        heartbeatCount++;
        bool pinState = (heartbeatCount % 2 != 0);
        gpioHAL.write(0, pinState);
        std::cout << "  [HeartbeatTask (Med Priority 5)] LED Pin 0 state: " << (pinState ? "HIGH" : "LOW") << "\n";
    }, 5);

    // Task 3: Telemetry Serial Communication Task (Priority 5 - Medium, Round-Robin partner)
    rtos.createTask("TelemetryTask", [&uartHAL, &commCount]() {
        commCount++;
        uartHAL.writeByte('T');
        std::cout << "  [TelemetryTask (Med Priority 5)] Sent serial byte 'T'.\n";
    }, 5);

    // 6. Run RTOS execution loop via CPU steps
    cpu.start();
    std::cout << "--- Executing 10 CPU / RTOS Simulation Steps ---\n";
    for (int step = 1; step <= 10; ++step) {
        std::cout << "Step " << std::setw(2) << step << ": ";
        cpu.step();
    }

    std::cout << "\nTask Execution Statistics:\n";
    for (std::uint32_t id = 0; id < rtos.taskCount(); ++id) {
        auto* tcb = rtos.getTask(id);
        if (tcb != nullptr) {
            std::cout << "  Task ID " << tcb->id << " (" << std::left << std::setw(15) << tcb->name
                      << ") | Priority: " << std::setw(2) << static_cast<int>(tcb->priority)
                      << " | State: " << static_cast<int>(tcb->state)
                      << " | Executions: " << tcb->executionCount << "\n";
        }
    }

    cpu.stop();
    std::cout << "\nRTOS Task Scheduler Demo completed successfully.\n";
    return 0;
}
