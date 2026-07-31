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

    bool registerInterrupt(std::uint8_t id);
    bool unregisterInterrupt(std::uint8_t id);

    void enable(std::uint8_t id);
    void disable(std::uint8_t id);
    void trigger(std::uint8_t id);
    void clear(std::uint8_t id);

    [[nodiscard]] bool pending(std::uint8_t id) const;
    [[nodiscard]] bool enabled(std::uint8_t id) const;
    [[nodiscard]] bool isRegistered(std::uint8_t id) const noexcept;

    void setPriority(std::uint8_t id, std::uint8_t priority);
    [[nodiscard]] std::uint8_t getPriority(std::uint8_t id) const;

    bool registerHandler(std::uint8_t id, ISRHandler callback);
    bool unregisterHandler(std::uint8_t id);

    bool dispatch();

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address enableAddress() const noexcept;
    [[nodiscard]] common::Address pendingAddress() const noexcept;
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
