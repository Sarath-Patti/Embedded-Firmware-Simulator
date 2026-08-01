#include "hal/timer_hal.hpp"
#include "common/logger.hpp"

namespace efs::hal {

TimerHAL::TimerHAL(drivers::timer::Timer* timer) noexcept
    : m_timer(timer) {
}

TimerHAL::TimerHAL(drivers::timer::Timer& timer) noexcept
    : m_timer(&timer) {
}

void TimerHAL::attachTimer(drivers::timer::Timer* timer) noexcept {
    m_timer = timer;
}

bool TimerHAL::isAttached() const noexcept {
    return m_timer != nullptr;
}

void TimerHAL::start() {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted start with no Timer driver attached.");
        return;
    }
    m_timer->start();
}

void TimerHAL::stop() {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted stop with no Timer driver attached.");
        return;
    }
    m_timer->stop();
}

void TimerHAL::reset() {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted reset with no Timer driver attached.");
        return;
    }
    m_timer->reset();
}

common::DWord TimerHAL::counter() const {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted counter read with no Timer driver attached.");
        return 0;
    }
    return m_timer->counter();
}

void TimerHAL::setCompare(common::DWord value) {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted setCompare with no Timer driver attached.");
        return;
    }
    m_timer->setCompare(value);
}

bool TimerHAL::hasMatch() const {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted hasMatch read with no Timer driver attached.");
        return false;
    }
    return m_timer->hasMatch();
}

bool TimerHAL::running() const {
    if (!m_timer) {
        common::Logger::warning("TimerHAL: Attempted running read with no Timer driver attached.");
        return false;
    }
    return m_timer->running();
}

} // namespace efs::hal
