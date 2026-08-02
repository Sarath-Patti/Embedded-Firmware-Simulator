#include "rtos/rtos_scheduler.hpp"
#include "common/logger.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>

namespace efs::rtos {

Scheduler::Scheduler(system::SystemBus* systemBus)
    : m_systemBus(systemBus) {
    ensureIdleTask();
}

Scheduler::~Scheduler() {
    stop();
    m_tasks.clear();
    m_readyQueue.clear();
    m_delayQueue.clear();
}

void Scheduler::attachSystemBus(system::SystemBus* systemBus) noexcept {
    m_systemBus = systemBus;
}

void Scheduler::ensureIdleTask() {
    if (m_tasks.find(IDLE_TASK_ID) == m_tasks.end()) {
        auto idleTask = std::make_unique<Task>(IDLE_TASK_ID,
                                               "IdleTask",
                                               []() {
                                                   // Low-power idle loop operation
                                               },
                                               IDLE_TASK_PRIORITY,
                                               512);
        m_tasks[IDLE_TASK_ID] = std::move(idleTask);
        addToReadyQueue(IDLE_TASK_ID);
    }
}

TaskId Scheduler::createTask(const std::string& name,
                             TaskFunction function,
                             std::uint8_t priority,
                             std::size_t stackSize) {
    if (!function) {
        common::Logger::error("Attempted to create task with null function");
        throw std::invalid_argument("Task function cannot be null");
    }

    TaskId id = m_nextTaskId++;
    auto task = std::make_unique<Task>(id, name, std::move(function), priority, stackSize);
    m_tasks[id] = std::move(task);
    addToReadyQueue(id);
    return id;
}

bool Scheduler::deleteTask(TaskId id) {
    if (id == IDLE_TASK_ID) {
        common::Logger::warning("Cannot delete RTOS Idle Task");
        return false;
    }

    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
        common::Logger::warning("Attempted to delete non-existent task ID: " + std::to_string(id));
        return false;
    }

    removeFromReadyQueue(id);

    auto delayIt = std::find(m_delayQueue.begin(), m_delayQueue.end(), id);
    if (delayIt != m_delayQueue.end()) {
        m_delayQueue.erase(delayIt);
    }

    it->second->setState(TaskState::TERMINATED);
    m_tasks.erase(it);

    if (m_currentTaskId == id) {
        m_currentTaskId = IDLE_TASK_ID;
        schedule();
    }

    return true;
}

void Scheduler::start() {
    ensureIdleTask();
    m_running = true;
}

void Scheduler::stop() {
    m_running = false;
}

bool Scheduler::running() const noexcept {
    return m_running;
}

void Scheduler::schedule() {
    if (!m_running) {
        return;
    }

    processDelayQueue();
    TaskId nextId = pickNextTask();

    if (m_currentTaskId != nextId && m_tasks.count(m_currentTaskId)) {
        Task* current = m_tasks[m_currentTaskId].get();
        if (current != nullptr && current->state() == TaskState::RUNNING) {
            current->setState(TaskState::READY);
        }
    }

    m_currentTaskId = nextId;
    Task* nextTask = m_tasks[nextId].get();
    if (nextTask != nullptr &&
        (nextTask->state() == TaskState::READY || nextTask->state() == TaskState::RUNNING)) {
        nextTask->setState(TaskState::RUNNING);
        nextTask->execute();
    }
}

void Scheduler::yield() {
    if (!m_running) {
        return;
    }
    if (m_currentTaskId != IDLE_TASK_ID && m_tasks.count(m_currentTaskId)) {
        Task* current = m_tasks[m_currentTaskId].get();
        if (current != nullptr && current->state() == TaskState::RUNNING) {
            current->setState(TaskState::READY);
            // Move current task to back of ready queue for round-robin
            removeFromReadyQueue(m_currentTaskId);
            addToReadyQueue(m_currentTaskId);
        }
    }
    schedule();
}

