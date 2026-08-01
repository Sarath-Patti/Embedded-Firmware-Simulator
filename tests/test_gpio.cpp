#include "drivers/gpio/gpio.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace efs::drivers::gpio;
using namespace efs::mmio;

void test_gpio_register_creation() {
    MMIOBus bus;
    {
        GPIO gpio(bus, 0x40000000);
        assert(bus.contains(0x40000000)); // DIR
        assert(bus.contains(0x40000004)); // OUT
        assert(bus.contains(0x40000008)); // IN
    }
    // RAII destructor unregisters
    assert(!bus.contains(0x40000000));
    assert(!bus.contains(0x40000004));
    assert(!bus.contains(0x40000008));

    std::cout << "[PASS] test_gpio_register_creation\n";
}

void test_pin_configuration() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    gpio.configurePin(5, PinDirection::Output);
    assert(bus.read(0x40000000) == (1U << 5));

    gpio.configurePin(5, PinDirection::Input);
    assert(bus.read(0x40000000) == 0);

    std::cout << "[PASS] test_pin_configuration\n";
}

void test_write_read() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    gpio.configurePin(3, PinDirection::Output);
    gpio.writePin(3, PinState::High);

    assert(gpio.readPin(3) == PinState::High);
    assert(bus.read(0x40000004) == (1U << 3));
    assert(bus.read(0x40000008) == (1U << 3));

    gpio.writePin(3, PinState::Low);
    assert(gpio.readPin(3) == PinState::Low);
    assert(bus.read(0x40000004) == 0);
    assert(bus.read(0x40000008) == 0);

    std::cout << "[PASS] test_write_read\n";
}

void test_toggle() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    gpio.configurePin(7, PinDirection::Output);
    gpio.writePin(7, PinState::Low);

    gpio.togglePin(7);
    assert(gpio.readPin(7) == PinState::High);

    gpio.togglePin(7);
    assert(gpio.readPin(7) == PinState::Low);

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
    assert(pin_threw);
    (void)pin_threw;

    bool read_threw = false;
    try {
        [[maybe_unused]] auto s = gpio.readPin(100);
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    assert(read_threw);
    (void)read_threw;

    std::cout << "[PASS] test_invalid_pin\n";
}

void test_invalid_direction() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);

    // Pin 2 is input by default
    bool write_threw = false;
    try {
        gpio.writePin(2, PinState::High);
    } catch (const std::logic_error&) {
        write_threw = true;
    }
    assert(write_threw);
    (void)write_threw;

    bool toggle_threw = false;
    try {
        gpio.togglePin(2);
    } catch (const std::logic_error&) {
        toggle_threw = true;
    }
    assert(toggle_threw);
    (void)toggle_threw;

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

    assert(gpio.readPin(0) == PinState::High);

    std::cout << "[PASS] test_mmio_register_interaction\n";
}

int main() {
    std::cout << "Running GPIO unit tests...\n";
    test_gpio_register_creation();
    test_pin_configuration();
    test_write_read();
    test_toggle();
    test_invalid_pin();
    test_invalid_direction();
    test_mmio_register_interaction();
    std::cout << "All GPIO unit tests passed successfully.\n";
    return 0;
}
