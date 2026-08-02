#include "rtos/task.hpp"
#include "common/logger.hpp"

namespace efs::rtos {

Task::Task(TaskId id,
           std::string name,
           TaskFunction function,
           std::uint8_t priority,
           std::size_t stackSize)
    : m_tcb{id, std::move(name), std::move(function), priority, TaskState::READY, 0, stackSize, 0} {
}

TaskId Task::id() const noexcept {
    return m_tcb.id;
}

const std::string& Task::name() const noexcept {
    return m_tcb.name;
}

std::uint8_t Task::priority() const noexcept {
    return m_tcb.priority;
}

void Task::setPriority(std::uint8_t priority) noexcept {
    m_tcb.priority = priority;
}

TaskState Task::state() const noexcept {
    return m_tcb.state;
}

void Task::setState(TaskState state) noexcept {
    m_tcb.state = state;
}

std::uint64_t Task::delayUntilTick() const noexcept {
    return m_tcb.delayUntilTick;
}

void Task::setDelayUntilTick(std::uint64_t tick) noexcept {
    m_tcb.delayUntilTick = tick;
}

std::size_t Task::stackSize() const noexcept {
    return m_tcb.stackSize;
}

std::size_t Task::executionCount() const noexcept {
    return m_tcb.executionCount;
}

void Task::incrementExecutionCount() noexcept {
    m_tcb.executionCount++;
}

void Task::execute() {
    if (m_tcb.function) {
        m_tcb.executionCount++;
        m_tcb.function();
    }
}

const TaskControlBlock& Task::tcb() const noexcept {
    return m_tcb;
}

TaskControlBlock& Task::tcb() noexcept {
    return m_tcb;
}

} // namespace efs::rtos
