#ifndef EFS_CPU_CPU_HPP
#define EFS_CPU_CPU_HPP

#include "common/types.hpp"
#include "cpu/registers/register_file.hpp"
#include "drivers/dma/dma_controller.hpp"
#include "drivers/timer/timer.hpp"
#include "firmware/firmware.hpp"
#include "firmware/firmware_manager.hpp"
#include "kernel/interrupt_controller.hpp"
#include "rtos/rtos_scheduler.hpp"
#include "system/power/power_controller.hpp"
#include "system/system_bus.hpp"
#include <memory>
#include <string>
#include <vector>

namespace efs::cpu {

/// Central CPU execution engine coordinating cycle simulation steps, SystemBus, FirmwareManager, RTOS Scheduler, and PowerController.
class CPU {
public:
    explicit CPU(system::SystemBus* systemBus = nullptr);
    explicit CPU(kernel::InterruptController* interruptController);
    ~CPU();

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    CPU(CPU&&) = delete;
    CPU& operator=(CPU&&) = delete;

    /// Accesses the PowerController instance (mutable).
    [[nodiscard]] system::power::PowerController& powerController() noexcept;

    /// Accesses the PowerController instance (read-only).
    [[nodiscard]] const system::power::PowerController& powerController() const noexcept;

    /// Sets or attaches an external PowerController reference.
    void setPowerController(system::power::PowerController* powerController) noexcept;

    /// Accesses the central FirmwareManager component (mutable).
    [[nodiscard]] firmware::FirmwareManager& firmwareManager() noexcept;

    /// Accesses the central FirmwareManager component (read-only).
    [[nodiscard]] const firmware::FirmwareManager& firmwareManager() const noexcept;

    /// Sets or attaches the SystemBus reference.
    void setSystemBus(system::SystemBus* systemBus) noexcept;

    /// Returns pointer to attached SystemBus.
    [[nodiscard]] system::SystemBus* systemBus() const noexcept;

    /// Attaches RTOS Task Scheduler.
    void attachRTOSScheduler(rtos::Scheduler* scheduler) noexcept;

    /// Returns attached RTOS Task Scheduler pointer or nullptr.
    [[nodiscard]] rtos::Scheduler* rtosScheduler() const noexcept;

    /// Starts CPU execution and initializes active firmware via FirmwareManager.
    void start();

    /// Halts CPU execution and shuts down active firmware via FirmwareManager.
    void stop();

    /// Resets the CPU cycle counter, register file, FirmwareManager, and SystemBus state.
    void reset();

    /// Executes exactly one simulation step.
    void step();

    /// Executes N simulation cycles while CPU remains in running state and power is ON.
    void run(common::QWord cycles);

    /// Helper loading firmware into FirmwareManager under a given name.
    bool loadFirmware(std::shared_ptr<firmware::Firmware> firmware, const std::string& name = "default");

    /// Helper unloading/shutting down current active firmware.
    void unloadFirmware();

    /// Returns true if an active firmware instance is loaded in FirmwareManager.
    [[nodiscard]] bool firmwareLoaded() const noexcept;

    /// Returns pointer to current active firmware instance or nullptr.
    [[nodiscard]] std::shared_ptr<firmware::Firmware> firmware() const noexcept;

    /// Attaches a DMA controller to receive tick notifications via SystemBus.
    bool attachDMA(drivers::dma::DMAController* dma);

    /// Detaches a DMA controller from the SystemBus.
    bool detachDMA(drivers::dma::DMAController* dma);

    /// Returns primary attached DMA controller pointer or nullptr.
    [[nodiscard]] drivers::dma::DMAController* dmaController() const noexcept;

    /// Attaches a hardware timer to receive CPU tick notifications via SystemBus.
    bool attachTimer(drivers::timer::Timer* timer);

    /// Detaches a hardware timer from the SystemBus.
    bool detachTimer(drivers::timer::Timer* timer);

    /// Sets the active interrupt controller on the SystemBus.
    void setInterruptController(kernel::InterruptController* controller);

    /// Returns pointer to active interrupt controller on the SystemBus.
    [[nodiscard]] kernel::InterruptController* interruptController() const noexcept;

    /// Returns reference to CPU Register File (read-only).
    [[nodiscard]] const registers::RegisterFile& registerFile() const noexcept;

    /// Returns reference to CPU Register File (mutable).
    [[nodiscard]] registers::RegisterFile& registerFile() noexcept;

    /// Returns total simulation cycles executed by CPU.
    [[nodiscard]] common::QWord cycleCount() const noexcept;

private:
    system::SystemBus* m_systemBus{nullptr};
    std::unique_ptr<system::SystemBus> m_ownedSystemBus;

    system::power::PowerController m_ownedPowerController;
    system::power::PowerController* m_powerController{&m_ownedPowerController};

    firmware::FirmwareManager m_firmwareManager;
    rtos::Scheduler* m_rtosScheduler{nullptr};
    registers::RegisterFile m_registerFile;

    bool m_running{false};
    common::QWord m_cycleCount{0};
};

} // namespace efs::cpu

#endif // EFS_CPU_CPU_HPP
