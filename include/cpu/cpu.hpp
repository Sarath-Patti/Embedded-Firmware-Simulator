#ifndef EFS_CPU_CPU_HPP
#define EFS_CPU_CPU_HPP

#include "common/types.hpp"
#include "drivers/timer/timer.hpp"
#include "kernel/interrupt_controller.hpp"
#include <vector>

namespace efs::cpu {

class CPU {
public:
    explicit CPU(kernel::InterruptController* interruptController = nullptr);
    ~CPU() = default;

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;
    CPU(CPU&&) = delete;
    CPU& operator=(CPU&&) = delete;

    void start();
    void stop();
    void reset();
    void step();
    void run(common::QWord cycles);

    bool attachTimer(drivers::timer::Timer* timer);
    bool detachTimer(drivers::timer::Timer* timer);

    void setInterruptController(kernel::InterruptController* controller);
    [[nodiscard]] kernel::InterruptController* interruptController() const noexcept;

    [[nodiscard]] common::QWord cycleCount() const noexcept;
    [[nodiscard]] bool running() const noexcept;

private:
    common::QWord m_cycleCount{0};
    bool m_running{false};
    kernel::InterruptController* m_interruptController{nullptr};
    std::vector<drivers::timer::Timer*> m_timers;
};

} // namespace efs::cpu

#endif // EFS_CPU_CPU_HPP
