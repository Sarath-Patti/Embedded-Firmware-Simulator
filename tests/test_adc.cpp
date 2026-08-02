#include "drivers/adc/adc_controller.hpp"
#include "hal/adc_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace efs::drivers::adc;
using namespace efs::hal;
using namespace efs::mmio;
using namespace efs::system;

void test_adc_initialization() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 12, 3.3, 8);

    assert(bus.contains(adc.controlAddress()));
    assert(bus.contains(adc.statusAddress()));
    assert(bus.contains(adc.resAddress()));
    assert(bus.contains(adc.dataAddress()));

    assert(!adc.enabled());
    assert(adc.resolution() == 12);
    assert(std::abs(adc.referenceVoltage() - 3.3) < 1e-6);
    assert(adc.channelCount() == 8);

    adc.enable();
    assert(adc.enabled());
    assert((bus.read(adc.controlAddress()) & ADCController::CTRL_ENABLE_BIT) != 0);

    adc.disable();
    assert(!adc.enabled());

    std::cout << "[PASS] test_adc_initialization\n";
}

void test_adc_resolution_changes() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000);

    adc.setResolution(8);
    assert(adc.resolution() == 8);
    assert(bus.read(adc.resAddress()) == 8);

    adc.setResolution(10);
    assert(adc.resolution() == 10);

    adc.setResolution(12);
    assert(adc.resolution() == 12);

    bool invalid_res = false;
    try {
        adc.setResolution(16);
    } catch (const std::invalid_argument&) {
        invalid_res = true;
    }
    if (!invalid_res) {
        throw std::runtime_error("Expected invalid_argument exception for resolution 16");
    }
    assert(invalid_res);

    std::cout << "[PASS] test_adc_resolution_changes\n";
}

void test_adc_reference_voltage() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000);

    adc.setReferenceVoltage(5.0);
    assert(std::abs(adc.referenceVoltage() - 5.0) < 1e-6);

    bool invalid_ref = false;
    try {
        adc.setReferenceVoltage(0.0);
    } catch (const std::invalid_argument&) {
        invalid_ref = true;
    }
    if (!invalid_ref) {
        throw std::runtime_error("Expected invalid_argument exception for reference voltage 0.0");
    }
    assert(invalid_ref);

    std::cout << "[PASS] test_adc_reference_voltage\n";
}

void test_adc_analog_sampling() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 12, 3.3, 8);
    adc.enable();

    // 0V input (zero scale) -> raw = 0
    adc.setAnalogInput(0, 0.0);
    std::uint32_t raw0 = adc.sample(0);
    if (raw0 != 0) {
        throw std::runtime_error("ADC raw0 sampling failed");
    }
    assert(raw0 == 0);

    // 1.65V input (half-scale of 3.3V) -> 12-bit max = 4095, half = 2048
    adc.setAnalogInput(0, 1.65);
    std::uint32_t rawHalf = adc.sample(0);
    if (rawHalf != 2048) {
        throw std::runtime_error("ADC rawHalf sampling failed");
    }
    assert(rawHalf == 2048);
    assert(adc.lastSample() == 2048);
    assert(bus.read(adc.dataAddress()) == 2048);

    // 3.3V input (full-scale ref) -> 4095
    adc.setAnalogInput(0, 3.3);
    std::uint32_t rawFull = adc.sample(0);
    if (rawFull != 4095) {
        throw std::runtime_error("ADC rawFull sampling failed");
    }
    assert(rawFull == 4095);

    std::cout << "[PASS] test_adc_analog_sampling\n";
}

void test_adc_multiple_channels() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 8 /* 8-bit */, 3.3, 4);
    adc.enable();

    adc.setAnalogInput(0, 0.0);
    adc.setAnalogInput(1, 1.1);
    adc.setAnalogInput(2, 2.2);
    adc.setAnalogInput(3, 3.3);

    // 8-bit max = 255
    assert(adc.sample(0) == 0);
    assert(adc.sample(1) == static_cast<std::uint32_t>(std::round(1.1 / 3.3 * 255)));
    assert(adc.sample(2) == static_cast<std::uint32_t>(std::round(2.2 / 3.3 * 255)));
    assert(adc.sample(3) == 255);

    std::cout << "[PASS] test_adc_multiple_channels\n";
}

