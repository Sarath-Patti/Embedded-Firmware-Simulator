#include "drivers/uart/uart.hpp"
#include "mmio/mmio_bus.hpp"
#include "monitor/monitor.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace efs::drivers::uart;
using namespace efs::mmio;
using namespace efs::monitor;

void test_mmio_registration() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);

    assert(bus.contains(uart.dataAddress()));
    assert(bus.contains(uart.statusAddress()));
    assert(bus.contains(uart.controlAddress()));
    assert(bus.contains(uart.baudAddress()));

    assert(bus.read(uart.baudAddress()) == 115200);

    std::cout << "[PASS] test_mmio_registration\n";
}

void test_enable_disable() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);

    assert(!uart.enabled());
    assert((bus.read(uart.controlAddress()) & UART::CTRL_ENABLE_BIT) == 0);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_ENABLED_BIT) == 0);

    uart.enable();
    assert(uart.enabled());
    assert((bus.read(uart.controlAddress()) & UART::CTRL_ENABLE_BIT) != 0);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_ENABLED_BIT) != 0);

    uart.disable();
    assert(!uart.enabled());
    assert((bus.read(uart.controlAddress()) & UART::CTRL_ENABLE_BIT) == 0);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_ENABLED_BIT) == 0);

    std::cout << "[PASS] test_enable_disable\n";
}

void test_fifo_operations_and_register_sync() {
    MMIOBus bus;
    UART uart(bus, 0x40003000, 9600);
    uart.enable();

    assert(uart.txEmpty());
    assert(!uart.hasReceivedData());
    assert((bus.read(uart.statusAddress()) & UART::STATUS_TX_EMPTY_BIT) != 0);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_RX_AVAIL_BIT) == 0);

    // TX FIFO test
    uart.writeByte('A');
    uart.writeByte('B');
    assert(!uart.txEmpty());
    assert(uart.txFifoSize() == 2);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_TX_EMPTY_BIT) == 0);

    assert(uart.popTxByte() == 'A');
    assert(uart.txFifoSize() == 1);
    assert(uart.popTxByte() == 'B');
    assert(uart.txEmpty());
    assert((bus.read(uart.statusAddress()) & UART::STATUS_TX_EMPTY_BIT) != 0);

    // RX FIFO test
    uart.pushReceivedByte('X');
    assert(uart.hasReceivedData());
    assert(uart.rxFifoSize() == 1);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_RX_AVAIL_BIT) != 0);

    assert(uart.readByte() == 'X');
    assert(!uart.hasReceivedData());
    assert(uart.rxFifoSize() == 0);
    assert((bus.read(uart.statusAddress()) & UART::STATUS_RX_AVAIL_BIT) == 0);

    std::cout << "[PASS] test_fifo_operations_and_register_sync\n";
}

void test_baud_rate() {
    MMIOBus bus;
    UART uart(bus, 0x40003000, 115200);

    assert(uart.baudRate() == 115200);
    assert(bus.read(uart.baudAddress()) == 115200);

    uart.setBaudRate(9600);
    assert(uart.baudRate() == 9600);
    assert(bus.read(uart.baudAddress()) == 9600);

    std::cout << "[PASS] test_baud_rate\n";
}

void test_invalid_operations() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);

    // Write when disabled
    bool threw_write_disabled = false;
    try {
        uart.writeByte('Z');
    } catch (const std::runtime_error&) {
        threw_write_disabled = true;
    }
    assert(threw_write_disabled);
    (void)threw_write_disabled;

    // Read when disabled
    bool threw_read_disabled = false;
    try {
        [[maybe_unused]] auto b = uart.readByte();
    } catch (const std::runtime_error&) {
        threw_read_disabled = true;
    }
    assert(threw_read_disabled);
    (void)threw_read_disabled;

    // Push when disabled
    bool threw_push_disabled = false;
    try {
        uart.pushReceivedByte('Q');
    } catch (const std::runtime_error&) {
        threw_push_disabled = true;
    }
    assert(threw_push_disabled);
    (void)threw_push_disabled;

    // Enable and test empty RX read
    uart.enable();
    bool threw_empty_rx = false;
    try {
        [[maybe_unused]] auto b = uart.readByte();
    } catch (const std::underflow_error&) {
        threw_empty_rx = true;
    }
    assert(threw_empty_rx);
    (void)threw_empty_rx;

    // Invalid baud rate 0
    bool threw_zero_baud = false;
    try {
        uart.setBaudRate(0);
    } catch (const std::invalid_argument&) {
        threw_zero_baud = true;
    }
    assert(threw_zero_baud);
    (void)threw_zero_baud;

    std::cout << "[PASS] test_invalid_operations\n";
}

void test_monitor_output() {
    MMIOBus bus;
    UART uart(bus, 0x40003000, 115200);
    uart.enable();
    uart.writeByte('1');
    uart.writeByte('2');

    Monitor monitor(nullptr, nullptr, &bus, nullptr, nullptr, nullptr, &uart);
    std::ostringstream ss;

    assert(monitor.executeCommand("uart", ss));
    std::string out = ss.str();

    assert(out.find("UART") != std::string::npos);
    assert(out.find("Enabled : YES") != std::string::npos);
    assert(out.find("Baud : 115200") != std::string::npos);
    assert(out.find("TX FIFO : 2 bytes") != std::string::npos);
    assert(out.find("TX Empty : NO") != std::string::npos);

    std::cout << "[PASS] test_monitor_output\n";
}

int main() {
    std::cout << "Running UART integration unit tests...\n";
    test_mmio_registration();
    test_enable_disable();
    test_fifo_operations_and_register_sync();
    test_baud_rate();
    test_invalid_operations();
    test_monitor_output();
    std::cout << "All UART integration unit tests passed successfully.\n";
    return 0;
}
