#include "system/scheduler/event_scheduler.hpp"
#include "common/logger.hpp"
#include <algorithm>

namespace efs::system::scheduler {

EventId EventScheduler::schedule(std::function<void()> callback, common::QWord cycle, std::string description) {
    if (!callback) {
        common::Logger::warning("Attempted to schedule event with null callback");
        return 0;
    }

    EventId id = m_nextId++;
    m_events.push_back(Event{id, cycle, std::move(callback), std::move(description)});
    return id;
}

bool EventScheduler::cancel(EventId eventId) {
    if (eventId == 0) {
        return false;
    }
    auto it = std::find_if(m_events.begin(), m_events.end(), [eventId](const Event& ev) {
        return ev.id == eventId;
    });
    if (it != m_events.end()) {
        m_events.erase(it);
        return true;
    }
    return false;
}

void EventScheduler::clear() {
    m_events.clear();
}

std::vector<Event> EventScheduler::pendingEvents() const {
    std::vector<Event> sorted = m_events;
    std::stable_sort(sorted.begin(), sorted.end(), [](const Event& a, const Event& b) {
        if (a.cycle != b.cycle) {
            return a.cycle < b.cycle;
        }
        return a.id < b.id;
    });
    return sorted;
}

std::size_t EventScheduler::pendingCount() const noexcept {
    return m_events.size();
}

std::size_t EventScheduler::executeReadyEvents(common::QWord currentCycle) {
    if (m_events.empty()) {
        return 0;
    }

    std::vector<Event> ready;
    std::vector<Event> remaining;
    ready.reserve(m_events.size());
    remaining.reserve(m_events.size());

    for (auto& ev : m_events) {
        if (ev.cycle <= currentCycle) {
            ready.push_back(std::move(ev));
        } else {
            remaining.push_back(std::move(ev));
        }
    }

    m_events = std::move(remaining);

    std::stable_sort(ready.begin(), ready.end(), [](const Event& a, const Event& b) {
        if (a.cycle != b.cycle) {
            return a.cycle < b.cycle;
        }
        return a.id < b.id;
    });

    std::size_t executed = 0;
    for (const auto& ev : ready) {
        if (ev.callback) {
            ev.callback();
            executed++;
        }
    }

    return executed;
}

} // namespace efs::system::scheduler
