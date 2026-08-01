#include "drivers/dma/dma_controller.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include "cpu/cpu.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace efs::drivers::dma;
using namespace efs::drivers::uart;
using namespace efs::kernel;
using namespace efs::memory;
using namespace efs::mmio;
using namespace efs::system;
using namespace efs::cpu;

void test_dma_memory_to_memory() {
    Memory mem(1024);
    for (std::size_t i = 0; i < 8; ++i) {
        mem.write(0x100 + i, static_cast<efs::common::Byte>(0x10 + i));
    }

    DMAController dma(&mem, nullptr, nullptr);
    bool cfg_ok = dma.configure(0x100, 0x200, 8);
    assert(cfg_ok);
    (void)cfg_ok;

    bool start_ok = dma.start();
    assert(start_ok);
    (void)start_ok;
    assert(dma.busy());

    for (int i = 0; i < 8; ++i) {
        dma.step();
    }

    assert(!dma.busy());
    assert(dma.completed());
    assert(!dma.hasError());
    assert(dma.transferredBytes() == 8);

    for (std::size_t i = 0; i < 8; ++i) {
        auto val = mem.read(0x200 + i);
        auto expected = static_cast<efs::common::Byte>(0x10 + i);
        assert(val == expected);
        (void)val;
        (void)expected;
    }

    std::cout << "[PASS] test_dma_memory_to_memory\n";
}

void test_dma_memory_to_mmio() {
    Memory mem(1024);
    MMIOBus bus;
    UART uart(bus, 0x40003000);
    uart.enable();

    const std::string msg = "HELLO";
    for (std::size_t i = 0; i < msg.size(); ++i) {
        mem.write(0x050 + i, static_cast<efs::common::Byte>(msg[i]));
    }

    DMAController dma(&mem, &bus, nullptr, &uart);
    bool cfg_ok = dma.configure(0x050, 0x40003000, msg.size()); // 0x40003000 is UART DATA register
    assert(cfg_ok);
    (void)cfg_ok;

    bool start_ok = dma.start();
    assert(start_ok);
    (void)start_ok;

    for (std::size_t i = 0; i < msg.size(); ++i) {
        dma.step();
    }

    assert(!dma.busy());
    assert(dma.completed());
    assert(uart.txFifoSize() == msg.size());

    for (char c : msg) {
        auto val = uart.popTxByte();
        assert(val == static_cast<efs::common::Byte>(c));
        (void)val;
        (void)c;
    }

    std::cout << "[PASS] test_dma_memory_to_mmio\n";
}

void test_dma_mmio_to_memory() {
    Memory mem(1024);
    MMIOBus bus;
    UART uart(bus, 0x40003000);
    uart.enable();

    uart.pushReceivedByte('A');
    uart.pushReceivedByte('B');
    uart.pushReceivedByte('C');
    assert(uart.rxFifoSize() == 3);

    DMAController dma(&mem, &bus, nullptr, &uart);
    bool cfg_ok = dma.configure(0x40003000, 0x300, 3);
    assert(cfg_ok);
    (void)cfg_ok;

    bool start_ok = dma.start();
    assert(start_ok);
    (void)start_ok;

    for (int i = 0; i < 3; ++i) {
        dma.step();
    }

    assert(dma.completed());
    auto byte0 = mem.read(0x300);
    auto byte1 = mem.read(0x301);
    auto byte2 = mem.read(0x302);
    assert(byte0 == 'A');
    assert(byte1 == 'B');
    assert(byte2 == 'C');
    (void)byte0;
    (void)byte1;
    (void)byte2;

    std::cout << "[PASS] test_dma_mmio_to_memory\n";
}

void test_dma_completion_interrupt() {
    Memory mem(1024);
    MMIOBus bus;
    InterruptController ic(bus, 0x40002000);

    constexpr std::uint8_t DMA_INT_ID = 4;
    ic.registerInterrupt(DMA_INT_ID);
    ic.enable(DMA_INT_ID);

    DMAController dma(&mem, &bus, &ic);
    dma.attachInterruptController(&ic, DMA_INT_ID);

    dma.configure(0x010, 0x020, 4);
    dma.start();

    for (int i = 0; i < 4; ++i) {
        dma.step();
    }

    assert(dma.completed());
    assert(ic.pending(DMA_INT_ID));

    std::cout << "[PASS] test_dma_completion_interrupt\n";
}

void test_dma_invalid_transfers() {
    // Unattached DMA controller
    DMAController unattachedDma;
    unattachedDma.configure(0x10, 0x20, 5);
    bool start_fail = !unattachedDma.start();
    assert(start_fail);
    (void)start_fail;
    assert(unattachedDma.hasError());

    // Out of bounds address
    Memory mem(128);
    DMAController dma(&mem, nullptr, nullptr);
    dma.configure(0x1000, 0x20, 5);
    bool start_oob = !dma.start();
    assert(start_oob);
    (void)start_oob;
    assert(dma.hasError());

    std::cout << "[PASS] test_dma_invalid_transfers\n";
}

void test_dma_zero_length() {
    Memory mem(1024);
    DMAController dma(&mem, nullptr, nullptr);
    dma.configure(0x100, 0x200, 0);
    bool start_ok = dma.start();
    assert(start_ok);
    (void)start_ok;

    assert(!dma.busy());
    assert(dma.completed());
    assert(dma.transferredBytes() == 0);

    std::cout << "[PASS] test_dma_zero_length\n";
}

void test_dma_stop_during_transfer() {
    Memory mem(1024);
    DMAController dma(&mem, nullptr, nullptr);
    dma.configure(0x100, 0x200, 10);
    dma.start();

    for (int i = 0; i < 3; ++i) {
        dma.step();
    }

    assert(dma.transferredBytes() == 3);
    assert(dma.busy());

    dma.stop();
    assert(!dma.busy());
    assert(!dma.completed());

    // Further steps do nothing while stopped
    dma.step();
    assert(dma.transferredBytes() == 3);

    std::cout << "[PASS] test_dma_stop_during_transfer\n";
}

void test_dma_system_bus_integration() {
    Memory mem(1024);
    MMIOBus mmioBus;
    SystemBus systemBus(&mem, &mmioBus, nullptr);
    DMAController dma;

    bool attach_ok = systemBus.attachDMA(&dma);
    assert(attach_ok);
    (void)attach_ok;
    assert(systemBus.dma() == &dma);

    CPU cpu(&systemBus);
    mem.write(0x01, 0xAA);
    mem.write(0x02, 0xBB);

    dma.configure(0x01, 0x10, 2);
    dma.start();

    cpu.run(2); // Runs 2 cycles, ticking DMA per cycle

    assert(dma.completed());
    auto byte10 = mem.read(0x10);
    auto byte11 = mem.read(0x11);
    assert(byte10 == 0xAA);
    assert(byte11 == 0xBB);
    (void)byte10;
    (void)byte11;

    std::cout << "[PASS] test_dma_system_bus_integration\n";
}

int main() {
    std::cout << "Running DMA Controller unit tests...\n";
    test_dma_memory_to_memory();
    test_dma_memory_to_mmio();
    test_dma_mmio_to_memory();
    test_dma_completion_interrupt();
    test_dma_invalid_transfers();
    test_dma_zero_length();
    test_dma_stop_during_transfer();
    test_dma_system_bus_integration();
    std::cout << "All DMA Controller unit tests passed successfully.\n";
    return 0;
}
