#ifndef EFS_SYSTEM_SCHEDULER_EVENT_SCHEDULER_HPP
#define EFS_SYSTEM_SCHEDULER_EVENT_SCHEDULER_HPP

#include "common/types.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace efs::system::scheduler {

using EventId = std::uint64_t;

struct Event {
    EventId id{0};
    common::QWord cycle{0};
    std::function<void()> callback;
    std::string description;
};

/// Deterministic Simulation-Clock-driven event scheduler for callback execution.
class EventScheduler {
public:
    EventScheduler() = default;
    ~EventScheduler() = default;

    EventScheduler(const EventScheduler&) = delete;
    EventScheduler& operator=(const EventScheduler&) = delete;
    EventScheduler(EventScheduler&&) noexcept = default;
    EventScheduler& operator=(EventScheduler&&) noexcept = default;

    /// Schedules a callback function to execute at target simulation cycle. Returns unique EventId.
    EventId schedule(std::function<void()> callback, common::QWord cycle, std::string description = "");

    /// Cancels a pending event by ID. Returns true if found and removed.
    bool cancel(EventId eventId);

    /// Clears all pending events.
    void clear();

    /// Returns a list of currently pending events sorted by cycle and ID (FIFO).
    [[nodiscard]] std::vector<Event> pendingEvents() const;

    /// Returns number of pending events.
    [[nodiscard]] std::size_t pendingCount() const noexcept;

    /// Executes all ready events whose cycle <= currentCycle in FIFO order. Returns number of executed events.
    std::size_t executeReadyEvents(common::QWord currentCycle);

private:
    EventId m_nextId{1};
    std::vector<Event> m_events;
};

} // namespace efs::system::scheduler

#endif // EFS_SYSTEM_SCHEDULER_EVENT_SCHEDULER_HPP
