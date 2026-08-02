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

void test_pwm_controller_initialization() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    assert(bus.contains(pwm.controlAddress()));
    assert(bus.contains(pwm.freqAddress()));
    assert(bus.contains(pwm.dutyAddress()));
    assert(bus.contains(pwm.statusAddress()));

    assert(!pwm.enabled());
    assert(pwm.frequency() == 1000);
    assert(pwm.dutyCycle() == 50);
    assert(!pwm.outputState());

    pwm.enable();
    assert(pwm.enabled());
    assert((bus.read(pwm.controlAddress()) & PWMController::CTRL_ENABLE_BIT) != 0);

    pwm.disable();
    assert(!pwm.enabled());

    std::cout << "[PASS] test_pwm_controller_initialization\n";
}

void test_pwm_frequency_configuration() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    pwm.setFrequency(5000);
    assert(pwm.frequency() == 5000);
    assert(bus.read(pwm.freqAddress()) == 5000);

    pwm.setFrequency(100);
    assert(pwm.frequency() == 100);
    assert(bus.read(pwm.freqAddress()) == 100);

    std::cout << "[PASS] test_pwm_frequency_configuration\n";
}

void test_pwm_duty_cycle_configuration() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000, 50, 1000000);

    pwm.setDutyCycle(75);
    assert(pwm.dutyCycle() == 75);
    assert(bus.read(pwm.dutyAddress()) == 75);

    pwm.setDutyCycle(0);
    assert(pwm.dutyCycle() == 0);

    pwm.setDutyCycle(100);
    assert(pwm.dutyCycle() == 100);

    std::cout << "[PASS] test_pwm_duty_cycle_configuration\n";
}

void test_pwm_output_state_transitions() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 1000 /* 1000 ticks period */, 25 /* 250 ticks HIGH, 750 ticks LOW */, 1000000);
    pwm.enable();

    // 0% duty cycle should stay LOW
    pwm.setDutyCycle(0);
    assert(!pwm.outputState());

    // 100% duty cycle should stay HIGH
    pwm.setDutyCycle(100);
    assert(pwm.outputState());

    // 25% duty cycle: HIGH for first 250 ticks, LOW for remaining 750 ticks
    pwm.setDutyCycle(25);
    pwm.timer().reset();

    // At tick 0 (counter < 250) -> HIGH
    assert(pwm.outputState());

    // Tick 200 cycles -> counter = 200 < 250 -> HIGH
    for (int i = 0; i < 200; ++i) {
        pwm.tick();
    }
    assert(pwm.outputState());

    // Tick another 100 cycles -> counter = 300 >= 250 -> LOW
    for (int i = 0; i < 100; ++i) {
        pwm.tick();
    }
    assert(!pwm.outputState());

    std::cout << "[PASS] test_pwm_output_state_transitions\n";
}

void test_pwm_reset_behaviour() {
    MMIOBus bus;
    PWMController pwm(bus, 0x40006000, 5000, 80, 1000000);
    pwm.enable();
    assert(pwm.enabled());

    pwm.reset();
    assert(!pwm.enabled());
    assert(!pwm.outputState());

    std::cout << "[PASS] test_pwm_reset_behaviour\n";
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
    assert(zero_freq);
    (void)zero_freq;

    // Invalid frequency > system clock
    bool high_freq = false;
    try {
        pwm.setFrequency(2000000);
    } catch (const std::invalid_argument&) {
        high_freq = true;
    }
    assert(high_freq);
    (void)high_freq;

    // Invalid duty cycle > 100%
    bool high_duty = false;
    try {
        pwm.setDutyCycle(101);
    } catch (const std::invalid_argument&) {
        high_duty = true;
    }
    assert(high_duty);
    (void)high_duty;

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
    assert(threw_unattached);
    (void)threw_unattached;

    std::cout << "[PASS] test_pwm_hal_interface\n";
}

void test_pwm_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    PWMController pwm(bus, 0x40006000);

    systemBus.attachPWM(&pwm);
    assert(systemBus.pwm() == &pwm);

    pwm.enable();
    systemBus.tickTimers();

    systemBus.reset();
    assert(!pwm.enabled());

    std::cout << "[PASS] test_pwm_system_bus_integration\n";
}

int main() {
    std::cout << "Running PWM Controller unit tests...\n";
    test_pwm_controller_initialization();
    test_pwm_frequency_configuration();
    test_pwm_duty_cycle_configuration();
    test_pwm_output_state_transitions();
    test_pwm_reset_behaviour();
    test_pwm_invalid_values();
    test_pwm_hal_interface();
    test_pwm_system_bus_integration();
    std::cout << "All PWM Controller unit tests passed successfully.\n";
    return 0;
}
