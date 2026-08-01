#ifndef EFS_DRIVERS_DMA_DMA_CONTROLLER_HPP
#define EFS_DRIVERS_DMA_DMA_CONTROLLER_HPP

#include "common/types.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/scheduler/event_scheduler.hpp"
#include <cstdint>

namespace efs::drivers::dma {

/// Direct Memory Access (DMA) controller for cycle-accurate background data transfers.
class DMAController {
public:
    explicit DMAController(memory::Memory* memory = nullptr,
                           mmio::MMIOBus* mmioBus = nullptr,
                           kernel::InterruptController* interruptController = nullptr,
                           drivers::uart::UART* uart = nullptr);
    ~DMAController() = default;

    DMAController(const DMAController&) = delete;
    DMAController& operator=(const DMAController&) = delete;
    DMAController(DMAController&&) noexcept = default;
    DMAController& operator=(DMAController&&) noexcept = default;

    /// Attaches the Memory subsystem.
    void attachMemory(memory::Memory* memory) noexcept;

    /// Attaches the MMIO Bus subsystem.
    void attachMMIO(mmio::MMIOBus* mmioBus) noexcept;

    /// Attaches an InterruptController reference and completion interrupt ID.
    void attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId = 0) noexcept;

    /// Detaches the InterruptController.
    void detachInterruptController() noexcept;

    /// Attaches a UART peripheral for UART hardware FIFO transfers.
    void attachUART(drivers::uart::UART* uart) noexcept;

    /// Detaches the UART peripheral.
    void detachUART() noexcept;

    /// Attaches EventScheduler reference.
    void attachScheduler(system::scheduler::EventScheduler* scheduler) noexcept;

    /// Detaches EventScheduler reference.
    void detachScheduler() noexcept;

    /// Configures source address, destination address, and transfer length in bytes.
    bool configure(common::Address sourceAddress, common::Address destinationAddress, common::Size length);

    /// Configures source and destination auto-increment behavior explicitly.
    void setIncrements(bool sourceIncrement, bool destinationIncrement) noexcept;

    /// Starts the configured DMA transfer. Returns true if transfer initiated or completed (zero-length).
    bool start();

    /// Halts active DMA transfer immediately.
    void stop();

    /// Resets the DMA controller configuration and internal state.
    void reset();

    /// Advances the active DMA transfer by one byte for the current simulation cycle.
    void step();

    /// Alias for step().
    void tick();

    /// Returns true if a DMA transfer is currently in progress.
    [[nodiscard]] bool busy() const noexcept;

    /// Returns true if the last configured DMA transfer completed successfully.
    [[nodiscard]] bool completed() const noexcept;

    /// Returns true if an error occurred during transfer (e.g. invalid memory/MMIO address).
    [[nodiscard]] bool hasError() const noexcept;

    /// Returns configured source address.
    [[nodiscard]] common::Address sourceAddress() const noexcept;

    /// Returns configured destination address.
    [[nodiscard]] common::Address destinationAddress() const noexcept;

    /// Returns configured transfer length in bytes.
    [[nodiscard]] common::Size length() const noexcept;

    /// Returns number of bytes transferred so far.
    [[nodiscard]] common::Size transferredBytes() const noexcept;

    /// Returns configured completion interrupt ID.
    [[nodiscard]] std::uint8_t interruptId() const noexcept;

    /// Returns source address auto-increment setting.
    [[nodiscard]] bool sourceIncrement() const noexcept;

    /// Returns destination address auto-increment setting.
    [[nodiscard]] bool destinationIncrement() const noexcept;

    /// Returns attached UART peripheral pointer or nullptr.
    [[nodiscard]] drivers::uart::UART* uart() const noexcept;

private:
    memory::Memory* m_memory{nullptr};
    mmio::MMIOBus* m_mmioBus{nullptr};
    kernel::InterruptController* m_interruptController{nullptr};
    system::scheduler::EventScheduler* m_scheduler{nullptr};
    drivers::uart::UART* m_uart{nullptr};

    std::uint8_t m_interruptId{0};
    common::Address m_sourceAddress{0};
    common::Address m_destinationAddress{0};
    common::Size m_length{0};

    common::Address m_currentSrc{0};
    common::Address m_currentDst{0};
    common::Size m_transferredBytes{0};

    bool m_srcIncrement{true};
    bool m_dstIncrement{true};
    bool m_userSetSrcIncrement{false};
    bool m_userSetDstIncrement{false};

    bool m_busy{false};
    bool m_completed{false};
    bool m_error{false};
};

} // namespace efs::drivers::dma

#endif // EFS_DRIVERS_DMA_DMA_CONTROLLER_HPP