void test_adc_saturation() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 10 /* 10-bit max = 1023 */, 3.3, 4);
    adc.enable();

    // Exceed reference voltage -> saturates at max 1023
    adc.setAnalogInput(0, 4.5);
    std::uint32_t raw = adc.sample(0);
    if (raw != 1023) {
        throw std::runtime_error("ADC saturation sampling failed");
    }
    assert(raw == 1023);

    std::cout << "[PASS] test_adc_saturation\n";
}

void test_adc_invalid_channels() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 12, 3.3, 4);
    adc.enable();

    bool invalid_ch_set = false;
    try {
        adc.setAnalogInput(5, 1.0);
    } catch (const std::out_of_range&) {
        invalid_ch_set = true;
    }
    if (!invalid_ch_set) {
        throw std::runtime_error("Expected out_of_range exception for channel 5 set");
    }
    assert(invalid_ch_set);

    bool invalid_ch_sample = false;
    try {
        adc.sample(5);
    } catch (const std::out_of_range&) {
        invalid_ch_sample = true;
    }
    if (!invalid_ch_sample) {
        throw std::runtime_error("Expected out_of_range exception for channel 5 sample");
    }
    assert(invalid_ch_sample);

    std::cout << "[PASS] test_adc_invalid_channels\n";
}

void test_adc_reset_behaviour() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 8, 5.0, 4);
    adc.enable();
    adc.setAnalogInput(0, 2.5);
    adc.sample(0);

    adc.reset();
    assert(!adc.enabled());
    assert(adc.resolution() == 12);
    assert(std::abs(adc.referenceVoltage() - 3.3) < 1e-6);
    assert(std::abs(adc.analogInput(0) - 0.0) < 1e-6);
    assert(adc.lastSample() == 0);

    std::cout << "[PASS] test_adc_reset_behaviour\n";
}

void test_adc_hal_interface() {
    MMIOBus bus;
    ADCController adc(bus, 0x40007000, 12, 3.3, 4);

    ADCHAL hal(&adc);
    assert(hal.isAttached());

    assert(!hal.enabled());
    hal.enable();
    assert(hal.enabled());

    hal.setReferenceVoltage(5.0);
    assert(std::abs(hal.referenceVoltage() - 5.0) < 1e-6);
    assert(std::abs(adc.referenceVoltage() - 5.0) < 1e-6);

    adc.setAnalogInput(1, 2.5);
    std::uint32_t val = hal.read(1);
    if (val != 2048) {
        throw std::runtime_error("ADCHAL read failed");
    }
    assert(val == 2048);

    hal.disable();
    assert(!hal.enabled());

    // Unattached HAL
    ADCHAL unattached;
    assert(!unattached.isAttached());

    bool threw_unattached = false;
    try {
        unattached.read(0);
    } catch (const std::runtime_error&) {
        threw_unattached = true;
    }
    if (!threw_unattached) {
        throw std::runtime_error("Expected runtime_error exception for unattached ADCHAL");
    }
    assert(threw_unattached);

    std::cout << "[PASS] test_adc_hal_interface\n";
}

void test_adc_system_bus_integration() {
    MMIOBus bus;
    SystemBus systemBus(nullptr, &bus, nullptr);
    ADCController adc(bus, 0x40007000);

    systemBus.attachADC(&adc);
    assert(systemBus.adc() == &adc);

    adc.enable();
    systemBus.reset();
    assert(!adc.enabled());

    std::cout << "[PASS] test_adc_system_bus_integration\n";
}

int main() {
    std::cout << "Running Analog-to-Digital Converter (ADC) unit tests...\n";
    test_adc_initialization();
    test_adc_resolution_changes();
    test_adc_reference_voltage();
    test_adc_analog_sampling();
    test_adc_multiple_channels();
    test_adc_saturation();
    test_adc_invalid_channels();
    test_adc_reset_behaviour();
    test_adc_hal_interface();
    test_adc_system_bus_integration();
    std::cout << "All ADC unit tests passed successfully.\n";
    return 0;
}
