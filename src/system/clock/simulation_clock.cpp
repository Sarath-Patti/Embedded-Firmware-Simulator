#include "system/clock/simulation_clock.hpp"
#include "common/logger.hpp"
#include <stdexcept>

namespace efs::system::clock {

SimulationClock::SimulationClock(std::uint64_t frequencyHz)
    : m_frequencyHz(frequencyHz) {
    if (frequencyHz == 0) {
        common::Logger::error("Invalid initial SimulationClock frequency: 0 Hz");
        throw std::invalid_argument("Clock frequency cannot be 0");
    }
}

void SimulationClock::tick(common::QWord count) noexcept {
    m_cycles += count;
}

void SimulationClock::reset() noexcept {
    m_cycles = 0;
}

void SimulationClock::setFrequency(std::uint64_t frequencyHz) {
    if (frequencyHz == 0) {
        common::Logger::error("Attempted to set invalid SimulationClock frequency: 0 Hz");
        throw std::invalid_argument("Clock frequency cannot be 0");
    }
    m_frequencyHz = frequencyHz;
}

std::uint64_t SimulationClock::frequency() const noexcept {
    return m_frequencyHz;
}

common::QWord SimulationClock::cycles() const noexcept {
    return m_cycles;
}

common::QWord SimulationClock::elapsedNanoseconds() const noexcept {
    if (m_frequencyHz == 0) return 0;
    return (m_cycles * 1'000'000'000ULL) / m_frequencyHz;
}

common::QWord SimulationClock::elapsedMicroseconds() const noexcept {
    if (m_frequencyHz == 0) return 0;
    return (m_cycles * 1'000'000ULL) / m_frequencyHz;
}

common::QWord SimulationClock::elapsedMilliseconds() const noexcept {
    if (m_frequencyHz == 0) return 0;
    return (m_cycles * 1'000ULL) / m_frequencyHz;
}

} // namespace efs::system::clock
