#include "drivers/adc/adc_controller.hpp"
#include "hal/adc_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>

int main() {
    std::cout << "Starting Analog-to-Digital Converter (ADC) Demo...\n";

    // 1. Initialize MMIO Bus and SystemBus
    efs::mmio::MMIOBus mmioBus;
    efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

    constexpr efs::common::Address ADC_BASE_ADDR = 0x40007000U;

    // 2. Instantiate ADC Controller (12-bit resolution, 3.3V reference voltage, 4 channels)
    efs::drivers::adc::ADCController adc(mmioBus, ADC_BASE_ADDR, 12, 3.3, 4);
    systemBus.attachADC(&adc);

    // 3. Attach ADC HAL for firmware access
    efs::hal::ADCHAL adcHAL(&adc);
    adcHAL.enable();

    std::cout << "ADC Peripheral enabled.\n";
    std::cout << "  Base Address:       0x" << std::hex << ADC_BASE_ADDR << std::dec << "\n";
    std::cout << "  Resolution:         " << static_cast<int>(adc.resolution()) << "-bit (0.." << ((1U << adc.resolution()) - 1) << ")\n";
    std::cout << "  Reference Voltage:  " << adcHAL.referenceVoltage() << " V\n";
    std::cout << "  Channel Count:      " << adc.channelCount() << "\n\n";

    // 4. Demo 1: Multi-Channel Analog Sensor Sampling
    std::cout << "--- Demo 1: Multi-Channel Analog Sensor Sampling ---\n";
    // Simulate sensors connected to ADC channels:
    // Channel 0: Potentiometer (1.65 V)
    // Channel 1: Temperature Sensor LM35 (0.75 V)
    // Channel 2: LiPo Battery Voltage Divider (3.0 V)
    // Channel 3: Over-voltage Sensor (4.2 V - exceeds 3.3V reference)
    adc.setAnalogInput(0, 1.65);
    adc.setAnalogInput(1, 0.75);
    adc.setAnalogInput(2, 3.00);
    adc.setAnalogInput(3, 4.20);

    for (std::size_t ch = 0; ch < adc.channelCount(); ++ch) {
        std::uint32_t rawDigital = adcHAL.read(ch);
        double inputVolt = adc.analogInput(ch);
        double reconstructedVolt = (static_cast<double>(rawDigital) / 4095.0) * adcHAL.referenceVoltage();

        std::cout << "  Channel " << ch << " | Analog Input: "
                  << std::fixed << std::setprecision(2) << inputVolt << " V | Digital Raw: "
                  << std::setw(4) << rawDigital << " | Reconstructed: "
                  << std::setprecision(2) << reconstructedVolt << " V"
                  << (inputVolt > adcHAL.referenceVoltage() ? " [SATURATED]" : "") << "\n";
    }

    // 5. Demo 2: Resolution Dynamics (8-bit vs 10-bit vs 12-bit)
    std::cout << "\n--- Demo 2: Resolution Dynamics (8-bit, 10-bit, 12-bit) ---\n";
    const std::uint8_t resolutions[] = {8, 10, 12};
    const double sampleVoltage = 2.475; // 75% of 3.3V reference
    adc.setAnalogInput(0, sampleVoltage);

    for (std::uint8_t res : resolutions) {
        adc.setResolution(res);
        std::uint32_t maxDigital = (1U << res) - 1;
        std::uint32_t raw = adcHAL.read(0);

        std::cout << "  Resolution: " << std::setw(2) << static_cast<int>(res)
                  << "-bit | Max Digital: " << std::setw(4) << maxDigital
                  << " | Raw Output: " << std::setw(4) << raw << "\n";
    }

    // 6. SystemBus Reset Verification
    std::cout << "\nResetting SystemBus...\n";
    systemBus.reset();
    std::cout << "ADC Enabled after reset? " << (adcHAL.enabled() ? "Yes" : "No") << "\n";

    std::cout << "\nADC Controller Demo completed successfully.\n";
    return 0;
}
