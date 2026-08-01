#ifndef EFS_SYSTEM_SYSTEM_BUS_HPP
#define EFS_SYSTEM_SYSTEM_BUS_HPP

#include "drivers/dma/dma_controller.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/clock/simulation_clock.hpp"
#include "system/scheduler/event_scheduler.hpp"
#include <vector>

namespace efs::system {

/// Central System Bus connecting Memory, MMIO Bus, Interrupt Controller, Simulation Clock, Event Scheduler, DMA, and Peripherals.
class SystemBus {
public:
    SystemBus(memory::Memory* memory = nullptr,
              mmio::MMIOBus* mmioBus = nullptr,
              kernel::InterruptController* interruptController = nullptr);
    ~SystemBus();

    SystemBus(const SystemBus&) = delete;
    SystemBus& operator=(const SystemBus&) = delete;
    SystemBus(SystemBus&&) noexcept = default;
    SystemBus& operator=(SystemBus&&) noexcept = default;

    /// Accesses the central Simulation Clock (mutable).
    [[nodiscard]] clock::SimulationClock& clock() noexcept;

    /// Accesses the central Simulation Clock (read-only).
    [[nodiscard]] const clock::SimulationClock& clock() const noexcept;

    /// Accesses the central Event Scheduler (mutable).
    [[nodiscard]] scheduler::EventScheduler& scheduler() noexcept;

    /// Accesses the central Event Scheduler (read-only).
    [[nodiscard]] const scheduler::EventScheduler& scheduler() const noexcept;

    /// Sets or attaches the Memory subsystem.
    void setMemory(memory::Memory* memory) noexcept;

    /// Returns pointer to attached Memory subsystem.
    [[nodiscard]] memory::Memory* memory() const noexcept;

    /// Sets or attaches the MMIO Bus subsystem.
    void setMMIO(mmio::MMIOBus* mmioBus) noexcept;

    /// Returns pointer to attached MMIO Bus subsystem.
    [[nodiscard]] mmio::MMIOBus* mmio() const noexcept;

    /// Sets or attaches the Interrupt Controller.
    void setInterrupts(kernel::InterruptController* interruptController) noexcept;

    /// Returns pointer to attached Interrupt Controller.
    [[nodiscard]] kernel::InterruptController* interrupts() const noexcept;

    /// Attaches a DMA controller to receive clock tick notifications.
    bool attachDMA(drivers::dma::DMAController* dma);

    /// Detaches a DMA controller from the system bus.
    bool detachDMA(drivers::dma::DMAController* dma);

    /// Returns primary attached DMA controller pointer or nullptr.
    [[nodiscard]] drivers::dma::DMAController* dma() const noexcept;

    /// Returns list of attached DMA controllers.
    [[nodiscard]] const std::vector<drivers::dma::DMAController*>& dmas() const noexcept;

    /// Attaches a hardware timer to receive CPU tick notifications per simulation cycle.
    bool attachTimer(drivers::timer::Timer* timer);

    /// Detaches a hardware timer from the system bus.
    bool detachTimer(drivers::timer::Timer* timer);

    /// Returns list of attached hardware timers.
    [[nodiscard]] const std::vector<drivers::timer::Timer*>& timers() const noexcept;

    /// Advances the Simulation Clock and ticks all attached DMA controllers and hardware timers.
    void tickTimers();

    /// Attaches GPIO peripheral.
    void attachGPIO(drivers::gpio::GPIO* gpio) noexcept;

    /// Returns attached GPIO peripheral pointer.
    [[nodiscard]] drivers::gpio::GPIO* gpio() const noexcept;

    /// Attaches UART peripheral.
    void attachUART(drivers::uart::UART* uart) noexcept;

    /// Returns attached UART peripheral pointer.
    [[nodiscard]] drivers::uart::UART* uart() const noexcept;

    /// Resets all attached peripherals (DMA, GPIO, Timers, UART, Interrupt Controller, Memory, Clock).
    void reset();

private:
    memory::Memory* m_memory{nullptr};
    mmio::MMIOBus* m_mmioBus{nullptr};
    kernel::InterruptController* m_interruptController{nullptr};
    clock::SimulationClock m_clock;
    scheduler::EventScheduler m_scheduler;
    std::vector<drivers::dma::DMAController*> m_dmas;
    std::vector<drivers::timer::Timer*> m_timers;
    drivers::gpio::GPIO* m_gpio{nullptr};
    drivers::uart::UART* m_uart{nullptr};
};

} // namespace efs::system

#endif // EFS_SYSTEM_SYSTEM_BUS_HPP
