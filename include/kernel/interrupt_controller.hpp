#ifndef EFS_KERNEL_INTERRUPT_CONTROLLER_HPP
#define EFS_KERNEL_INTERRUPT_CONTROLLER_HPP

#include "common/types.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <array>
#include <cstdint>
#include <functional>
#include <memory>

namespace efs::kernel {

constexpr common::Size MAX_INTERRUPTS = 32;

using ISRHandler = std::function<void()>;

/// Priority Interrupt Controller managing 32 hardware IRQ lines mapped via MMIO.
class InterruptController {
public:
    static constexpr common::Address ENABLE_OFFSET   = 0x00;
    static constexpr common::Address PENDING_OFFSET  = 0x04;
    static constexpr common::Address PRIORITY_OFFSET = 0x08;

    InterruptController(mmio::MMIOBus& bus, common::Address baseAddress);
    ~InterruptController();

    InterruptController(const InterruptController&) = delete;
    InterruptController& operator=(const InterruptController&) = delete;
    InterruptController(InterruptController&&) = delete;
    InterruptController& operator=(InterruptController&&) = delete;

    /// Registers a hardware interrupt line. Returns false if already registered.
    bool registerInterrupt(std::uint8_t id);

    /// Unregisters a hardware interrupt line. Returns false if not registered.
    bool unregisterInterrupt(std::uint8_t id);

    /// Enables an interrupt line.
    void enable(std::uint8_t id);

    /// Disables an interrupt line.
    void disable(std::uint8_t id);

    /// Triggers an interrupt line, marking it pending.
    void trigger(std::uint8_t id);

    /// Clears the pending flag for an interrupt line.
    void clear(std::uint8_t id);

    /// Returns true if an interrupt line is currently pending.
    [[nodiscard]] bool pending(std::uint8_t id) const;

    /// Returns true if an interrupt line is currently enabled.
    [[nodiscard]] bool enabled(std::uint8_t id) const;

    /// Returns true if an interrupt line is registered.
    [[nodiscard]] bool isRegistered(std::uint8_t id) const noexcept;

    /// Sets the priority level of an interrupt line (lower numerical value = higher priority).
    void setPriority(std::uint8_t id, std::uint8_t priority);

    /// Returns the priority level of an interrupt line.
    [[nodiscard]] std::uint8_t getPriority(std::uint8_t id) const;

    /// Registers an ISR callback handler for an interrupt line.
    bool registerHandler(std::uint8_t id, ISRHandler callback);

    /// Unregisters an ISR callback handler for an interrupt line.
    bool unregisterHandler(std::uint8_t id);

    /// Dispatches the highest-priority enabled pending interrupt to its registered ISR. Returns true if dispatched.
    bool dispatch();

    /// Returns base address of MMIO register block.
    [[nodiscard]] common::Address baseAddress() const noexcept;

    /// Returns address of ENABLE register.
    [[nodiscard]] common::Address enableAddress() const noexcept;

    /// Returns address of PENDING register.
    [[nodiscard]] common::Address pendingAddress() const noexcept;

    /// Returns address of PRIORITY register.
    [[nodiscard]] common::Address priorityAddress() const noexcept;

private:
    void validateInterruptId(std::uint8_t id) const;

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::shared_ptr<mmio::Register> m_enableRegister;
    std::shared_ptr<mmio::Register> m_pendingRegister;
    std::shared_ptr<mmio::Register> m_priorityRegister;

    std::array<bool, MAX_INTERRUPTS> m_registered;
    std::array<std::uint8_t, MAX_INTERRUPTS> m_priorities;
    std::array<ISRHandler, MAX_INTERRUPTS> m_handlers;
};

} // namespace efs::kernel

#endif // EFS_KERNEL_INTERRUPT_CONTROLLER_HPP
