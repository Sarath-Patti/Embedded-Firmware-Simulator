#ifndef EFS_SYSTEM_CLOCK_SIMULATION_CLOCK_HPP
#define EFS_SYSTEM_CLOCK_SIMULATION_CLOCK_HPP

#include "common/types.hpp"
#include <cstdint>

namespace efs::system::clock {

/// Provides deterministic simulation timing and cycle tracking.
class SimulationClock {
public:
    static constexpr std::uint64_t DEFAULT_FREQUENCY_HZ = 1'000'000; // 1 MHz

    explicit SimulationClock(std::uint64_t frequencyHz = DEFAULT_FREQUENCY_HZ);
    ~SimulationClock() = default;

    SimulationClock(const SimulationClock&) = default;
    SimulationClock& operator=(const SimulationClock&) = default;
    SimulationClock(SimulationClock&&) noexcept = default;
    SimulationClock& operator=(SimulationClock&&) noexcept = default;

    /// Advances the clock by the given cycle count.
    void tick(common::QWord count = 1) noexcept;

    /// Resets total cycle count to zero.
    void reset() noexcept;

    /// Sets the clock frequency in Hz. Throws std::invalid_argument if frequency is 0.
    void setFrequency(std::uint64_t frequencyHz);

    /// Returns current clock frequency in Hz.
    [[nodiscard]] std::uint64_t frequency() const noexcept;

    /// Returns total elapsed cycles.
    [[nodiscard]] common::QWord cycles() const noexcept;

    /// Returns total elapsed time in nanoseconds.
    [[nodiscard]] common::QWord elapsedNanoseconds() const noexcept;

    /// Returns total elapsed time in microseconds.
    [[nodiscard]] common::QWord elapsedMicroseconds() const noexcept;

    /// Returns total elapsed time in milliseconds.
    [[nodiscard]] common::QWord elapsedMilliseconds() const noexcept;

private:
    std::uint64_t m_frequencyHz{DEFAULT_FREQUENCY_HZ};
    common::QWord m_cycles{0};
};

} // namespace efs::system::clock

#endif // EFS_SYSTEM_CLOCK_SIMULATION_CLOCK_HPP
