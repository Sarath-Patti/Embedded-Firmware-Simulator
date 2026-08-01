#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>

void test_register_construction() {
    efs::mmio::Register reg(0x40000000, 0x12345678);
    assert(reg.address() == 0x40000000);
    assert(reg.read() == 0x12345678);

    std::cout << "[PASS] test_register_construction\n";
}

void test_register_read_write() {
    efs::mmio::Register reg(0x40000004, 0x00);
    assert(reg.read() == 0x00);

    reg.write(0xDEADBEEF);
    assert(reg.read() == 0xDEADBEEF);

    std::cout << "[PASS] test_register_read_write\n";
}

void test_register_registration() {
    efs::mmio::MMIOBus bus;
    auto reg = std::make_shared<efs::mmio::Register>(0x40000000, 0x00);

    assert(!bus.contains(0x40000000));
    assert(bus.registerRegister(reg));
    assert(bus.contains(0x40000000));

    std::cout << "[PASS] test_register_registration\n";
}

void test_register_duplicate_registration() {
    efs::mmio::MMIOBus bus;
    auto reg1 = std::make_shared<efs::mmio::Register>(0x40000000, 0x10);
    auto reg2 = std::make_shared<efs::mmio::Register>(0x40000000, 0x20);

    assert(bus.registerRegister(reg1));
    assert(!bus.registerRegister(reg2));
    assert(bus.read(0x40000000) == 0x10);

    std::cout << "[PASS] test_register_duplicate_registration\n";
}

void test_valid_mapped_access() {
    efs::mmio::MMIOBus bus;
    auto reg1 = std::make_shared<efs::mmio::Register>(0x40000000, 0x100);
    auto reg2 = std::make_shared<efs::mmio::Register>(0x40000004, 0x200);

    bus.registerRegister(reg1);
    bus.registerRegister(reg2);

    assert(bus.read(0x40000000) == 0x100);
    assert(bus.read(0x40000004) == 0x200);

    bus.write(0x40000000, 0xABC);
    assert(bus.read(0x40000000) == 0xABC);
    assert(reg1->read() == 0xABC);

    std::cout << "[PASS] test_valid_mapped_access\n";
}

void test_invalid_address_access() {
    efs::mmio::MMIOBus bus;

    bool read_threw = false;
    try {
        [[maybe_unused]] auto val = bus.read(0x40000000);
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    assert(read_threw);
    (void)read_threw;

    bool write_threw = false;
    try {
        bus.write(0x40000000, 0x123);
    } catch (const std::out_of_range&) {
        write_threw = true;
    }
    assert(write_threw);
    (void)write_threw;

    std::cout << "[PASS] test_invalid_address_access\n";
}

void test_register_removal() {
    efs::mmio::MMIOBus bus;
    bus.registerRegister(0x40000000, 0x55);
    assert(bus.contains(0x40000000));

    assert(bus.unregisterRegister(0x40000000));
    assert(!bus.contains(0x40000000));
    assert(!bus.unregisterRegister(0x40000000));

    bool read_threw = false;
    try {
        [[maybe_unused]] auto val = bus.read(0x40000000);
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    assert(read_threw);
    (void)read_threw;

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
