#ifndef EFS_DRIVERS_GPIO_GPIO_HPP
#define EFS_DRIVERS_GPIO_GPIO_HPP

#include "common/types.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstdint>
#include <memory>

namespace efs::drivers::gpio {

enum class PinDirection {
    Input,
    Output
};

enum class PinState {
    Low = 0,
    High = 1
};

constexpr common::Size MAX_PINS = 32;

/// 32-pin GPIO peripheral modeling DIR, OUT, and IN registers over MMIO.
class GPIO {
public:
    static constexpr common::Address DIR_OFFSET = 0x00;
    static constexpr common::Address OUT_OFFSET = 0x04;
    static constexpr common::Address IN_OFFSET  = 0x08;

    GPIO(mmio::MMIOBus& bus, common::Address baseAddress);
    ~GPIO();

    GPIO(const GPIO&) = delete;
    GPIO& operator=(const GPIO&) = delete;
    GPIO(GPIO&&) = delete;
    GPIO& operator=(GPIO&&) = delete;

    /// Configures pin direction (Input or Output).
    void configurePin(std::uint8_t pin, PinDirection direction);

    /// Writes PinState (High/Low) to output pin.
    void writePin(std::uint8_t pin, PinState state);

    /// Reads PinState (High/Low) of specified pin.
    [[nodiscard]] PinState readPin(std::uint8_t pin) const;

    /// Toggles the state of specified output pin.
    void togglePin(std::uint8_t pin);

    /// Simulates external signal input on a pin.
    void setExternalInput(std::uint8_t pin, PinState state);

    /// Resets DIR, OUT, and IN registers to 0.
    void reset();

    /// Returns MMIO base address.
    [[nodiscard]] common::Address baseAddress() const noexcept;

    /// Returns address of DIR register.
    [[nodiscard]] common::Address dirAddress() const noexcept;

    /// Returns address of OUT register.
    [[nodiscard]] common::Address outAddress() const noexcept;

    /// Returns address of IN register.
    [[nodiscard]] common::Address inAddress() const noexcept;

private:
    void validatePin(std::uint8_t pin) const;
    [[nodiscard]] bool isOutputPin(std::uint8_t pin) const noexcept;

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;
    std::shared_ptr<mmio::Register> m_dirRegister;
    std::shared_ptr<mmio::Register> m_outRegister;
    std::shared_ptr<mmio::Register> m_inRegister;
};

} // namespace efs::drivers::gpio

#endif // EFS_DRIVERS_GPIO_GPIO_HPP
