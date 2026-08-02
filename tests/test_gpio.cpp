#include "drivers/gpio/gpio.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace efs::drivers::gpio;
using namespace efs::mmio;

void test_gpio_register_creation() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    assert(bus.contains(0x40000000)); // DIR
    assert(bus.contains(0x40000004)); // OUT
    assert(bus.contains(0x40000008)); // IN

    std::cout << "[PASS] test_gpio_register_creation\n";
}

void test_pin_configuration() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    constexpr std::uint8_t PIN0 = 0;

    assert((bus.read(gpio.dirAddress()) & (1U << PIN0)) == 0);

    gpio.configurePin(PIN0, PinDirection::Output);
    assert((bus.read(gpio.dirAddress()) & (1U << PIN0)) != 0);

    gpio.configurePin(PIN0, PinDirection::Input);
    assert((bus.read(gpio.dirAddress()) & (1U << PIN0)) == 0);

    std::cout << "[PASS] test_pin_configuration\n";
}

void test_write_and_read() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    constexpr std::uint8_t PIN5 = 5;

    gpio.configurePin(PIN5, PinDirection::Output);
    assert(gpio.readPin(PIN5) == PinState::Low);

    gpio.writePin(PIN5, PinState::High);
    assert(gpio.readPin(PIN5) == PinState::High);

    gpio.writePin(PIN5, PinState::Low);
    assert(gpio.readPin(PIN5) == PinState::Low);

    std::cout << "[PASS] test_write_and_read\n";
}

void test_toggle() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    constexpr std::uint8_t PIN7 = 7;

    gpio.configurePin(PIN7, PinDirection::Output);
    assert(gpio.readPin(PIN7) == PinState::Low);

    gpio.togglePin(PIN7);
    assert(gpio.readPin(PIN7) == PinState::High);

    gpio.togglePin(PIN7);
    assert(gpio.readPin(PIN7) == PinState::Low);

    std::cout << "[PASS] test_toggle\n";
}

void test_invalid_pin() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    bool pin_threw = false;
    try {
        gpio.configurePin(32, PinDirection::Output);
    } catch (const std::out_of_range&) {
        pin_threw = true;
    }
    if (!pin_threw) {
        throw std::runtime_error("Expected out_of_range exception for pin 32 configure");
    }
    assert(pin_threw);

    bool read_threw = false;
    try {
        PinState s = gpio.readPin(100);
        if (s != PinState::Low && s != PinState::High) {
            throw std::runtime_error("Invalid state");
        }
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    if (!read_threw) {
        throw std::runtime_error("Expected out_of_range exception for pin 100 read");
    }
    assert(read_threw);

    std::cout << "[PASS] test_invalid_pin\n";
}

void test_invalid_direction() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    constexpr std::uint8_t PIN2 = 2;
    // Pin 2 is input by default
    bool write_threw = false;
    try {
        gpio.writePin(PIN2, PinState::High);
    } catch (const std::logic_error&) {
        write_threw = true;
    }
    if (!write_threw) {
        throw std::runtime_error("Expected logic_error exception for pin 2 write");
    }
    assert(write_threw);

    bool toggle_threw = false;
    try {
        gpio.togglePin(PIN2);
    } catch (const std::logic_error&) {
        toggle_threw = true;
    }
    if (!toggle_threw) {
        throw std::runtime_error("Expected logic_error exception for pin 2 toggle");
    }
    assert(toggle_threw);

    std::cout << "[PASS] test_invalid_direction\n";
}

void test_mmio_register_interaction() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    // Set Pin 0 as Output via MMIO bus write to DIR register
    bus.write(0x40000000, 0x00000001);
    // Write Pin 0 High via MMIO bus write to OUT and IN registers
    bus.write(0x40000004, 0x00000001);
    bus.write(0x40000008, 0x00000001);

    constexpr std::uint8_t PIN0 = 0;
    PinState p0State = gpio.readPin(PIN0);
    if (p0State != PinState::High) {
        throw std::runtime_error("Pin 0 state should be High");
    }
    assert(p0State == PinState::High);

    std::cout << "[PASS] test_mmio_register_interaction\n";
}

int main() {
    std::cout << "Running GPIO unit tests...\n";
    test_gpio_register_creation();
    test_pin_configuration();
    test_write_and_read();
    test_toggle();
    test_invalid_pin();
    test_invalid_direction();
    test_mmio_register_interaction();
    std::cout << "All GPIO unit tests passed successfully.\n";
    return 0;
}
