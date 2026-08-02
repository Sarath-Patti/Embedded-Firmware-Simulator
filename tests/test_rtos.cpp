#include "cpu/cpu.hpp"
#include "rtos/rtos_scheduler.hpp"
#include "rtos/task.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace efs::rtos;
using namespace efs::cpu;

void test_rtos_task_creation() {
    Scheduler scheduler;
    assert(scheduler.taskCount() == 1); // Idle task

    int runCount = 0;
    TaskId t1 = scheduler.createTask("Task1", [&runCount]() { runCount++; }, 1);
    if (t1 == 0) {
        throw std::runtime_error("Task creation failed");
    }
    assert(t1 > 0);
    assert(scheduler.taskCount() == 2);

    TaskControlBlock* tcb = scheduler.getTask(t1);
    if (tcb == nullptr) {
        throw std::runtime_error("Task lookup failed");
    }
    assert(tcb != nullptr);
    assert(tcb->id == t1);
    assert(tcb->name == "Task1");
    assert(tcb->priority == 1);
    assert(tcb->state == TaskState::READY);

    std::cout << "[PASS] test_rtos_task_creation\n";
}

void test_rtos_task_deletion() {
    Scheduler scheduler;
    TaskId t1 = scheduler.createTask("Task1", []() {}, 1);
    if (t1 == 0) {
        throw std::runtime_error("Task creation failed");
    }
    assert(scheduler.taskCount() == 2);

    bool deleted = scheduler.deleteTask(t1);
    if (!deleted) {
        throw std::runtime_error("Task deletion failed");
    }
    assert(deleted);
    assert(scheduler.taskCount() == 1);
    assert(scheduler.getTask(t1) == nullptr);

    // Deleting non-existent task
    assert(!scheduler.deleteTask(999));

    // Deleting Idle Task (not allowed)
    assert(!scheduler.deleteTask(Scheduler::IDLE_TASK_ID));

    std::cout << "[PASS] test_rtos_task_deletion\n";
}

void test_rtos_start_stop() {
    Scheduler scheduler;
    assert(!scheduler.running());

    scheduler.start();
    assert(scheduler.running());

    scheduler.stop();
    assert(!scheduler.running());

    std::cout << "[PASS] test_rtos_start_stop\n";
}

void test_rtos_priority_scheduling() {
    Scheduler scheduler;
    scheduler.start();

    std::vector<std::string> executionOrder;

    TaskId lowId = scheduler.createTask("LowPriority", [&executionOrder]() {
        executionOrder.push_back("Low");
    }, 1);
    if (lowId == 0) {
        throw std::runtime_error("Low priority task creation failed");
    }

    TaskId highId = scheduler.createTask("HighPriority", [&executionOrder]() {
        executionOrder.push_back("High");
    }, 10);
    if (highId == 0) {
        throw std::runtime_error("High priority task creation failed");
    }

    // Higher priority task should run first
    scheduler.schedule();
    assert(!executionOrder.empty());
    assert(executionOrder.front() == "High");

    std::cout << "[PASS] test_rtos_priority_scheduling\n";
}

void test_rtos_round_robin_scheduling() {
    Scheduler scheduler;
    scheduler.start();

    std::vector<std::string> log;

    TaskId tA = scheduler.createTask("TaskA", [&log]() { log.push_back("A"); }, 5);
    if (tA == 0) {
        throw std::runtime_error("TaskA creation failed");
    }

    TaskId tB = scheduler.createTask("TaskB", [&log]() { log.push_back("B"); }, 5);
    if (tB == 0) {
        throw std::runtime_error("TaskB creation failed");
    }

    // Schedule 4 cycles -> round robin between A and B
    for (int i = 0; i < 4; ++i) {
        scheduler.schedule();
    }

    assert(log.size() == 4);
    assert(log[0] == "A");
    assert(log[1] == "B");
    assert(log[2] == "A");
    assert(log[3] == "B");

    std::cout << "[PASS] test_rtos_round_robin_scheduling\n";
}

