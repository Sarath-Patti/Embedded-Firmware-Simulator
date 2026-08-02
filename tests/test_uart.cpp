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

    assert(bus.contains(0x40003000)); // DATA
    assert(bus.contains(0x40003004)); // STATUS
    assert(bus.contains(0x40003008)); // CONTROL
    assert(bus.contains(0x4000300C)); // BAUD

    std::cout << "[PASS] test_mmio_registration\n";
}

void test_enable_disable() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);

    assert(!uart.enabled());

    uart.enable();
    assert(uart.enabled());
    assert((bus.read(0x40003008) & UART::CTRL_ENABLE_BIT) != 0);

    uart.disable();
    assert(!uart.enabled());
    assert((bus.read(0x40003008) & UART::CTRL_ENABLE_BIT) == 0);

    std::cout << "[PASS] test_enable_disable\n";
}

void test_fifo_operations_and_register_sync() {
    MMIOBus bus;
    UART uart(bus, 0x40003000);
    uart.enable();

    // Initial state: empty FIFOs
    assert(!uart.hasReceivedData());
    assert(uart.txEmpty());
    assert((bus.read(0x40003004) & UART::STATUS_TX_EMPTY_BIT) != 0);
    assert((bus.read(0x40003004) & UART::STATUS_RX_AVAIL_BIT) == 0);

    // TX Write via method
    uart.writeByte('H');
    uart.writeByte('i');
    assert(bus.read(0x40003000) == 'i'); // DATA register shows last written byte
    assert((bus.read(0x40003004) & UART::STATUS_TX_EMPTY_BIT) == 0);

    // Check TX buffer retrieval via popTxByte
    assert(uart.txFifoSize() == 2);
    std::uint8_t t1 = uart.popTxByte();
    std::uint8_t t2 = uart.popTxByte();
    if (t1 != 'H' || t2 != 'i') {
        throw std::runtime_error("TX FIFO retrieval failed");
    }
    assert(t1 == 'H');
    assert(t2 == 'i');
    assert(uart.txEmpty());
    assert((bus.read(0x40003004) & UART::STATUS_TX_EMPTY_BIT) != 0);

    // RX Operations (simulate incoming data)
    uart.pushReceivedByte('A');
    uart.pushReceivedByte('B');

    assert(uart.hasReceivedData());
    assert((bus.read(0x40003004) & UART::STATUS_RX_AVAIL_BIT) != 0);
    assert(bus.read(0x40003000) == 'A'); // Peek front byte via MMIO read

    // Read byte via method
    std::uint8_t b1 = uart.readByte();
    if (b1 != 'A') {
        throw std::runtime_error("Read b1 failed");
    }
    assert(b1 == 'A');
    assert(uart.hasReceivedData());

    std::uint8_t b2 = uart.readByte();
    if (b2 != 'B') {
        throw std::runtime_error("Read b2 failed");
    }
    assert(b2 == 'B');
    assert(!uart.hasReceivedData());
    assert((bus.read(0x40003004) & UART::STATUS_RX_AVAIL_BIT) == 0);

    std::cout << "[PASS] test_fifo_operations_and_register_sync\n";
}

void test_baud_rate() {
    MMIOBus bus;
    UART uart(bus, 0x40003000, 9600);
    assert(uart.baudRate() == 9600);
    assert(bus.read(0x4000300C) == 9600);

    uart.setBaudRate(115200);
    assert(uart.baudRate() == 115200);
    assert(bus.read(0x4000300C) == 115200);

    // Modify BAUD via MMIO register
    bus.write(0x4000300C, 57600);
    assert(uart.baudRate() == 57600);

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
    if (!threw_write_disabled) {
        throw std::runtime_error("Expected runtime_error for writeByte when disabled");
    }
    assert(threw_write_disabled);

    // Read when disabled
    bool threw_read_disabled = false;
    try {
        std::uint8_t b = uart.readByte();
        if (b != 0) {
            throw std::runtime_error("Unexpected byte");
        }
    } catch (const std::runtime_error&) {
        threw_read_disabled = true;
    }
    if (!threw_read_disabled) {
        throw std::runtime_error("Expected runtime_error for readByte when disabled");
    }
    assert(threw_read_disabled);

    // Push when disabled
    bool threw_push_disabled = false;
    try {
        uart.pushReceivedByte('Q');
    } catch (const std::runtime_error&) {
        threw_push_disabled = true;
    }
    if (!threw_push_disabled) {
        throw std::runtime_error("Expected runtime_error for pushReceivedByte when disabled");
    }
    assert(threw_push_disabled);

    // Enable and test empty RX read
    uart.enable();
    bool threw_empty_rx = false;
    try {
        std::uint8_t b = uart.readByte();
        if (b != 0) {
            throw std::runtime_error("Unexpected byte");
        }
    } catch (const std::underflow_error&) {
        threw_empty_rx = true;
    }
    if (!threw_empty_rx) {
        throw std::runtime_error("Expected underflow_error for readByte on empty RX");
    }
    assert(threw_empty_rx);

    // Invalid baud rate 0
    bool threw_zero_baud = false;
    try {
        uart.setBaudRate(0);
    } catch (const std::invalid_argument&) {
        threw_zero_baud = true;
    }
    if (!threw_zero_baud) {
        throw std::runtime_error("Expected invalid_argument for baud rate 0");
    }
    assert(threw_zero_baud);

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

    bool exec_ok = monitor.executeCommand("uart", ss);
    if (!exec_ok) {
        throw std::runtime_error("Monitor uart command failed");
    }
    assert(exec_ok);
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
