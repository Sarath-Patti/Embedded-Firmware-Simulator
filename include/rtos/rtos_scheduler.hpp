#ifndef EFS_RTOS_RTOS_SCHEDULER_HPP
#define EFS_RTOS_RTOS_SCHEDULER_HPP

#include "rtos/task.hpp"
#include "system/system_bus.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace efs::rtos {

/// Simulated RTOS Task Scheduler supporting priority scheduling, round-robin, delay queue, and idle task.
class Scheduler {
public:
    static constexpr std::uint8_t IDLE_TASK_PRIORITY = 0;
    static constexpr TaskId IDLE_TASK_ID = 0;

    explicit Scheduler(system::SystemBus* systemBus = nullptr);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

    /// Attaches or updates SystemBus pointer.
    void attachSystemBus(system::SystemBus* systemBus) noexcept;

    /// Creates and registers a new task. Returns non-zero TaskId, or 0 on failure.
    TaskId createTask(const std::string& name,
                       TaskFunction function,
                       std::uint8_t priority = 1,
                       std::size_t stackSize = 1024);

    /// Deletes task by TaskId. Returns true if task was found and deleted.
    bool deleteTask(TaskId id);

    /// Starts RTOS scheduler and creates Idle Task if not present.
    void start();

    /// Stops RTOS scheduler.
    void stop();

    /// Returns true if scheduler is active and running.
    [[nodiscard]] bool running() const noexcept;

    /// Selects and executes the next highest-priority ready task slice.
    void schedule();

    /// Yields execution of current task and triggers scheduling.
    void yield();

    /// Blocks current running task for specified ticks.
    void delay(std::uint64_t ticks);

    /// Advances RTOS tick count by 1 and wakes up unblocked tasks from delay queue.
    void tick();

    /// Returns current tick count.
    [[nodiscard]] std::uint64_t currentTick() const noexcept;

    /// Suspends task by TaskId.
    bool suspendTask(TaskId id);

    /// Resumes suspended task by TaskId.
    bool resumeTask(TaskId id);

    /// Returns pointer to currently running TaskControlBlock or nullptr.
    [[nodiscard]] TaskControlBlock* currentTask() const noexcept;

    /// Returns pointer to TaskControlBlock for a given TaskId or nullptr.
    [[nodiscard]] TaskControlBlock* getTask(TaskId id) const noexcept;

    /// Returns total count of active tasks (including Idle Task).
    [[nodiscard]] std::size_t taskCount() const noexcept;

    /// Resets scheduler state, tasks, queues, and tick count.
    void reset();

private:
    void ensureIdleTask();
    void addToReadyQueue(TaskId id);
    void removeFromReadyQueue(TaskId id);
    void processDelayQueue();
    TaskId pickNextTask();

    system::SystemBus* m_systemBus{nullptr};
    bool m_running{false};
    std::uint64_t m_tickCount{0};
    TaskId m_nextTaskId{1};
    TaskId m_currentTaskId{IDLE_TASK_ID};

    std::unordered_map<TaskId, std::unique_ptr<Task>> m_tasks;
    std::map<std::uint8_t, std::vector<TaskId>, std::greater<std::uint8_t>> m_readyQueue;
    std::vector<TaskId> m_delayQueue;
};

} // namespace efs::rtos

#endif // EFS_RTOS_RTOS_SCHEDULER_HPP
