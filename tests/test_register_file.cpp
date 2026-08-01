#include "cpu/cpu.hpp"
#include "cpu/registers/register_file.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

using namespace efs::cpu;
using namespace efs::cpu::registers;

void test_register_reads() {
    RegisterFile rf;
    for (std::size_t i = 0; i < NUM_GPRS; ++i) {
        assert(rf.readRegister(i) == 0);
    }
    std::cout << "[PASS] test_register_reads\n";
}

void test_register_writes() {
    RegisterFile rf;

    rf.writeRegister(0, 0x12345678);
    rf.writeRegister(15, 0xDEADBEEF);
    rf.writeRegister(7, 0xAABBCCDD);

    assert(rf.readRegister(0) == 0x12345678);
    assert(rf.readRegister(15) == 0xDEADBEEF);
    assert(rf.readRegister(7) == 0xAABBCCDD);

    std::cout << "[PASS] test_register_writes\n";
}

void test_register_reset() {
    RegisterFile rf;

    for (std::size_t i = 0; i < NUM_GPRS; ++i) {
        rf.writeRegister(i, static_cast<efs::common::DWord>(i + 1));
    }
    rf.writePC(0x08000000);
    rf.writeSP(0x20008000);
    rf.writeStatus(0x00000001);

    rf.reset();

    for (std::size_t i = 0; i < NUM_GPRS; ++i) {
        assert(rf.readRegister(i) == 0);
    }
    assert(rf.readPC() == 0);
    assert(rf.readSP() == 0);
    assert(rf.readStatus() == 0);

    std::cout << "[PASS] test_register_reset\n";
}

void test_invalid_register_index() {
    RegisterFile rf;

    bool read_threw = false;
    try {
        [[maybe_unused]] auto val = rf.readRegister(NUM_GPRS);
    } catch (const std::out_of_range&) {
        read_threw = true;
    }
    assert(read_threw);
    (void)read_threw;

    bool write_threw = false;
    try {
        rf.writeRegister(NUM_GPRS, 0x123);
    } catch (const std::out_of_range&) {
        write_threw = true;
    }
    assert(write_threw);
    (void)write_threw;

    std::cout << "[PASS] test_invalid_register_index\n";
}

void test_special_registers() {
    RegisterFile rf;

    assert(rf.readPC() == 0);
    rf.writePC(0x08001000);
    assert(rf.readPC() == 0x08001000);

    assert(rf.readSP() == 0);
    rf.writeSP(0x20010000);
    assert(rf.readSP() == 0x20010000);

    assert(rf.readStatus() == 0);
    rf.writeStatus(0x0000001F);
    assert(rf.readStatus() == 0x0000001F);

    std::cout << "[PASS] test_special_registers\n";
}

void test_cpu_register_file_integration() {
    CPU cpu;
    cpu.registerFile().writePC(0x08000000);
    cpu.registerFile().writeSP(0x20008000);
    cpu.registerFile().writeRegister(0, 0x42);

    assert(cpu.registerFile().readPC() == 0x08000000);
    assert(cpu.registerFile().readSP() == 0x20008000);
    assert(cpu.registerFile().readRegister(0) == 0x42);

    cpu.reset();

    assert(cpu.registerFile().readPC() == 0);
    assert(cpu.registerFile().readSP() == 0);
    assert(cpu.registerFile().readRegister(0) == 0);

    std::cout << "[PASS] test_cpu_register_file_integration\n";
}

int main() {
    std::cout << "Running RegisterFile unit tests...\n";
    test_register_reads();
    test_register_writes();
    test_register_reset();
    test_invalid_register_index();
    test_special_registers();
    test_cpu_register_file_integration();
    std::cout << "All RegisterFile unit tests passed successfully.\n";
    return 0;
}
