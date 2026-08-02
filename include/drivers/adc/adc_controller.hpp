#ifndef EFS_DRIVERS_ADC_ADC_CONTROLLER_HPP
#define EFS_DRIVERS_ADC_ADC_CONTROLLER_HPP

#include "common/types.hpp"
#include "mmio/mmio_bus.hpp"
#include "mmio/register.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace efs::drivers::adc {

/// Analog-to-Digital Converter (ADC) peripheral modeling multi-channel sampling, resolution, reference voltage, and MMIO registers.
class ADCController {
public:
    static constexpr common::Address CTRL_OFFSET   = 0x00;
    static constexpr common::Address STATUS_OFFSET = 0x04;
    static constexpr common::Address RES_OFFSET    = 0x08;
    static constexpr common::Address DATA_OFFSET   = 0x0C;

    static constexpr common::DWord CTRL_ENABLE_BIT     = (1U << 0);
    static constexpr common::DWord STATUS_ENABLED_BIT  = (1U << 0);
    static constexpr common::DWord STATUS_COMPLETE_BIT = (1U << 1);

    explicit ADCController(mmio::MMIOBus& bus,
                           common::Address baseAddress,
                           std::uint8_t resolutionBits = 12,
                           double referenceVoltage = 3.3,
                           std::size_t channelCount = 8);
    ~ADCController();

    ADCController(const ADCController&) = delete;
    ADCController& operator=(const ADCController&) = delete;
    ADCController(ADCController&&) = delete;
    ADCController& operator=(ADCController&&) = delete;

    /// Enables the ADC peripheral.
    void enable();

    /// Disables the ADC peripheral.
    void disable();

    /// Returns true if the ADC peripheral is enabled.
    [[nodiscard]] bool enabled() const noexcept;

    /// Sets ADC sampling resolution in bits (supported: 8, 10, 12).
    void setResolution(std::uint8_t bits);

    /// Returns current ADC sampling resolution in bits.
    [[nodiscard]] std::uint8_t resolution() const noexcept;

    /// Sets ADC reference voltage in Volts. Throws std::invalid_argument if voltage <= 0.0.
    void setReferenceVoltage(double voltage);

    /// Returns ADC reference voltage in Volts.
    [[nodiscard]] double referenceVoltage() const noexcept;

    /// Sets simulated analog input voltage on a specific channel.
    void setAnalogInput(std::size_t channel, double voltage);

    /// Returns current simulated analog input voltage on a specific channel.
    [[nodiscard]] double analogInput(std::size_t channel) const;

    /// Digitizes input voltage on specified channel and returns raw digital conversion value.
    std::uint32_t sample(std::size_t channel);

    /// Returns last sampled digital raw value.
    [[nodiscard]] std::uint32_t lastSample() const noexcept;

    /// Returns total number of analog input channels.
    [[nodiscard]] std::size_t channelCount() const noexcept;

    /// Resets registers, channel voltages, resolution, reference voltage, and state.
    void reset();

    [[nodiscard]] common::Address baseAddress() const noexcept;
    [[nodiscard]] common::Address controlAddress() const noexcept;
    [[nodiscard]] common::Address statusAddress() const noexcept;
    [[nodiscard]] common::Address resAddress() const noexcept;
    [[nodiscard]] common::Address dataAddress() const noexcept;

private:
    void updateStatusRegister();

    mmio::MMIOBus& m_bus;
    common::Address m_baseAddress;

    std::uint8_t m_resolutionBits{12};
    double m_referenceVoltage{3.3};
    std::vector<double> m_channels;

    std::shared_ptr<mmio::Register> m_controlRegister;
    std::shared_ptr<mmio::Register> m_statusRegister;
    std::shared_ptr<mmio::Register> m_resRegister;
    std::shared_ptr<mmio::Register> m_dataRegister;

    std::uint32_t m_lastSample{0};
};

} // namespace efs::drivers::adc

#endif // EFS_DRIVERS_ADC_ADC_CONTROLLER_HPP
