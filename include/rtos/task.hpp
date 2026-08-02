#ifndef EFS_RTOS_TASK_HPP
#define EFS_RTOS_TASK_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace efs::rtos {

using TaskId = std::uint32_t;
using TaskFunction = std::function<void()>;

enum class TaskState {
    READY,
    RUNNING,
    BLOCKED,
    SUSPENDED,
    TERMINATED
};

struct TaskControlBlock {
    TaskId id{0};
    std::string name;
    TaskFunction function;
    std::uint8_t priority{1};
    TaskState state{TaskState::READY};
    std::uint64_t delayUntilTick{0};
    std::size_t stackSize{1024};
    std::size_t executionCount{0};
};

/// Task model holding execution function, priority, state, and metadata.
class Task {
public:
    Task(TaskId id,
         std::string name,
         TaskFunction function,
         std::uint8_t priority = 1,
         std::size_t stackSize = 1024);
    ~Task() = default;

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&&) noexcept = default;
    Task& operator=(Task&&) noexcept = default;

    [[nodiscard]] TaskId id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] std::uint8_t priority() const noexcept;
    void setPriority(std::uint8_t priority) noexcept;

    [[nodiscard]] TaskState state() const noexcept;
    void setState(TaskState state) noexcept;

    [[nodiscard]] std::uint64_t delayUntilTick() const noexcept;
    void setDelayUntilTick(std::uint64_t tick) noexcept;

    [[nodiscard]] std::size_t stackSize() const noexcept;
    [[nodiscard]] std::size_t executionCount() const noexcept;
    void incrementExecutionCount() noexcept;

    void execute();

    [[nodiscard]] const TaskControlBlock& tcb() const noexcept;
    [[nodiscard]] TaskControlBlock& tcb() noexcept;

private:
    TaskControlBlock m_tcb;
};

} // namespace efs::rtos

#endif // EFS_RTOS_TASK_HPP
