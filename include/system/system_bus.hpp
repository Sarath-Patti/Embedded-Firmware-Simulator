#ifndef EFS_SYSTEM_SYSTEM_BUS_HPP
#define EFS_SYSTEM_SYSTEM_BUS_HPP

#include "drivers/adc/adc_controller.hpp"
#include "drivers/dma/dma_controller.hpp"
#include "drivers/gpio/gpio.hpp"
#include "drivers/i2c/i2c_controller.hpp"
#include "drivers/pwm/pwm_controller.hpp"
#include "drivers/spi/spi_controller.hpp"
#include "drivers/timer/timer.hpp"
#include "drivers/uart/uart.hpp"
#include "kernel/interrupt_controller.hpp"
#include "memory/memory.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/clock/simulation_clock.hpp"
#include "system/scheduler/event_scheduler.hpp"
#include <vector>

namespace efs::system {

/// Central System Bus connecting Memory, MMIO Bus, Interrupt Controller, Simulation Clock, Event Scheduler, DMA, and Peripherals.
class SystemBus {
public:
    SystemBus(memory::Memory* memory = nullptr,
              mmio::MMIOBus* mmioBus = nullptr,
              kernel::InterruptController* interruptController = nullptr);
    ~SystemBus();

    SystemBus(const SystemBus&) = delete;
    SystemBus& operator=(const SystemBus&) = delete;
    SystemBus(SystemBus&&) noexcept = default;
    SystemBus& operator=(SystemBus&&) noexcept = default;

    /// Accesses the central Simulation Clock (mutable).
    [[nodiscard]] clock::SimulationClock& clock() noexcept;

    /// Accesses the central Simulation Clock (read-only).
    [[nodiscard]] const clock::SimulationClock& clock() const noexcept;

    /// Accesses the central Event Scheduler (mutable).
    [[nodiscard]] scheduler::EventScheduler& scheduler() noexcept;

    /// Accesses the central Event Scheduler (read-only).
    [[nodiscard]] const scheduler::EventScheduler& scheduler() const noexcept;

    /// Sets or updates the Memory subsystem pointer.
    void setMemory(memory::Memory* memory) noexcept;

    /// Returns attached Memory pointer.
    [[nodiscard]] memory::Memory* memory() const noexcept;

    /// Sets or updates the MMIO Bus subsystem pointer.
    void setMMIO(mmio::MMIOBus* mmioBus) noexcept;

    /// Returns attached MMIO Bus pointer.
    [[nodiscard]] mmio::MMIOBus* mmio() const noexcept;

    /// Sets or updates the Interrupt Controller pointer.
    void setInterrupts(kernel::InterruptController* interruptController) noexcept;

    /// Returns attached Interrupt Controller pointer.
    [[nodiscard]] kernel::InterruptController* interrupts() const noexcept;

    /// Attaches DMA Controller to SystemBus.
    bool attachDMA(drivers::dma::DMAController* dma);

    /// Detaches DMA Controller from SystemBus.
    bool detachDMA(drivers::dma::DMAController* dma);

    /// Returns pointer to attached DMA Controller (first attached) or nullptr.
    [[nodiscard]] drivers::dma::DMAController* dma() const noexcept;

    /// Returns all attached DMA controllers.
    [[nodiscard]] const std::vector<drivers::dma::DMAController*>& dmas() const noexcept;

    /// Attaches Timer to SystemBus.
    bool attachTimer(drivers::timer::Timer* timer);

    /// Detaches Timer from SystemBus.
    bool detachTimer(drivers::timer::Timer* timer);

    /// Returns all attached timers.
    [[nodiscard]] const std::vector<drivers::timer::Timer*>& timers() const noexcept;

    /// Advances the Simulation Clock and ticks all attached DMA controllers, timers, and PWM controllers.
    void tickTimers();

    /// Attaches GPIO peripheral.
    void attachGPIO(drivers::gpio::GPIO* gpio) noexcept;

    /// Returns attached GPIO peripheral pointer.
    [[nodiscard]] drivers::gpio::GPIO* gpio() const noexcept;

    /// Attaches UART peripheral.
    void attachUART(drivers::uart::UART* uart) noexcept;

    /// Returns attached UART peripheral pointer.
    [[nodiscard]] drivers::uart::UART* uart() const noexcept;

    /// Attaches SPI peripheral.
    void attachSPI(drivers::spi::SPIController* spi) noexcept;

    /// Returns attached SPI peripheral pointer.
    [[nodiscard]] drivers::spi::SPIController* spi() const noexcept;

    /// Attaches I2C peripheral.
    void attachI2C(drivers::i2c::I2CController* i2c) noexcept;

    /// Returns attached I2C peripheral pointer.
    [[nodiscard]] drivers::i2c::I2CController* i2c() const noexcept;

    /// Attaches PWM peripheral.
    void attachPWM(drivers::pwm::PWMController* pwm) noexcept;

    /// Returns attached PWM peripheral pointer.
    [[nodiscard]] drivers::pwm::PWMController* pwm() const noexcept;

    /// Attaches ADC peripheral.
    void attachADC(drivers::adc::ADCController* adc) noexcept;

    /// Returns attached ADC peripheral pointer.
    [[nodiscard]] drivers::adc::ADCController* adc() const noexcept;

    /// Resets all attached peripherals (DMA, GPIO, Timers, UART, SPI, I2C, PWM, ADC, Interrupt Controller, Memory, Clock).
    void reset();

private:
    memory::Memory* m_memory{nullptr};
    mmio::MMIOBus* m_mmioBus{nullptr};
    kernel::InterruptController* m_interruptController{nullptr};
    clock::SimulationClock m_clock;
    scheduler::EventScheduler m_scheduler;
    std::vector<drivers::dma::DMAController*> m_dmas;
    std::vector<drivers::timer::Timer*> m_timers;
    drivers::gpio::GPIO* m_gpio{nullptr};
    drivers::uart::UART* m_uart{nullptr};
    drivers::spi::SPIController* m_spi{nullptr};
    drivers::i2c::I2CController* m_i2c{nullptr};
    drivers::pwm::PWMController* m_pwm{nullptr};
    drivers::adc::ADCController* m_adc{nullptr};
};

} // namespace efs::system

#endif // EFS_SYSTEM_SYSTEM_BUS_HPP
