#include "drivers/i2c/i2c_controller.hpp"
#include "drivers/i2c/i2c_device.hpp"
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

void test_i2c_controller_initialization() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000, 0x50);

    assert(bus.contains(i2c.dataAddress()));
    assert(bus.contains(i2c.statusAddress()));
    assert(bus.contains(i2c.controlAddress()));
    assert(bus.contains(i2c.slaveAddrAddress()));

    assert(!i2c.enabled());
    assert(i2c.slaveAddress() == 0x50);
    assert(!i2c.busy());

    i2c.enable();
    assert(i2c.enabled());
    assert((bus.read(i2c.controlAddress()) & I2CController::CTRL_ENABLE_BIT) != 0);

    i2c.disable();
    assert(!i2c.enabled());

    std::cout << "[PASS] test_i2c_controller_initialization\n";
}

void test_i2c_master_write() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice eeprom(0x50);
    i2c.attachDevice(&eeprom);
    i2c.setSlaveAddress(0x50);

    bool start_ack = i2c.start(false /* write mode */);
    assert(start_ack);
    (void)start_ack;
    assert(i2c.busy());

    bool w1 = i2c.writeByte(0x10); // Register address
    bool w2 = i2c.writeByte(0xAB); // Data byte
    assert(w1);
    assert(w2);
    (void)w1;
    (void)w2;

    i2c.stop();
    assert(!i2c.busy());

    assert(eeprom.receivedBytes().size() == 2);
    assert(eeprom.receivedBytes()[0] == 0x10);
    assert(eeprom.receivedBytes()[1] == 0xAB);

    std::cout << "[PASS] test_i2c_master_write\n";
}

void test_i2c_master_read() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice sensor(0x68, 0x00);
    sensor.queueResponseByte(0x3B); // Accel X High
    sensor.queueResponseByte(0x7F); // Accel X Low
    i2c.attachDevice(&sensor);
    i2c.setSlaveAddress(0x68);

    bool start_ack = i2c.start(true /* read mode */);
    assert(start_ack);
    (void)start_ack;

    std::uint8_t b1 = i2c.readByte();
    std::uint8_t b2 = i2c.readByte();
    assert(b1 == 0x3B);
    assert(b2 == 0x7F);
    (void)b1;
    (void)b2;

    i2c.stop();
    assert(!i2c.busy());

    std::cout << "[PASS] test_i2c_master_read\n";
}

void test_i2c_ack_nack_behaviour() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice slave(0x3C);
    slave.setAcknowledge(false); // Force NACK
    i2c.attachDevice(&slave);
    i2c.setSlaveAddress(0x3C);

    bool start_ack = i2c.start(false);
    assert(!start_ack);
    (void)start_ack;
    assert(!i2c.lastAck());

    i2c.stop();

    std::cout << "[PASS] test_i2c_ack_nack_behaviour\n";
}

void test_i2c_invalid_address() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice slave(0x50);
    i2c.attachDevice(&slave);

    // Target unattached address 0x77
    i2c.setSlaveAddress(0x77);
    bool start_ack = i2c.start(false);
    assert(!start_ack);
    (void)start_ack;
    assert(!i2c.lastAck());

    i2c.stop();

    std::cout << "[PASS] test_i2c_invalid_address\n";
}

void test_i2c_multiple_transactions() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice dev1(0x10);
    SimulatedI2CDevice dev2(0x20);
    i2c.attachDevice(&dev1);
    i2c.attachDevice(&dev2);

    // Transaction 1 to Dev 1
    i2c.setSlaveAddress(0x10);
    assert(i2c.start(false));
    assert(i2c.writeByte(0xAA));
    i2c.stop();

    // Transaction 2 to Dev 2
    i2c.setSlaveAddress(0x20);
    assert(i2c.start(false));
    assert(i2c.writeByte(0xBB));
    i2c.stop();

    assert(dev1.receivedBytes().size() == 1);
    assert(dev1.receivedBytes()[0] == 0xAA);
    assert(dev2.receivedBytes().size() == 1);
    assert(dev2.receivedBytes()[0] == 0xBB);

    std::cout << "[PASS] test_i2c_multiple_transactions\n";
}

void test_i2c_reset_behaviour() {
    MMIOBus bus;
    I2CController i2c(bus, 0x40005000);
    i2c.enable();

    SimulatedI2CDevice slave(0x50);
    i2c.attachDevice(&slave);

    i2c.setSlaveAddress(0x50);
    (void)i2c.start(false);

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
    assert(tx_ok);
    (void)tx_ok;

    bool w_ok = hal.writeByte(0x75); // WHO_AM_I register
    assert(w_ok);
    (void)w_ok;

    bool end_ok = hal.endTransmission();
    assert(end_ok);
    (void)end_ok;

    // Read transaction via HAL requestFrom
    std::size_t count = hal.requestFrom(0x68, 2);
    assert(count == 2);
    assert(hal.available() == 2);
    (void)count;

    std::uint8_t r1 = hal.readByte();
    std::uint8_t r2 = hal.readByte();
    assert(r1 == 0x12);
    assert(r2 == 0x34);
    (void)r1;
    (void)r2;
    assert(hal.available() == 0);

    hal.disable();
    assert(!hal.enabled());

    // Unattached HAL error handling
    I2CHAL unattached;
    assert(!unattached.isAttached());
    bool threw_unattached = false;
    try {
        (void)unattached.beginTransmission(0x50);
    } catch (const std::runtime_error&) {
        threw_unattached = true;
    }
    assert(threw_unattached);
    (void)threw_unattached;

    std::cout << "[PASS] test_i2c_hal_interface\n";
}

void test_i2c_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    I2CController i2c(bus, 0x40005000);

    systemBus.attachI2C(&i2c);
    assert(systemBus.i2c() == &i2c);

    i2c.enable();
    SimulatedI2CDevice slave(0x50);
    i2c.attachDevice(&slave);

    systemBus.reset();
    assert(!i2c.enabled());

    std::cout << "[PASS] test_i2c_system_bus_integration\n";
}

int main() {
    std::cout << "Running I2C Controller unit tests...\n";
    test_i2c_controller_initialization();
    test_i2c_master_write();
    test_i2c_master_read();
    test_i2c_ack_nack_behaviour();
    test_i2c_invalid_address();
    test_i2c_multiple_transactions();
    test_i2c_reset_behaviour();
    test_i2c_hal_interface();
    test_i2c_system_bus_integration();
    std::cout << "All I2C Controller unit tests passed successfully.\n";
    return 0;
}
