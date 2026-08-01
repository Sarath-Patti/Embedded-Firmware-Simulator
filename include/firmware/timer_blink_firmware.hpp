#ifndef EFS_FIRMWARE_TIMER_BLINK_FIRMWARE_HPP
#define EFS_FIRMWARE_TIMER_BLINK_FIRMWARE_HPP

#include "firmware/firmware.hpp"
#include "hal/gpio_hal.hpp"
#include "hal/timer_hal.hpp"
#include <cstdint>

namespace efs::firmware {

/// Sample firmware toggling a GPIO pin based on hardware TimerHAL compare matches.
class TimerBlinkFirmware : public Firmware {
public:
    TimerBlinkFirmware(hal::GPIOHAL& gpioHAL, hal::TimerHAL& timerHAL, std::uint8_t pin, common::Size compareValue = 5);
    ~TimerBlinkFirmware() override;

    void initialize() override;
    void update() override;
    void execute() override;
    void shutdown() override;
    void reset() override;

    [[nodiscard]] common::Size toggleCount() const noexcept;
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] bool isShutdown() const noexcept;

private:
    hal::GPIOHAL& m_gpioHAL;
    hal::TimerHAL& m_timerHAL;
    std::uint8_t m_pin;
    common::Size m_compareValue;
    common::Size m_toggleCount{0};
    bool m_initialized{false};
    bool m_shutdown{false};
};

} // namespace efs::firmware

#endif // EFS_FIRMWARE_TIMER_BLINK_FIRMWARE_HPP
