#include "drivers/pwm/pwm_controller.hpp"
#include "hal/pwm_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace efs::drivers::pwm;
using namespace efs::hal;
using namespace efs::mmio;
using namespace efs::system;

void test_pwm_initialization() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    assert(bus.contains(pwm.controlAddress()));
    assert(bus.contains(pwm.freqAddress()));
    assert(bus.contains(pwm.dutyAddress()));
    assert(bus.contains(pwm.statusAddress()));

    assert(!pwm.enabled());
    assert(pwm.frequency() == 1000);
    assert(pwm.dutyCycle() == 50);

    pwm.enable();
    assert(pwm.enabled());
    assert((bus.read(pwm.controlAddress()) & PWMController::CTRL_ENABLE_BIT) != 0);

    pwm.disable();
    assert(!pwm.enabled());

    std::cout << "[PASS] test_pwm_initialization\n";
}

void test_pwm_frequency_duty_changes() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    pwm.setFrequency(2000);
    assert(pwm.frequency() == 2000);

    pwm.setDutyCycle(25);
    assert(pwm.dutyCycle() == 25);

    pwm.setDutyCycle(0);
    assert(pwm.dutyCycle() == 0);

    pwm.setDutyCycle(100);
    assert(pwm.dutyCycle() == 100);

    std::cout << "[PASS] test_pwm_frequency_duty_changes\n";
}

void test_pwm_output_state_transitions() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    // Disabled -> Output LOW
    assert(!pwm.outputState());

    pwm.enable();

    // At tick 0 -> Output HIGH
    assert(pwm.outputState());

    // Step PWM controller 500 ticks -> Output LOW
    for (std::uint32_t i = 0; i < 500; ++i) {
        pwm.tick();
    }
    assert(!pwm.outputState());

    // Step PWM controller 500 ticks -> wraps period -> Output HIGH
    for (std::uint32_t i = 0; i < 500; ++i) {
        pwm.tick();
    }
    assert(pwm.outputState());

    pwm.disable();
    assert(!pwm.outputState());

    std::cout << "[PASS] test_pwm_output_state_transitions\n";
}

void test_pwm_invalid_values() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    // Invalid frequency 0
    bool zero_freq = false;
    try {
        pwm.setFrequency(0);
    } catch (const std::invalid_argument&) {
        zero_freq = true;
    }
    if (!zero_freq) {
        throw std::runtime_error("Expected invalid_argument exception for zero frequency");
    }
    assert(zero_freq);

    // Invalid frequency > system clock
    bool high_freq = false;
    try {
        pwm.setFrequency(2000000);
    } catch (const std::invalid_argument&) {
        high_freq = true;
    }
    if (!high_freq) {
        throw std::runtime_error("Expected invalid_argument exception for frequency > system clock");
    }
    assert(high_freq);

    // Invalid duty cycle > 100%
    bool high_duty = false;
    try {
        pwm.setDutyCycle(101);
    } catch (const std::invalid_argument&) {
        high_duty = true;
    }
    if (!high_duty) {
        throw std::runtime_error("Expected invalid_argument exception for duty cycle > 100");
    }
    assert(high_duty);

    std::cout << "[PASS] test_pwm_invalid_values\n";
}

void test_pwm_hal_interface() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    PWMHAL hal(&pwm);
    assert(hal.isAttached());

    assert(!hal.enabled());
    hal.enable();
    assert(hal.enabled());

    hal.setFrequency(2000);
    assert(hal.frequency() == 2000);
    assert(pwm.frequency() == 2000);

    hal.setDutyCycle(75);
    assert(hal.dutyCycle() == 75);
    assert(pwm.dutyCycle() == 75);

    hal.disable();
    assert(!hal.enabled());

    // Unattached HAL
    PWMHAL unattached;
    assert(!unattached.isAttached());
    bool threw_unattached = false;
    try {
        unattached.enable();
    } catch (const std::runtime_error&) {
        threw_unattached = true;
    }
    if (!threw_unattached) {
        throw std::runtime_error("Expected runtime_error exception for unattached PWMHAL");
    }
    assert(threw_unattached);

    std::cout << "[PASS] test_pwm_hal_interface\n";
}

void test_pwm_reset_behaviour() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 2000, 75, 1000000);
    pwm.enable();

    pwm.reset();
    assert(!pwm.enabled());
    assert(pwm.frequency() == 1000);
    assert(pwm.dutyCycle() == 50);

    std::cout << "[PASS] test_pwm_reset_behaviour\n";
}

void test_pwm_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    PWMController pwm(bus, 0x40006000);

    systemBus.attachPWM(&pwm);
    assert(systemBus.pwm() == &pwm);

    pwm.enable();
    systemBus.reset();
    assert(!pwm.enabled());

    std::cout << "[PASS] test_pwm_system_bus_integration\n";
}

int main() {
    std::cout << "Running Pulse Width Modulation (PWM) unit tests...\n";
    test_pwm_initialization();
    test_pwm_frequency_duty_changes();
    test_pwm_output_state_transitions();
    test_pwm_invalid_values();
    test_pwm_hal_interface();
    test_pwm_reset_behaviour();
    test_pwm_system_bus_integration();
    std::cout << "All PWM unit tests passed successfully.\n";
    return 0;
}
