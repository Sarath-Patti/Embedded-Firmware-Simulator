#include "drivers/spi/spi_controller.hpp"
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

    assert(bus.contains(spi.controlAddress()));
    assert(bus.contains(spi.statusAddress()));
    assert(bus.contains(spi.dataAddress()));
    assert(bus.contains(spi.clockDivAddress()));

    assert(!spi.enabled());
    assert(spi.clockDivider() == 4);
    assert(spi.mode() == 1);
    assert(spi.slave() == nullptr);

    spi.enable();
    assert(spi.enabled());

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
    if (rx != 0x55) {
        throw std::runtime_error("SPI transfer failed");
    }
    assert(rx == 0x55);
    assert(slave.lastReceivedByte() == 0xA5);
    assert(slave.receivedBytes().size() == 1);

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

    if (r1 != 0x11 || r2 != 0x22 || r3 != 0x33) {
        throw std::runtime_error("SPI full duplex transfer failed");
    }
    assert(r1 == 0x11);
    assert(r2 == 0x22);
    assert(r3 == 0x33);

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
        if (rx != 0xFF) {
            throw std::runtime_error("SPI readByte failed");
        }
        assert(rx == 0xFF);
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

    std::uint8_t rx = spi.transfer(0x12);
    if (rx != 0xAA) {
        throw std::runtime_error("SPI transfer before reset failed");
    }
    assert(rx == 0xAA);
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
        std::uint8_t b = spi.transfer(0x01);
        if (b != 0) {
            throw std::runtime_error("Unexpected byte");
        }
    } catch (const std::runtime_error&) {
        transfer_disabled = true;
    }
    if (!transfer_disabled) {
        throw std::runtime_error("Expected runtime_error for disabled transfer");
    }
    assert(transfer_disabled);

    // Disabled readByte attempt
    bool read_disabled = false;
    try {
        std::uint8_t b = spi.readByte();
        if (b != 0) {
            throw std::runtime_error("Unexpected byte");
        }
    } catch (const std::runtime_error&) {
        read_disabled = true;
    }
    if (!read_disabled) {
        throw std::runtime_error("Expected runtime_error for disabled readByte");
    }
    assert(read_disabled);

    // Enabled readByte on empty RX buffer
    spi.enable();
    bool read_empty = false;
    try {
        std::uint8_t b = spi.readByte();
        if (b != 0) {
            throw std::runtime_error("Unexpected byte");
        }
    } catch (const std::underflow_error&) {
        read_empty = true;
    }
    if (!read_empty) {
        throw std::runtime_error("Expected underflow_error for empty readByte");
    }
    assert(read_empty);

    // Invalid clock divider 0
    bool invalid_clk = false;
    try {
        spi.setClockDivider(0);
    } catch (const std::invalid_argument&) {
        invalid_clk = true;
    }
    if (!invalid_clk) {
        throw std::runtime_error("Expected invalid_argument for clock divider 0");
    }
    assert(invalid_clk);

    // Invalid SPI mode 4
    bool invalid_mode = false;
    try {
        spi.setMode(4);
    } catch (const std::invalid_argument&) {
        invalid_mode = true;
    }
    if (!invalid_mode) {
        throw std::runtime_error("Expected invalid_argument for mode 4");
    }
    assert(invalid_mode);

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
    if (rx != 0xBE) {
        throw std::runtime_error("SPIHAL transfer failed");
    }
    assert(rx == 0xBE);
    assert(slave.lastReceivedByte() == 0xEF);

    hal.writeByte(0x77);
    std::uint8_t read_rx = hal.readByte();
    if (read_rx != 0x00) {
        throw std::runtime_error("SPIHAL readByte failed");
    }
    assert(read_rx == 0x00);

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
    if (!threw_unattached) {
        throw std::runtime_error("Expected runtime_error for unattached SPIHAL writeByte");
    }
    assert(threw_unattached);

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
    if (rx != 0x42) {
        throw std::runtime_error("SPI system bus integration transfer failed");
    }
    assert(rx == 0x42);

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
