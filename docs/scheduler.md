# Event & RTOS Scheduler Architecture

This document describes the scheduling architecture in the **Embedded Firmware Simulator (EFS)** platform.

## Overview

The simulator contains two distinct scheduling subsystems designed for different operational needs:
1. **Event Scheduler**: Deterministic hardware-level simulation event queue.
2. **RTOS Scheduler**: Priority-based task scheduling for simulated embedded applications.

---

## 1. Event Scheduler (`efs::system::scheduler::EventScheduler`)

The `EventScheduler` coordinates hardware simulation events (such as timer interrupts or DMA completion callbacks):
- Uses a priority queue sorted by target simulation cycle timestamp.
- Executes callbacks via `executeReadyEvents(currentCycle)`.
- Guarantees deterministic execution across compiler toolchains and host architectures.

---

## 2. RTOS Scheduler (`efs::rtos::RTOSScheduler`)

The `RTOSScheduler` provides a simulated Real-Time Operating System kernel.

### Task Control Block (TCB)
Each task is represented by `efs::rtos::TaskControlBlock`:
- `id`: Unique task identifier (`TaskId`).
- `priority`: Task priority (0 = Highest priority).
- `state`: Task state (`Ready`, `Running`, `Suspended`, `Blocked`, `Terminated`).
- `entryPoint`: Task execution callback std::function.

```
       +------------------------------------+
       |              Ready                 |
       +------------------------------------+
         |                                ^
         | schedule()                     | yield() / preempt()
         v                                |
       +------------------------------------+
       |             Running                |
       +------------------------------------+
         |                                |
         | suspendTask()                  | terminateTask()
         v                                v
       +------------------------------------+  +-------------------+
       |            Suspended               |  |    Terminated     |
       +------------------------------------+  +-------------------+
```

### Scheduling Policies
- **Priority-Based Preemption**: Highest priority ready task is scheduled for execution.
- **Round-Robin Time Slicing**: Equal-priority tasks share execution time slices.
- **Task Controls**: `createTask()`, `deleteTask()`, `suspendTask()`, `resumeTask()`, `yield()`.
