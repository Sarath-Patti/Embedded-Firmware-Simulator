#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "firmware/basic_firmware.hpp"
#include "firmware/firmware_manager.hpp"
#include "firmware/timer_blink_firmware.hpp"
#include "firmware/uart_echo_firmware.hpp"
#include "hal/hal.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

using namespace efs::firmware;
using namespace efs::hal;
using namespace efs::drivers::gpio;
using namespace efs::drivers::timer;
using namespace efs::drivers::uart;
using namespace efs::mmio;
using namespace efs::cpu;

class MockLifecycleFirmware : public Firmware {
public:
    ~MockLifecycleFirmware() override = default;

    void initialize() override {
        init_count++;
    }
    void update() override {
        update_count++;
    }
    void shutdown() override {
        shutdown_count++;
    }
    void reset() override {
        reset_count++;
    }

    int init_count{0};
    int update_count{0};
    int shutdown_count{0};
    int reset_count{0};
};

void test_registration_and_lookup() {
    FirmwareManager mgr;
    assert(mgr.count() == 0);
    assert(mgr.activeFirmware() == nullptr);
    assert(mgr.activeFirmwareName().empty());

    auto fw1 = std::make_shared<MockLifecycleFirmware>();
    auto fw2 = std::make_shared<MockLifecycleFirmware>();

    bool reg1 = mgr.registerFirmware("Firmware1", fw1);
    assert(reg1);
    (void)reg1;
    assert(mgr.count() == 1);
    assert(mgr.hasFirmware("Firmware1"));
    assert(mgr.activeFirmwareName() == "Firmware1");

    bool reg2 = mgr.registerFirmware("Firmware2", fw2);
    assert(reg2);
    (void)reg2;
    assert(mgr.count() == 2);
    assert(mgr.hasFirmware("Firmware2"));

    auto names = mgr.registeredNames();
    assert(names.size() == 2);
    assert(names[0] == "Firmware1");
    assert(names[1] == "Firmware2");

    std::cout << "[PASS] test_registration_and_lookup\n";
}

void test_active_selection_and_switching() {
    FirmwareManager mgr;
    auto fw1 = std::make_shared<MockLifecycleFirmware>();
    auto fw2 = std::make_shared<MockLifecycleFirmware>();

    mgr.registerFirmware("App1", fw1);
    mgr.registerFirmware("App2", fw2);

    assert(mgr.activeFirmwareName() == "App1");
    mgr.initialize();
    assert(fw1->init_count == 1);

    // Switching active firmware shuts down App1
    bool switch_ok = mgr.setActiveFirmware("App2");
    assert(switch_ok);
    (void)switch_ok;

    assert(fw1->shutdown_count == 1);
    assert(mgr.activeFirmwareName() == "App2");

    mgr.initialize();
    assert(fw2->init_count == 1);

    std::cout << "[PASS] test_active_selection_and_switching\n";
}

void test_lifecycle_forwarding() {
    FirmwareManager mgr;
    auto fw = std::make_shared<MockLifecycleFirmware>();

    mgr.registerFirmware("App", fw);
    mgr.setActiveFirmware("App");

    mgr.initialize();
    assert(fw->init_count == 1);

    mgr.update();
    mgr.update();
    assert(fw->update_count == 2);

    mgr.reset();
    assert(fw->reset_count == 1);

    mgr.shutdown();
    assert(fw->shutdown_count == 1);

    std::cout << "[PASS] test_lifecycle_forwarding\n";
}

void test_unregistration_behavior() {
    FirmwareManager mgr;
    auto fw1 = std::make_shared<MockLifecycleFirmware>();
    auto fw2 = std::make_shared<MockLifecycleFirmware>();

    mgr.registerFirmware("A", fw1);
    mgr.registerFirmware("B", fw2);

    mgr.setActiveFirmware("A");
    mgr.initialize();

    // Unregistering active firmware shuts it down and selects remaining
    bool unreg_ok = mgr.unregisterFirmware("A");
    assert(unreg_ok);
    (void)unreg_ok;

    assert(fw1->shutdown_count == 1);
    assert(!mgr.hasFirmware("A"));
    assert(mgr.count() == 1);
    assert(mgr.activeFirmwareName() == "B");

    std::cout << "[PASS] test_unregistration_behavior\n";
}

