#include "hal/hal.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "firmware/basic_firmware.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>

using namespace efs::hal;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::drivers::uart;
using namespace efs::mmio;
using namespace efs::firmware;

void test_gpio_hal() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    GPIOHAL hal(gpio);

    assert(hal.isAttached());

    constexpr std::uint8_t PIN3 = 3;
    constexpr std::uint8_t PIN4 = 4;
    constexpr std::uint8_t PIN1 = 1;

    hal.configureOutput(PIN3);
    assert(!hal.read(PIN3));

    hal.write(PIN3, true);
    assert(hal.read(PIN3));

    hal.toggle(PIN3);
    assert(!hal.read(PIN3));

    hal.configureInput(PIN4);
    gpio.setExternalInput(PIN4, PinState::High);
    assert(hal.read(PIN4));

    // Detached check
    GPIOHAL unattached;
    assert(!unattached.isAttached());
    unattached.configureOutput(PIN1);
    assert(!unattached.read(PIN1));

    (void)PIN3;
    (void)PIN4;
    (void)PIN1;

    std::cout << "[PASS] test_gpio_hal\n";
}

void test_timer_hal() {
    MMIOBus bus;
    Timer timer(bus, 0x40001000);
    TimerHAL hal(timer);

    assert(hal.isAttached());
    assert(!hal.running());
    assert(hal.counter() == 0);

    hal.setCompare(50);
    hal.start();
    assert(hal.running());

    timer.tick();
    assert(hal.counter() == 1);

    hal.stop();
    assert(!hal.running());

    hal.reset();
    assert(hal.counter() == 0);

    // Detached check
    TimerHAL unattached;
    assert(!unattached.isAttached());
    assert(!unattached.running());
    assert(unattached.counter() == 0);

    std::cout << "[PASS] test_timer_hal\n";
}

void test_uart_hal() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);
    UARTHAL hal(uart);

    assert(hal.isAttached());
    assert(hal.enabled());

    hal.setBaudRate(9600);

    assert(!hal.hasData());
    uart.pushReceivedByte('X');
    assert(hal.hasData());
    assert(hal.readByte() == 'X');

    hal.writeByte('Y');
    assert(uart.txFifoSize() == 1);
    assert(uart.popTxByte() == 'Y');

    hal.disable();
    assert(!hal.enabled());
    hal.enable();
    assert(hal.enabled());

    // Detached check
    UARTHAL unattached;
    assert(!unattached.isAttached());
    assert(!unattached.hasData());
    assert(unattached.readByte() == 0);

    std::cout << "[PASS] test_uart_hal\n";
}

void test_firmware_using_hal() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    GPIOHAL hal(gpio);
    constexpr std::uint8_t PIN1 = 1;
    constexpr efs::common::Size TOGGLE_INTERVAL = 2;
    BasicFirmware fw(hal, PIN1, TOGGLE_INTERVAL);

    fw.initialize();
    assert(fw.isInitialized());
    assert(!hal.read(PIN1));

    fw.execute(); // cycle 1
    assert(!hal.read(PIN1));

    fw.execute(); // cycle 2 -> toggles
    assert(hal.read(PIN1));

    fw.execute(); // cycle 3
    assert(hal.read(PIN1));

    fw.execute(); // cycle 4 -> toggles
    assert(!hal.read(PIN1));

    fw.shutdown();
    assert(fw.isShutdown());
    assert(!hal.read(PIN1));
    (void)PIN1;

    std::cout << "[PASS] test_firmware_using_hal\n";
}

int main() {
    std::cout << "Running Hardware Abstraction Layer (HAL) unit tests...\n";
    test_gpio_hal();
    test_timer_hal();
    test_uart_hal();
    test_firmware_using_hal();
    std::cout << "All HAL unit tests passed successfully.\n";
    return 0;
}
