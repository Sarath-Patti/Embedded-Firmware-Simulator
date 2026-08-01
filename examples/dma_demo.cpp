#include "cpu/cpu.hpp"
#include "drivers/dma/dma_controller.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cstdint>
#include <iostream>
#include <string>

int main() {
    std::cout << "Starting Direct Memory Access (DMA) Controller Demo...\n";

    // 1. Setup Memory, MMIO Bus, Interrupt Controller, and UART
    efs::memory::Memory memory(2048);
    efs::mmio::MMIOBus mmioBus;
    constexpr efs::common::Address UART_BASE = 0x40003000U;
    constexpr efs::common::Address INT_BASE = 0x40002000U;

    efs::drivers::uart::UART uart(mmioBus, UART_BASE);
    uart.enable();

    efs::kernel::InterruptController ic(mmioBus, INT_BASE);
    constexpr std::uint8_t DMA_INT_ID = 5;
    ic.registerInterrupt(DMA_INT_ID);
    ic.enable(DMA_INT_ID);

    bool interruptFired = false;
    ic.registerHandler(DMA_INT_ID, [&]() {
        interruptFired = true;
        std::cout << "  [ISR] DMA Completion Interrupt (ID " << static_cast<int>(DMA_INT_ID) << ") fired!\n";
    });

    // 2. Setup SystemBus, DMA Controller, and CPU
    efs::system::SystemBus systemBus(&memory, &mmioBus, &ic);
    systemBus.attachUART(&uart);

    efs::drivers::dma::DMAController dma(&memory, &mmioBus, &ic);
    dma.attachInterruptController(&ic, DMA_INT_ID);
    systemBus.attachDMA(&dma);

    efs::cpu::CPU cpu(&systemBus);

    // 3. Demo 1: Memory-to-Memory DMA Copy
    std::cout << "\n--- Demo 1: Memory-to-Memory DMA Copy ---\n";
    constexpr efs::common::Address SRC_BUF = 0x0100U;
    constexpr efs::common::Address DST_BUF = 0x0200U;
    const std::string testData = "DMA Memory Copy Test Payload!";

    for (std::size_t i = 0; i < testData.size(); ++i) {
        memory.write(SRC_BUF + i, static_cast<efs::common::Byte>(testData[i]));
    }

    std::cout << "Configuring DMA: Source=0x" << std::hex << SRC_BUF
              << ", Dest=0x" << DST_BUF << ", Length=" << std::dec << testData.size() << " bytes\n";
    dma.configure(SRC_BUF, DST_BUF, testData.size());
    dma.start();

    std::cout << "Running CPU simulation cycles for DMA transfer...\n";
    cpu.run(testData.size());

    std::cout << "Transfer Busy? " << (dma.busy() ? "Yes" : "No") << "\n";
    std::cout << "Transfer Completed? " << (dma.completed() ? "Yes" : "No") << "\n";

    std::string resultBuf;
    for (std::size_t i = 0; i < testData.size(); ++i) {
        resultBuf.push_back(static_cast<char>(memory.read(DST_BUF + i)));
    }
    std::cout << "Copied Memory Content: \"" << resultBuf << "\"\n";

    // 4. Demo 2: Memory-to-MMIO (UART TX) DMA Transfer with Interrupt Notification
    std::cout << "\n--- Demo 2: Memory-to-MMIO (UART TX) DMA Transfer ---\n";
    constexpr efs::common::Address UART_TX_BUF = 0x0300U;
    const std::string uartMsg = "Hello via DMA to UART!";

    for (std::size_t i = 0; i < uartMsg.size(); ++i) {
        memory.write(UART_TX_BUF + i, static_cast<efs::common::Byte>(uartMsg[i]));
    }

    dma.configure(UART_TX_BUF, UART_BASE, uartMsg.size());
    dma.start();

    std::cout << "Running CPU cycles to stream data to UART TX register via DMA...\n";
    cpu.run(uartMsg.size());

    std::cout << "UART TX FIFO Size: " << uart.txFifoSize() << " bytes\n";
    std::cout << "UART Streamed Message: \"";
    while (uart.txFifoSize() > 0) {
        std::cout << static_cast<char>(uart.popTxByte());
    }
    std::cout << "\"\n";

    std::cout << "Interrupt Fired? " << (interruptFired ? "Yes" : "No") << "\n";

    std::cout << "\nDirect Memory Access (DMA) Controller Demo completed successfully.\n";
    return 0;
}
