#include "cpu/cpu.hpp"
#include "drivers/gpio/gpio.hpp"
#include "firmware/basic_firmware.hpp"
#include "firmware/firmware.hpp"
#include "mmio/mmio_bus.hpp"
#include <cassert>
#include <iostream>
#include <memory>

class MockFirmware : public efs::firmware::Firmware {
public:
    ~MockFirmware() override = default;
    void initialize() override {
        init_count++;
    }
    void execute() override {
        exec_count++;
    }
    void shutdown() override {
        shutdown_count++;
    }

    int init_count{0};
    int exec_count{0};
    int shutdown_count{0};
};

void test_firmware_loading() {
    efs::cpu::CPU cpu;
    assert(!cpu.firmwareLoaded());

    auto fw = std::make_shared<MockFirmware>();
    bool load_ok = cpu.loadFirmware(fw);
    assert(load_ok);
    (void)load_ok;
    assert(cpu.firmwareLoaded());
    assert(cpu.firmware() == fw);

    std::cout << "[PASS] test_firmware_loading\n";
}

void test_firmware_unloading() {
    efs::cpu::CPU cpu;
    auto fw = std::make_shared<MockFirmware>();

    cpu.loadFirmware(fw);
    assert(cpu.firmwareLoaded());

    cpu.unloadFirmware();
    assert(!cpu.firmwareLoaded());

    std::cout << "[PASS] test_firmware_unloading\n";
}

void test_initialize_called_once() {
    efs::cpu::CPU cpu;
    auto fw = std::make_shared<MockFirmware>();
    cpu.loadFirmware(fw);

    cpu.start();
    cpu.run(5);
    cpu.stop();

    assert(fw->init_count == 1);

    std::cout << "[PASS] test_initialize_called_once\n";
}

void test_execute_called_once_per_step() {
    efs::cpu::CPU cpu;
    auto fw = std::make_shared<MockFirmware>();
    cpu.loadFirmware(fw);

    cpu.start();
    cpu.run(7);
    cpu.stop();

    assert(fw->exec_count == 7);

    std::cout << "[PASS] test_execute_called_once_per_step\n";
}

void test_shutdown_called_once() {
    efs::cpu::CPU cpu;
    auto fw = std::make_shared<MockFirmware>();
    cpu.loadFirmware(fw);

    cpu.start();
    cpu.run(3);
    cpu.stop();

    assert(fw->shutdown_count == 1);

    std::cout << "[PASS] test_shutdown_called_once\n";
}

void test_cpu_operation_without_firmware() {
    efs::cpu::CPU cpu;
    assert(!cpu.firmwareLoaded());

    cpu.start();
    cpu.run(5);
    cpu.stop();

    assert(cpu.cycleCount() == 5);

    std::cout << "[PASS] test_cpu_operation_without_firmware\n";
}

void test_basic_firmware_gpio_behavior() {
    efs::mmio::MMIOBus bus;
    efs::drivers::gpio::GPIO gpio(bus, 0x40000000);
    efs::cpu::CPU cpu;

    constexpr std::uint8_t TARGET_PIN = 4;
    auto fw = std::make_shared<efs::firmware::BasicFirmware>(gpio, TARGET_PIN, 2);
    cpu.loadFirmware(fw);

    cpu.start();
    assert(fw->isInitialized());
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::Low);

    // Step 1: exec_count = 1, cycleCounter = 1 -> Pin remains Low
    cpu.step();
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::Low);

    // Step 2: exec_count = 2, cycleCounter = 2 -> Toggles to High
    cpu.step();
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::High);

    // Step 3: exec_count = 3, cycleCounter = 1 -> Pin remains High
    cpu.step();
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::High);

    // Step 4: exec_count = 4, cycleCounter = 2 -> Toggles to Low
    cpu.step();
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::Low);

    cpu.stop();
    assert(fw->isShutdown());
    assert(gpio.readPin(TARGET_PIN) == efs::drivers::gpio::PinState::Low);

    std::cout << "[PASS] test_basic_firmware_gpio_behavior\n";
}

int main() {
    std::cout << "Running Firmware unit tests...\n";
    test_firmware_loading();
    test_firmware_unloading();
    test_initialize_called_once();
    test_execute_called_once_per_step();
    test_shutdown_called_once();
    test_cpu_operation_without_firmware();
    test_basic_firmware_gpio_behavior();
    std::cout << "All Firmware unit tests passed successfully.\n";
    return 0;
}
