#include "drivers/i2c/i2c_controller.hpp"
#include "hal/i2c_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace efs::drivers::i2c;
using namespace efs::hal;
using namespace efs::mmio;
using namespace efs::system;

void test_i2c_initialization() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000, 0x50);

    assert(bus.contains(i2c.controlAddress()));
    assert(bus.contains(i2c.statusAddress()));
    assert(bus.contains(i2c.slaveAddrAddress()));
    assert(bus.contains(i2c.dataAddress()));

    assert(!i2c.enabled());
    assert(i2c.slaveAddress() == 0x50);
    assert(!i2c.busy());

    i2c.enable();
    assert(i2c.enabled());

    i2c.disable();
    assert(!i2c.enabled());

    std::cout << "[PASS] test_i2c_initialization\n";
}

void test_i2c_device_attachment() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);

    SimulatedI2CDevice dev1(0x50);
    SimulatedI2CDevice dev2(0x68);

    i2c.attachDevice(&dev1);
    i2c.attachDevice(&dev2);
    i2c.attachDevice(&dev1); // Duplicate safely ignored

    i2c.detachDevice(&dev1);

    std::cout << "[PASS] test_i2c_device_attachment\n";
}

void test_i2c_write_transaction() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice EEPROM(0x50);
    i2c.attachDevice(&EEPROM);

    i2c.setSlaveAddress(0x50);
    bool start_ok = i2c.start(false /* isRead = false for Write */);
    if (!start_ok) {
        throw std::runtime_error("I2C start failed");
    }
    assert(start_ok);
    assert(i2c.busy());
    assert(i2c.slaveAddress() == 0x50);

    bool w1 = i2c.writeByte(0x10); // Register address
    bool w2 = i2c.writeByte(0xAB); // Data byte
    if (!w1 || !w2) {
        throw std::runtime_error("I2C writeByte failed");
    }
    assert(w1);
    assert(w2);

    i2c.stop();
    assert(!i2c.busy());

    assert(EEPROM.receivedBytes().size() == 2);
    assert(EEPROM.receivedBytes()[0] == 0x10);
    assert(EEPROM.receivedBytes()[1] == 0xAB);

    std::cout << "[PASS] test_i2c_write_transaction\n";
}

void test_i2c_read_transaction() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice sensor(0x68);
    sensor.queueResponseByte(0x42);
    sensor.queueResponseByte(0x99);
    i2c.attachDevice(&sensor);

    i2c.setSlaveAddress(0x68);
    bool start_ok = i2c.start(true /* isRead = true for Read */);
    if (!start_ok) {
        throw std::runtime_error("I2C start failed");
    }
    assert(start_ok);

    std::uint8_t b1 = i2c.readByte();
    std::uint8_t b2 = i2c.readByte();
    if (b1 != 0x42 || b2 != 0x99) {
        throw std::runtime_error("I2C readByte failed");
    }
    assert(b1 == 0x42);
    assert(b2 == 0x99);

    i2c.stop();
    assert(!i2c.busy());

    std::cout << "[PASS] test_i2c_read_transaction\n";
}

void test_i2c_nack_on_unattached_address() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    // Start with address that has no attached device -> returns false (NACK)
    i2c.setSlaveAddress(0x3C);
    bool start_ok = i2c.start(false);
    if (start_ok) {
        throw std::runtime_error("Start on unattached address should return false");
    }
    assert(!start_ok);
    assert(i2c.busy());

    i2c.stop();
    assert(!i2c.busy());

    std::cout << "[PASS] test_i2c_nack_on_unattached_address\n";
}

void test_i2c_reset_behaviour() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice dev(0x50);
    i2c.attachDevice(&dev);
    i2c.setSlaveAddress(0x50);
    i2c.start(false);

    i2c.reset();
    assert(!i2c.enabled());
    assert(!i2c.busy());

    std::cout << "[PASS] test_i2c_reset_behaviour\n";
}

void test_i2c_hal_interface() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    SimulatedI2CDevice sensor(0x68);
    sensor.queueResponseByte(0x12);
    sensor.queueResponseByte(0x34);
    i2c.attachDevice(&sensor);

    I2CHAL hal(&i2c);
    assert(hal.isAttached());
    hal.enable();

    // Write transaction via HAL
    bool tx_ok = hal.beginTransmission(0x68);
    if (!tx_ok) {
        throw std::runtime_error("HAL beginTransmission failed");
    }
    assert(tx_ok);

    bool w_ok = hal.writeByte(0x75); // WHO_AM_I register
    if (!w_ok) {
        throw std::runtime_error("HAL writeByte failed");
    }
    assert(w_ok);

    bool end_ok = hal.endTransmission();
    if (!end_ok) {
        throw std::runtime_error("HAL endTransmission failed");
    }
    assert(end_ok);

    // Read transaction via HAL requestFrom
    std::size_t count = hal.requestFrom(0x68, 2);
    if (count != 2) {
        throw std::runtime_error("HAL requestFrom failed");
    }
    assert(count == 2);
    assert(hal.available() == 2);

    std::uint8_t r1 = hal.readByte();
    std::uint8_t r2 = hal.readByte();
    if (r1 != 0x12 || r2 != 0x34) {
        throw std::runtime_error("HAL readByte failed");
    }
    assert(r1 == 0x12);
    assert(r2 == 0x34);
    assert(hal.available() == 0);

    hal.disable();
    assert(!hal.enabled());

    // Unattached HAL error handling
    I2CHAL unattached;
    assert(!unattached.isAttached());
    bool threw_unattached = false;
    try {
        bool ok = unattached.beginTransmission(0x50);
        if (ok) {
            throw std::runtime_error("Unexpected tx ok");
        }
    } catch (const std::runtime_error&) {
        threw_unattached = true;
    }
    if (!threw_unattached) {
        throw std::runtime_error("Expected runtime_error exception for unattached I2CHAL");
    }
    assert(threw_unattached);

    std::cout << "[PASS] test_i2c_hal_interface\n";
}

void test_i2c_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    I2CController i2c(bus, 0x40005000);

    systemBus.attachI2C(&i2c);
    assert(systemBus.i2c() == &i2c);

    i2c.enable();
    SimulatedI2CDevice dev(0x50);
    i2c.attachDevice(&dev);

    i2c.setSlaveAddress(0x50);
    i2c.start(false);
    systemBus.reset();
    assert(!i2c.enabled());

    std::cout << "[PASS] test_i2c_system_bus_integration\n";
}

int main() {
    std::cout << "Running I2C Controller unit tests...\n";
    test_i2c_initialization();
    test_i2c_device_attachment();
    test_i2c_write_transaction();
    test_i2c_read_transaction();
    test_i2c_nack_on_unattached_address();
    test_i2c_reset_behaviour();
    test_i2c_hal_interface();
    test_i2c_system_bus_integration();
    std::cout << "All I2C Controller unit tests passed successfully.\n";
    return 0;
}
