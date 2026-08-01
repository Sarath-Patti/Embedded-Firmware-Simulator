#include "drivers/spi/spi_controller.hpp"
#include "drivers/spi/spi_device.hpp"
#include "hal/spi_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace efs::drivers::spi;
using namespace efs::hal;
using namespace efs::mmio;
using namespace efs::system;

void test_spi_initialization() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000, 4, 1);

    assert(bus.contains(spi.dataAddress()));
    assert(bus.contains(spi.statusAddress()));
    assert(bus.contains(spi.controlAddress()));
    assert(bus.contains(spi.clockDivAddress()));

    assert(!spi.enabled());
    assert(spi.clockDivider() == 4);
    assert(spi.mode() == 1);
    assert(!spi.busy());
    assert(!spi.hasData());

    spi.enable();
    assert(spi.enabled());
    assert((bus.read(spi.controlAddress()) & SPIController::CTRL_ENABLE_BIT) != 0);

    spi.disable();
    assert(!spi.enabled());

    std::cout << "[PASS] test_spi_initialization\n";
}

void test_spi_master_slave_transfer() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000);
    spi.enable();

    SimulatedSPIDevice slave(0x55);
    spi.attachSlave(&slave);
    assert(spi.slave() == &slave);

    std::uint8_t rx = spi.transfer(0xA5);
    assert(rx == 0x55);
    assert(slave.lastReceivedByte() == 0xA5);
    assert(slave.receivedBytes().size() == 1);
    (void)rx;

    std::cout << "[PASS] test_spi_master_slave_transfer\n";
}

void test_spi_full_duplex_communication() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000);
    spi.enable();

    SimulatedSPIDevice slave;
    slave.queueResponseByte(0x11);
    slave.queueResponseByte(0x22);
    slave.queueResponseByte(0x33);
    spi.attachSlave(&slave);

    std::uint8_t r1 = spi.transfer(0xDE);
    std::uint8_t r2 = spi.transfer(0xAD);
    std::uint8_t r3 = spi.transfer(0xBE);

    assert(r1 == 0x11);
    assert(r2 == 0x22);
    assert(r3 == 0x33);
    (void)r1;
    (void)r2;
    (void)r3;

    assert(slave.receivedBytes().size() == 3);
    assert(slave.receivedBytes()[0] == 0xDE);
    assert(slave.receivedBytes()[1] == 0xAD);
    assert(slave.receivedBytes()[2] == 0xBE);

    std::cout << "[PASS] test_spi_full_duplex_communication\n";
}

void test_spi_multiple_transfers() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000);
    spi.enable();

    SimulatedSPIDevice slave(0xFF);
    spi.attachSlave(&slave);

    for (std::uint8_t i = 0; i < 10; ++i) {
        spi.writeByte(i);
    }

    assert(spi.hasData());
    for (std::uint8_t i = 0; i < 10; ++i) {
        std::uint8_t rx = spi.readByte();
        assert(rx == 0xFF);
        (void)rx;
    }
    assert(!spi.hasData());
    assert(slave.receivedBytes().size() == 10);

    std::cout << "[PASS] test_spi_multiple_transfers\n";
}

void test_spi_reset_behaviour() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000, 8, 2);
    spi.enable();

    SimulatedSPIDevice slave(0xAA);
    spi.attachSlave(&slave);

    (void)spi.transfer(0x12);
    assert(spi.hasData());

    spi.reset();
    assert(!spi.enabled());
    assert(spi.slave() == nullptr);
    assert(!spi.hasData());

    std::cout << "[PASS] test_spi_reset_behaviour\n";
}

void test_spi_invalid_operations() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000);

    // Disabled transfer attempt
    bool transfer_disabled = false;
    try {
        (void)spi.transfer(0x01);
    } catch (const std::runtime_error&) {
        transfer_disabled = true;
    }
    assert(transfer_disabled);
    (void)transfer_disabled;

    // Disabled readByte attempt
    bool read_disabled = false;
    try {
        (void)spi.readByte();
    } catch (const std::runtime_error&) {
        read_disabled = true;
    }
    assert(read_disabled);
    (void)read_disabled;

    // Enabled readByte on empty RX buffer
    spi.enable();
    bool read_empty = false;
    try {
        (void)spi.readByte();
    } catch (const std::underflow_error&) {
        read_empty = true;
    }
    assert(read_empty);
    (void)read_empty;

    // Invalid clock divider 0
    bool invalid_clk = false;
    try {
        spi.setClockDivider(0);
    } catch (const std::invalid_argument&) {
        invalid_clk = true;
    }
    assert(invalid_clk);
    (void)invalid_clk;

    // Invalid SPI mode 4
    bool invalid_mode = false;
    try {
        spi.setMode(4);
    } catch (const std::invalid_argument&) {
        invalid_mode = true;
    }
    assert(invalid_mode);
    (void)invalid_mode;

    std::cout << "[PASS] test_spi_invalid_operations\n";
}

void test_spi_hal_interface() {
    MMIOBus bus;
    SPIController spi(bus, 0x40004000);
    SimulatedSPIDevice slave;
    slave.queueResponseByte(0xBE);
    spi.attachSlave(&slave);

    SPIHAL hal(&spi);
    assert(hal.isAttached());

    hal.enable();
    assert(hal.enabled());

    hal.configure(3, 16);
    assert(spi.mode() == 3);
    assert(spi.clockDivider() == 16);

    std::uint8_t rx = hal.transfer(0xEF);
    assert(rx == 0xBE);
    assert(slave.lastReceivedByte() == 0xEF);
    (void)rx;

    hal.writeByte(0x77);
    std::uint8_t read_rx = hal.readByte();
    assert(read_rx == 0x00);
    (void)read_rx;

    hal.disable();
    assert(!hal.enabled());

    // Unattached HAL
    SPIHAL unattached;
    assert(!unattached.isAttached());
    bool threw_unattached = false;
    try {
        unattached.writeByte(0x00);
    } catch (const std::runtime_error&) {
        threw_unattached = true;
    }
    assert(threw_unattached);
    (void)threw_unattached;

    std::cout << "[PASS] test_spi_hal_interface\n";
}

void test_spi_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    SPIController spi(bus, 0x40004000);

    systemBus.attachSPI(&spi);
    assert(systemBus.spi() == &spi);

    spi.enable();
    SimulatedSPIDevice slave(0x42);
    spi.attachSlave(&slave);

    std::uint8_t rx = spi.transfer(0x99);
    assert(rx == 0x42);
    (void)rx;

    systemBus.reset();
    assert(!spi.enabled());

    std::cout << "[PASS] test_spi_system_bus_integration\n";
}

int main() {
    std::cout << "Running SPI Controller unit tests...\n";
    test_spi_initialization();
    test_spi_master_slave_transfer();
    test_spi_full_duplex_communication();
    test_spi_multiple_transfers();
    test_spi_reset_behaviour();
    test_spi_invalid_operations();
    test_spi_hal_interface();
    test_spi_system_bus_integration();
    std::cout << "All SPI Controller unit tests passed successfully.\n";
    return 0;
}
