#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_register_construction() {
    efs::mmio::Register reg(0x40000000, 0x12345678);
    assert(reg.address() == 0x40000000);
    assert(reg.read() == 0x12345678);

    std::cout << "[PASS] test_register_construction\n";
}

void test_register_read_write() {
    efs::mmio::Register reg(0x40000000, 0x0);
    reg.write(0xDEADBEEF);
    assert(reg.read() == 0xDEADBEEF);

    reg.reset();
    assert(reg.read() == 0x0);

    std::cout << "[PASS] test_register_read_write\n";
}

void test_register_registration() {
    efs::mmio::MMIOBus bus;
    assert(bus.size() == 0);

    bool reg = bus.registerRegister(0x40000000, 0x10);
    if (!reg) {
        throw std::runtime_error("Register registration failed");
    }
    assert(reg);
    assert(bus.size() == 1);
    assert(bus.contains(0x40000000));

    std::cout << "[PASS] test_register_registration\n";
}

void test_register_duplicate_registration() {
    efs::mmio::MMIOBus bus;
    bus.registerRegister(0x40000000, 0x10);

    bool res = bus.registerRegister(0x40000000, 0x20);
    if (res) {
        throw std::runtime_error("Expected false for duplicate registration");
    }
    assert(!res);

    std::cout << "[PASS] test_register_duplicate_registration\n";
}

void test_valid_mapped_access() {
    efs::mmio::MMIOBus bus;
    bool reg1 = bus.registerRegister(0x40000000, 0x0);
    if (!reg1) {
        throw std::runtime_error("Register registration failed");
    }
    assert(reg1);

    bus.write(0x40000000, 0xABC);
    assert(bus.read(0x40000000) == 0xABC);

    std::cout << "[PASS] test_valid_mapped_access\n";
}

void test_invalid_address_access() {
    efs::mmio::MMIOBus bus;

    bool read_threw = false;
    try {
        std::uint32_t val = bus.read(0x40000000);
        if (val != 0) {
            throw std::runtime_error("Unexpected read value");
        }
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    if (!read_threw) {
        throw std::runtime_error("Expected out_of_range exception for read at unmapped address");
    }
    assert(read_threw);

    bool write_threw = false;
    try {
        bus.write(0x40000000, 0x123);
    } catch (const std::out_of_range&) {
        write_threw = true;
    }
    if (!write_threw) {
        throw std::runtime_error("Expected out_of_range exception for write at unmapped address");
    }
    assert(write_threw);

    std::cout << "[PASS] test_invalid_address_access\n";
}

void test_register_removal() {
    efs::mmio::MMIOBus bus;
    bus.registerRegister(0x40000000, 0x55);
    assert(bus.contains(0x40000000));

    bool unreg_ok = bus.unregisterRegister(0x40000000);
    if (!unreg_ok) {
        throw std::runtime_error("Unregister register failed");
    }
    assert(unreg_ok);
    assert(!bus.contains(0x40000000));

    bool unreg_fail = !bus.unregisterRegister(0x40000000);
    if (!unreg_fail) {
        throw std::runtime_error("Duplicate unregister register should return false");
    }
    assert(unreg_fail);

    bool read_threw = false;
    try {
        std::uint32_t val = bus.read(0x40000000);
        if (val != 0) {
            throw std::runtime_error("Unexpected read value");
        }
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    if (!read_threw) {
        throw std::runtime_error("Expected out_of_range exception after unregistering address");
    }
    assert(read_threw);

    std::cout << "[PASS] test_register_removal\n";
}

int main() {
    std::cout << "Running MMIO unit tests...\n";
    test_register_construction();
    test_register_read_write();
    test_register_registration();
    test_register_duplicate_registration();
    test_valid_mapped_access();
    test_invalid_address_access();
    test_register_removal();
    std::cout << "All MMIO unit tests passed successfully.\n";
    return 0;
}
