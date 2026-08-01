#include "system/power/power_controller.hpp"
#include "common/logger.hpp"

namespace efs::system::power {

void PowerController::powerOn() noexcept {
    if (m_state != PowerState::ON) {
        common::Logger::info("PowerController: Power ON");
        m_state = PowerState::ON;
    }
}

void PowerController::powerOff() noexcept {
    if (m_state != PowerState::OFF) {
        common::Logger::info("PowerController: Power OFF");
        m_state = PowerState::OFF;
    }
}

void PowerController::sleep() noexcept {
    if (m_state == PowerState::ON) {
        common::Logger::info("PowerController: Entering SLEEP mode");
        m_state = PowerState::SLEEP;
    } else {
        common::Logger::warning("PowerController: Cannot sleep unless in ON state");
    }
}

void PowerController::wake() noexcept {
    if (m_state == PowerState::SLEEP) {
        common::Logger::info("PowerController: Waking up from SLEEP mode");
        m_state = PowerState::ON;
    } else {
        common::Logger::warning("PowerController: Cannot wake unless in SLEEP state");
    }
}

PowerState PowerController::state() const noexcept {
    return m_state;
}

bool PowerController::isPowerOn() const noexcept {
    return m_state == PowerState::ON;
}

bool PowerController::isSleep() const noexcept {
    return m_state == PowerState::SLEEP;
}

bool PowerController::isPowerOff() const noexcept {
    return m_state == PowerState::OFF;
}

} // namespace efs::system::power