void Scheduler::delay(std::uint64_t ticks) {
    if (!m_running || m_currentTaskId == IDLE_TASK_ID) {
        return;
    }

    auto it = m_tasks.find(m_currentTaskId);
    if (it != m_tasks.end()) {
        Task* task = it->second.get();
        task->setState(TaskState::BLOCKED);
        task->setDelayUntilTick(m_tickCount + ticks);
        removeFromReadyQueue(m_currentTaskId);
        m_delayQueue.push_back(m_currentTaskId);
    }

    schedule();
}

void Scheduler::tick() {
    m_tickCount++;
    processDelayQueue();
}

std::uint64_t Scheduler::currentTick() const noexcept {
    return m_tickCount;
}

bool Scheduler::suspendTask(TaskId id) {
    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
        return false;
    }

    Task* task = it->second.get();
    task->setState(TaskState::SUSPENDED);
    removeFromReadyQueue(id);

    auto delayIt = std::find(m_delayQueue.begin(), m_delayQueue.end(), id);
    if (delayIt != m_delayQueue.end()) {
        m_delayQueue.erase(delayIt);
    }

    if (id == m_currentTaskId && m_running) {
        schedule();
    }

    return true;
}

bool Scheduler::resumeTask(TaskId id) {
    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
        return false;
    }

    Task* task = it->second.get();
    if (task->state() == TaskState::SUSPENDED) {
        task->setState(TaskState::READY);
        addToReadyQueue(id);
        return true;
    }
    return false;
}

TaskControlBlock* Scheduler::currentTask() const noexcept {
    auto it = m_tasks.find(m_currentTaskId);
    if (it != m_tasks.end()) {
        return &it->second->tcb();
    }
    return nullptr;
}

TaskControlBlock* Scheduler::getTask(TaskId id) const noexcept {
    auto it = m_tasks.find(id);
    if (it != m_tasks.end()) {
        return &it->second->tcb();
    }
    return nullptr;
}

std::size_t Scheduler::taskCount() const noexcept {
    return m_tasks.size();
}

void Scheduler::reset() {
    stop();
    m_tasks.clear();
    m_readyQueue.clear();
    m_delayQueue.clear();
    m_tickCount = 0;
    m_nextTaskId = 1;
    m_currentTaskId = IDLE_TASK_ID;
    ensureIdleTask();
}

void Scheduler::addToReadyQueue(TaskId id) {
    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
        return;
    }

    std::uint8_t priority = it->second->priority();
    auto& vec = m_readyQueue[priority];
    if (std::find(vec.begin(), vec.end(), id) == vec.end()) {
        vec.push_back(id);
    }
}

void Scheduler::removeFromReadyQueue(TaskId id) {
    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
        return;
    }

    std::uint8_t priority = it->second->priority();
    auto mapIt = m_readyQueue.find(priority);
    if (mapIt != m_readyQueue.end()) {
        auto vecIt = std::find(mapIt->second.begin(), mapIt->second.end(), id);
        if (vecIt != mapIt->second.end()) {
            mapIt->second.erase(vecIt);
        }
        if (mapIt->second.empty()) {
            m_readyQueue.erase(mapIt);
        }
    }
}

void Scheduler::processDelayQueue() {
    for (auto it = m_delayQueue.begin(); it != m_delayQueue.end(); ) {
        TaskId id = *it;
        auto taskIt = m_tasks.find(id);
        if (taskIt != m_tasks.end() && taskIt->second->delayUntilTick() <= m_tickCount) {
            taskIt->second->setState(TaskState::READY);
            addToReadyQueue(id);
            it = m_delayQueue.erase(it);
        } else {
            ++it;
        }
    }
}

TaskId Scheduler::pickNextTask() {
    for (auto& [priority, vec] : m_readyQueue) {
        if (!vec.empty()) {
            TaskId id = vec.front();
            // Move front element to back for round-robin scheduling among equal priorities
            if (vec.size() > 1) {
                vec.erase(vec.begin());
                vec.push_back(id);
            }
            return id;
        }
    }
    return IDLE_TASK_ID;
}

} // namespace efs::rtos