void test_rtos_delay_wakeup() {
    Scheduler scheduler;
    scheduler.start();

    bool taskRan = false;
    TaskId t1 = scheduler.createTask("DelayedTask", [&scheduler, &taskRan]() {
        taskRan = true;
        scheduler.delay(3);
    }, 5);
    if (t1 == 0) {
        throw std::runtime_error("Delayed task creation failed");
    }

    // Initial schedule runs task and puts it into BLOCKED delay queue for 3 ticks
    scheduler.schedule();
    assert(taskRan);

    TaskControlBlock* tcb = scheduler.getTask(t1);
    if (tcb == nullptr) {
        throw std::runtime_error("Delayed task lookup failed");
    }
    assert(tcb != nullptr);
    assert(tcb->id == t1);
    assert(tcb->state == TaskState::BLOCKED);

    // Tick 1
    scheduler.tick();
    assert(tcb->state == TaskState::BLOCKED);

    // Tick 2
    scheduler.tick();
    assert(tcb->state == TaskState::BLOCKED);

    // Tick 3 -> wake tick reached -> transitions back to READY
    scheduler.tick();
    assert(tcb->state == TaskState::READY);

    std::cout << "[PASS] test_rtos_delay_wakeup\n";
}

void test_rtos_idle_task() {
    Scheduler scheduler;
    scheduler.start();

    // With no user tasks, scheduler runs Idle Task
    scheduler.schedule();
    TaskControlBlock* cur = scheduler.currentTask();
    if (cur == nullptr) {
        throw std::runtime_error("Idle task lookup failed");
    }
    assert(cur != nullptr);
    assert(cur->id == Scheduler::IDLE_TASK_ID);
    assert(cur->priority == Scheduler::IDLE_TASK_PRIORITY);
    assert(scheduler.getTask(Scheduler::IDLE_TASK_ID) == cur);

    std::cout << "[PASS] test_rtos_idle_task\n";
}

void test_rtos_state_transitions() {
    Scheduler scheduler;
    scheduler.start();

    TaskId t1 = scheduler.createTask("StateTask", []() {}, 5);
    if (t1 == 0) {
        throw std::runtime_error("StateTask creation failed");
    }

    TaskControlBlock* tcb = scheduler.getTask(t1);
    if (tcb == nullptr) {
        throw std::runtime_error("StateTask lookup failed");
    }
    assert(tcb != nullptr);
    assert(tcb->id == t1);
    assert(tcb->state == TaskState::READY);

    bool suspended = scheduler.suspendTask(t1);
    if (!suspended) {
        throw std::runtime_error("Task suspension failed");
    }
    assert(suspended);
    assert(tcb->state == TaskState::SUSPENDED);

    bool resumed = scheduler.resumeTask(t1);
    if (!resumed) {
        throw std::runtime_error("Task resumption failed");
    }
    assert(resumed);
    assert(tcb->state == TaskState::READY);

    std::cout << "[PASS] test_rtos_state_transitions\n";
}

void test_rtos_cpu_integration() {
    CPU cpu;
    Scheduler scheduler(cpu.systemBus());

    cpu.attachRTOSScheduler(&scheduler);
    assert(cpu.rtosScheduler() == &scheduler);

    int count = 0;
    TaskId t1 = scheduler.createTask("CPUTask", [&count]() { count++; }, 2);
    if (t1 == 0) {
        throw std::runtime_error("CPUTask creation failed");
    }

    cpu.start();
    assert(scheduler.running());

    cpu.step();
    assert(count > 0);

    cpu.stop();
    assert(!scheduler.running());

    std::cout << "[PASS] test_rtos_cpu_integration\n";
}

int main() {
    std::cout << "Running RTOS Task Scheduler unit tests...\n";
    test_rtos_task_creation();
    test_rtos_task_deletion();
    test_rtos_start_stop();
    test_rtos_priority_scheduling();
    test_rtos_round_robin_scheduling();
    test_rtos_delay_wakeup();
    test_rtos_idle_task();
    test_rtos_state_transitions();
    test_rtos_cpu_integration();
    std::cout << "All RTOS Task Scheduler unit tests passed successfully.\n";
    return 0;
}
