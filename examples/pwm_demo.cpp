#include "drivers/pwm/pwm_controller.hpp"
#include "hal/pwm_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    std::cout << "Starting Pulse Width Modulation (PWM) Demo...\n";

    // 1. Initialize MMIO Bus and SystemBus
    efs::mmio::MMIOBus mmioBus;
    efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

    constexpr efs::common::Address PWM_BASE_ADDR = 0x40006000U;

    // 2. Instantiate PWM Controller (1000 Hz, 0% initial duty cycle, 1,000,000 Hz system clock)
    efs::drivers::pwm::PWMController pwm(mmioBus, PWM_BASE_ADDR, 1000, 0, 1000000);
    systemBus.attachPWM(&pwm);

    // 3. Attach PWM HAL for firmware access
    efs::hal::PWMHAL pwmHAL(&pwm);
    pwmHAL.enable();

    std::cout << "PWM Peripheral enabled.\n";
    std::cout << "  Base Address: 0x" << std::hex << PWM_BASE_ADDR << std::dec << "\n";
    std::cout << "  Frequency:    " << pwmHAL.frequency() << " Hz\n";
    std::cout << "  Duty Cycle:   " << pwmHAL.dutyCycle() << "%\n\n";

    // 4. Demo 1: LED Dimming Simulation via Duty Cycle Stepping (0% -> 25% -> 50% -> 75% -> 100%)
    std::cout << "--- Demo 1: LED Dimming Simulation (Duty Cycle Sweeping) ---\n";
    const std::vector<std::uint32_t> dutyLevels = {0, 25, 50, 75, 100};

    for (std::uint32_t duty : dutyLevels) {
        pwmHAL.setDutyCycle(duty);

        std::size_t highCycles = 0;
        std::size_t lowCycles = 0;

        // Sample 1000 simulation clock cycles (one full 1000 Hz PWM period at 1 MHz clock)
        for (int i = 0; i < 1000; ++i) {
            if (pwmHAL.outputState()) {
                highCycles++;
            } else {
                lowCycles++;
            }
            systemBus.tickTimers();
        }

        std::cout << "  Duty: " << std::setw(3) << duty << "% | Output HIGH: "
                  << std::setw(4) << highCycles << " cycles | Output LOW: "
                  << std::setw(4) << lowCycles << " cycles | Pin State: "
                  << (pwmHAL.outputState() ? "HIGH" : "LOW") << "\n";
    }

    // 5. Demo 2: Motor Speed / Frequency Reconfiguration
    std::cout << "\n--- Demo 2: Frequency Reconfiguration ---\n";
    pwmHAL.setFrequency(5000); // 5 kHz
    pwmHAL.setDutyCycle(50);
    std::cout << "  Reconfigured PWM Frequency to " << pwmHAL.frequency() << " Hz, Duty to " << pwmHAL.dutyCycle() << "%\n";

    std::size_t highCount5k = 0;
    for (int i = 0; i < 200; ++i) { // 1 period = 200 cycles
        if (pwmHAL.outputState()) {
            highCount5k++;
        }
        systemBus.tickTimers();
    }
    std::cout << "  Sampled 1 period (200 cycles): HIGH for " << highCount5k << " cycles.\n";

    // 6. SystemBus Reset Verification
    std::cout << "\nResetting SystemBus...\n";
    systemBus.reset();
    std::cout << "PWM Enabled after reset? " << (pwmHAL.enabled() ? "Yes" : "No") << "\n";

    std::cout << "\nPWM Controller Demo completed successfully.\n";
    return 0;
}
