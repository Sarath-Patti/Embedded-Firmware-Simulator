#include "memory/memory.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void test_valid_read_write() {
    efs::memory::Memory mem(1024);
    assert(mem.size() == 1024);

    mem.write(0x10, 0x42);
    assert(mem.read(0x10) == 0x42);

    mem.write(0x00, 0xFF);
    assert(mem.read(0x00) == 0xFF);

    std::cout << "[PASS] test_valid_read_write\n";
}

void test_boundary_addresses() {
    efs::memory::Memory mem(256);
    assert(mem.size() == 256);

    // First address
    mem.write(0, 0x11);
    assert(mem.read(0) == 0x11);

    // Last address
    mem.write(255, 0xAA);
    assert(mem.read(255) == 0xAA);

    std::cout << "[PASS] test_boundary_addresses\n";
}

void test_out_of_range_access() {
    efs::memory::Memory mem(256);

    // Read past boundary
    bool read_threw = false;
    try {
        [[maybe_unused]] auto val = mem.read(256);
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    assert(read_threw);
    (void)read_threw;

    // Write past boundary
    bool write_threw = false;
    try {
        mem.write(256, 0x55);
    } catch (const std::out_of_range&) {
        write_threw = true;
    }
    assert(write_threw);
    (void)write_threw;

    // Far out-of-bounds
    bool far_read_threw = false;
    try {
        [[maybe_unused]] auto val = mem.read(0xFFFF);
    } catch (const std::out_of_range&) {
        far_read_threw = true;
    }
    assert(far_read_threw);
    (void)far_read_threw;

    std::cout << "[PASS] test_out_of_range_access\n";
}

void test_clear_operation() {
    efs::memory::Memory mem(512);

    mem.write(0, 0x12);
    mem.write(255, 0x34);
    mem.write(511, 0x56);

    assert(mem.read(0) == 0x12);
    assert(mem.read(255) == 0x34);
    assert(mem.read(511) == 0x56);

    mem.clear();

    assert(mem.read(0) == 0x00);
    assert(mem.read(255) == 0x00);
    assert(mem.read(511) == 0x00);

    std::cout << "[PASS] test_clear_operation\n";
}

int main() {
    std::cout << "Running Memory unit tests...\n";
    test_valid_read_write();
    test_boundary_addresses();
    test_out_of_range_access();
    test_clear_operation();
    std::cout << "All Memory unit tests passed successfully.\n";
    return 0;
}
