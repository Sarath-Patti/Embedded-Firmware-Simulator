#ifndef EFS_CPU_CPU_HPP
#define EFS_CPU_CPU_HPP

#include "common/types.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include "firmware/firmware.hpp"
#include "cpu/registers/register_file.hpp"
#include "system/system_bus.hpp"
#include <memory>
#include <vector>

namespace efs::cpu {

/// Central CPU execution engine coordinating cycle simulation steps, SystemBus, and firmware.
class CPU {
public:
    explicit CPU(system::SystemBus* systemBus = nullptr);
    explicit CPU(kernel::InterruptController* interruptController);
    ~CPU() = default;

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    CPU(CPU&&) = delete;
    CPU& operator=(CPU&&) = delete;

    /// Sets or attaches the SystemBus reference.
    void setSystemBus(system::SystemBus* systemBus) noexcept;

    /// Returns pointer to attached SystemBus.
    [[nodiscard]] system::SystemBus* systemBus() const noexcept;

    /// Starts CPU execution and initializes loaded firmware if present.
    void start();

    /// Halts CPU execution and shuts down loaded firmware if present.
    void stop();

    /// Resets the CPU cycle counter and register file state.
    void reset();

    /// Executes exactly one simulation step (firmware tick, SystemBus timer update, SystemBus interrupt dispatch).
    void step();

    /// Executes N simulation cycles while CPU remains in running state.
    void run(common::QWord cycles);

    /// Loads firmware into the CPU. Automatically halts active execution first.
    bool loadFirmware(std::shared_ptr<firmware::Firmware> firmware);

    /// Unloads current firmware from the CPU.
    void unloadFirmware();

    /// Returns true if a valid firmware instance is loaded.
    [[nodiscard]] bool firmwareLoaded() const noexcept;

    /// Returns pointer to current firmware instance or nullptr.
    [[nodiscard]] std::shared_ptr<firmware::Firmware> firmware() const noexcept;

    /// Attaches a hardware timer to receive CPU tick notifications via SystemBus.
    bool attachTimer(drivers::timer::Timer* timer);

    /// Detaches a hardware timer from the SystemBus.
    bool detachTimer(drivers::timer::Timer* timer);

    /// Sets the active interrupt controller on the SystemBus.
    void setInterruptController(kernel::InterruptController* controller);

    /// Returns pointer to active interrupt controller on the SystemBus.
    [[nodiscard]] kernel::InterruptController* interruptController() const noexcept;

    /// Accesses the CPU RegisterFile state (read-only).
    [[nodiscard]] const registers::RegisterFile& registerFile() const noexcept;

    /// Accesses the CPU RegisterFile state (mutable).
    [[nodiscard]] registers::RegisterFile& registerFile() noexcept;

    /// Returns total simulation cycle count since last reset.
    [[nodiscard]] common::QWord cycleCount() const noexcept;

    /// Returns true if CPU is currently running.
    [[nodiscard]] bool running() const noexcept;

private:
    common::QWord m_cycleCount{0};
    bool m_running{false};
    registers::RegisterFile m_registerFile;
    std::shared_ptr<firmware::Firmware> m_firmware{nullptr};
    system::SystemBus* m_systemBus{nullptr};
    std::unique_ptr<system::SystemBus> m_ownedSystemBus{nullptr};
};

} // namespace efs::cpu

#endif // EFS_CPU_CPU_HPP