void test_invalid_inputs_and_edge_cases() {
    FirmwareManager mgr;

    // Registering null or empty name fails
    assert(!mgr.registerFirmware("", std::make_shared<MockLifecycleFirmware>()));
    assert(!mgr.registerFirmware("Valid", nullptr));

    // Duplicate registration fails
    auto fw = std::make_shared<MockLifecycleFirmware>();
    assert(mgr.registerFirmware("Unique", fw));
    assert(!mgr.registerFirmware("Unique", fw));

    // Unregistering non-existent name fails
    assert(!mgr.unregisterFirmware("NonExistent"));

    // Setting non-existent active fails
    assert(!mgr.setActiveFirmware("Unknown"));

    // Empty manager operations are safe no-ops
    FirmwareManager emptyMgr;
    emptyMgr.initialize();
    emptyMgr.update();
    emptyMgr.reset();
    emptyMgr.shutdown();
    assert(emptyMgr.activeFirmware() == nullptr);

    std::cout << "[PASS] test_invalid_inputs_and_edge_cases\n";
}

void test_concrete_firmware_timer_blink() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    Timer timer(bus, 0x40001000);
    GPIOHAL gpioHAL(gpio);
    TimerHAL timerHAL(timer);

    constexpr std::uint8_t LED_PIN = 3;
    constexpr efs::common::Size COMPARE_VAL = 2;
    auto blinkFw = std::make_shared<TimerBlinkFirmware>(gpioHAL, timerHAL, LED_PIN, COMPARE_VAL);

    FirmwareManager mgr;
    mgr.registerFirmware("TimerBlink", blinkFw);
    mgr.setActiveFirmware("TimerBlink");

    mgr.initialize();
    assert(blinkFw->isInitialized());
    assert(!gpioHAL.read(LED_PIN));

    // Tick timer 2 times to trigger compare match
    timer.tick();
    mgr.update();
    assert(!gpioHAL.read(LED_PIN));

    timer.tick();
    mgr.update(); // Match triggers toggle!
    assert(gpioHAL.read(LED_PIN));
    assert(blinkFw->toggleCount() == 1);

    mgr.reset();
    assert(!blinkFw->isInitialized());
    assert(blinkFw->toggleCount() == 0);

    std::cout << "[PASS] test_concrete_firmware_timer_blink\n";
}

void test_concrete_firmware_uart_echo() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);
    UARTHAL uartHAL(uart);

    constexpr std::uint32_t BAUD = 115200U;
    auto echoFw = std::make_shared<UARTEchoFirmware>(uartHAL, BAUD);

    FirmwareManager mgr;
    mgr.registerFirmware("UARTEcho", echoFw);
    mgr.setActiveFirmware("UARTEcho");

    mgr.initialize();
    assert(echoFw->isInitialized());

    // Push bytes into RX FIFO
    uart.pushReceivedByte(static_cast<std::uint8_t>('Z'));
    assert(uart.hasReceivedData());

    mgr.update(); // Reads from RX, writes to TX via HAL

    assert(echoFw->echoCount() == 1);
    assert(uart.txFifoSize() == 1);
    assert(uart.popTxByte() == 'Z');

    mgr.shutdown();
    assert(echoFw->isShutdown());

    std::cout << "[PASS] test_concrete_firmware_uart_echo\n";
}

void test_cpu_integration_with_firmware_manager() {
    MMIOBus bus;
    GPIO gpio(bus, 0x40000000);
    GPIOHAL gpioHAL(gpio);

    CPU cpu;
    constexpr std::uint8_t PIN = 4;
    constexpr efs::common::Size INTERVAL = 2;
    auto basicFw = std::make_shared<BasicFirmware>(gpioHAL, PIN, INTERVAL);

    bool load_ok = cpu.loadFirmware(basicFw, "Basic");
    assert(load_ok);
    (void)load_ok;
    assert(cpu.firmwareLoaded());
    assert(cpu.firmwareManager().activeFirmwareName() == "Basic");

    cpu.start();
    assert(basicFw->isInitialized());

    cpu.step(); // Cycle 1
    cpu.step(); // Cycle 2 -> Toggles
    assert(gpioHAL.read(PIN));

    cpu.reset();
    assert(cpu.cycleCount() == 0);
    assert(!basicFw->isInitialized());

    std::cout << "[PASS] test_cpu_integration_with_firmware_manager\n";
}

int main() {
    std::cout << "Running FirmwareManager unit tests...\n";
    test_registration_and_lookup();
    test_active_selection_and_switching();
    test_lifecycle_forwarding();
    test_unregistration_behavior();
    test_invalid_inputs_and_edge_cases();
    test_concrete_firmware_timer_blink();
    test_concrete_firmware_uart_echo();
    test_cpu_integration_with_firmware_manager();
    std::cout << "All FirmwareManager unit tests passed successfully.\n";
    return 0;
}
