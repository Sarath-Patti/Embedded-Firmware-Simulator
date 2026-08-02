#ifndef EFS_HAL_ADC_HAL_HPP
#define EFS_HAL_ADC_HAL_HPP

#include "drivers/adc/adc_controller.hpp"
#include <cstddef>
#include <cstdint>

namespace efs::hal {

/// Hardware Abstraction Layer for Analog-to-Digital Converter (ADC) peripheral sampling.
class ADCHAL {
public:
    explicit ADCHAL(drivers::adc::ADCController* controller = nullptr) noexcept;
    explicit ADCHAL(drivers::adc::ADCController& controller) noexcept;
    ~ADCHAL() = default;

    ADCHAL(const ADCHAL&) = default;
    ADCHAL& operator=(const ADCHAL&) = default;
    ADCHAL(ADCHAL&&) noexcept = default;
    ADCHAL& operator=(ADCHAL&&) noexcept = default;

    /// Attaches or updates underlying ADC controller driver.
    void attachADC(drivers::adc::ADCController* controller) noexcept;

    /// Returns true if an ADC controller driver is attached.
    [[nodiscard]] bool isAttached() const noexcept;

    /// Enables the ADC peripheral.
    void enable();

    /// Disables the ADC peripheral.
    void disable();

    /// Returns true if ADC peripheral is enabled.
    [[nodiscard]] bool enabled() const;

    /// Samples specified analog channel and returns digital conversion value.
    std::uint32_t read(std::size_t channel);

    /// Sets ADC reference voltage.
    void setReferenceVoltage(double voltage);

    /// Returns current ADC reference voltage.
    [[nodiscard]] double referenceVoltage() const;

private:
    drivers::adc::ADCController* m_controller{nullptr};
};

} // namespace efs::hal

#endif // EFS_HAL_ADC_HAL_HPP
