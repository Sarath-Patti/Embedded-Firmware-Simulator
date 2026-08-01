#include "drivers/dma/dma_controller.hpp"
#include "common/logger.hpp"

namespace efs::drivers::dma {

DMAController::DMAController(memory::Memory* memory,
                             mmio::MMIOBus* mmioBus,
                             kernel::InterruptController* interruptController,
                             drivers::uart::UART* uart)
    : m_memory(memory),
      m_mmioBus(mmioBus),
      m_interruptController(interruptController),
      m_uart(uart) {
}

void DMAController::attachMemory(memory::Memory* memory) noexcept {
    m_memory = memory;
}

void DMAController::attachMMIO(mmio::MMIOBus* mmioBus) noexcept {
    m_mmioBus = mmioBus;
}

void DMAController::attachInterruptController(kernel::InterruptController* controller, std::uint8_t interruptId) noexcept {
    m_interruptController = controller;
    m_interruptId = interruptId;
}

void DMAController::detachInterruptController() noexcept {
    m_interruptController = nullptr;
    m_interruptId = 0;
}

void DMAController::attachUART(drivers::uart::UART* uart) noexcept {
    m_uart = uart;
}

void DMAController::detachUART() noexcept {
    m_uart = nullptr;
}

void DMAController::attachScheduler(system::scheduler::EventScheduler* scheduler) noexcept {
    m_scheduler = scheduler;
}

void DMAController::detachScheduler() noexcept {
    m_scheduler = nullptr;
}

bool DMAController::configure(common::Address sourceAddress, common::Address destinationAddress, common::Size length) {
    if (m_busy) {
        common::Logger::warning("DMAController: Cannot configure while transfer is busy");
        return false;
    }

    m_sourceAddress = sourceAddress;
    m_destinationAddress = destinationAddress;
    m_length = length;
    m_currentSrc = sourceAddress;
    m_currentDst = destinationAddress;
    m_transferredBytes = 0;
    m_busy = false;
    m_completed = false;
    m_error = false;

    if (!m_userSetSrcIncrement) {
        bool isUartData = (m_uart != nullptr && sourceAddress == m_uart->dataAddress());
        bool isMmioReg = (m_mmioBus != nullptr && m_mmioBus->contains(sourceAddress));
        m_srcIncrement = !(isUartData || isMmioReg);
    }
    if (!m_userSetDstIncrement) {
        bool isUartData = (m_uart != nullptr && destinationAddress == m_uart->dataAddress());
        bool isMmioReg = (m_mmioBus != nullptr && m_mmioBus->contains(destinationAddress));
        m_dstIncrement = !(isUartData || isMmioReg);
    }

    return true;
}

void DMAController::setIncrements(bool sourceIncrement, bool destinationIncrement) noexcept {
    m_srcIncrement = sourceIncrement;
    m_dstIncrement = destinationIncrement;
    m_userSetSrcIncrement = true;
    m_userSetDstIncrement = true;
}

bool DMAController::start() {
    if (m_memory == nullptr && m_mmioBus == nullptr && m_uart == nullptr) {
        common::Logger::warning("DMAController: Attempted start without Memory, MMIO, or UART attached");
        m_error = true;
        return false;
    }

    if (m_length == 0) {
        m_busy = false;
        m_completed = true;
        m_error = false;
        if (m_interruptController != nullptr) {
            m_interruptController->trigger(m_interruptId);
        }
        return true;
    }

    bool srcValid = (m_uart != nullptr && m_sourceAddress == m_uart->dataAddress()) ||
                     (m_mmioBus != nullptr && m_mmioBus->contains(m_sourceAddress)) ||
                     (m_memory != nullptr && m_memory->isValidAddress(m_sourceAddress));
    bool dstValid = (m_uart != nullptr && m_destinationAddress == m_uart->dataAddress()) ||
                     (m_mmioBus != nullptr && m_mmioBus->contains(m_destinationAddress)) ||
                     (m_memory != nullptr && m_memory->isValidAddress(m_destinationAddress));

    if (!srcValid || !dstValid) {
        common::Logger::warning("DMAController: Start failed due to invalid source or destination address");
        m_error = true;
        m_busy = false;
        m_completed = false;
        return false;
    }

    m_currentSrc = m_sourceAddress;
    m_currentDst = m_destinationAddress;
    m_transferredBytes = 0;
    m_busy = true;
    m_completed = false;
    m_error = false;
    return true;
}

void DMAController::stop() {
    m_busy = false;
}

void DMAController::reset() {
    stop();
    m_sourceAddress = 0;
    m_destinationAddress = 0;
    m_length = 0;
    m_currentSrc = 0;
    m_currentDst = 0;
    m_transferredBytes = 0;
    m_srcIncrement = true;
    m_dstIncrement = true;
    m_userSetSrcIncrement = false;
    m_userSetDstIncrement = false;
    m_busy = false;
    m_completed = false;
    m_error = false;
}

void DMAController::step() {
    if (!m_busy) {
        return;
    }

    if (m_transferredBytes >= m_length) {
        m_busy = false;
        m_completed = true;
        return;
    }

    common::Byte val = 0;
    if (m_uart != nullptr && m_currentSrc == m_uart->dataAddress()) {
        val = m_uart->readByte();
    } else if (m_mmioBus != nullptr && m_mmioBus->contains(m_currentSrc)) {
        val = static_cast<common::Byte>(m_mmioBus->read(m_currentSrc) & 0xFF);
    } else if (m_memory != nullptr && m_memory->isValidAddress(m_currentSrc)) {
        val = m_memory->read(m_currentSrc);
    } else {
        common::Logger::error("DMAController: Memory/MMIO read out of bounds during step");
        m_error = true;
        m_busy = false;
        return;
    }

    if (m_uart != nullptr && m_currentDst == m_uart->dataAddress()) {
        m_uart->writeByte(val);
    } else if (m_mmioBus != nullptr && m_mmioBus->contains(m_currentDst)) {
        m_mmioBus->write(m_currentDst, static_cast<common::DWord>(val));
    } else if (m_memory != nullptr && m_memory->isValidAddress(m_currentDst)) {
        m_memory->write(m_currentDst, val);
    } else {
        common::Logger::error("DMAController: Memory/MMIO write out of bounds during step");
        m_error = true;
        m_busy = false;
        return;
    }

    m_transferredBytes++;
    if (m_srcIncrement) {
        m_currentSrc++;
    }
    if (m_dstIncrement) {
        m_currentDst++;
    }

    if (m_transferredBytes >= m_length) {
        m_busy = false;
        m_completed = true;
        if (m_interruptController != nullptr) {
            m_interruptController->trigger(m_interruptId);
        }
        if (m_scheduler != nullptr) {
            m_scheduler->schedule([]() {}, 0, "DMA Transfer Complete");
        }
    }
}

void DMAController::tick() {
    step();
}

bool DMAController::busy() const noexcept {
    return m_busy;
}

bool DMAController::completed() const noexcept {
    return m_completed;
}

bool DMAController::hasError() const noexcept {
    return m_error;
}

common::Address DMAController::sourceAddress() const noexcept {
    return m_sourceAddress;
}

common::Address DMAController::destinationAddress() const noexcept {
    return m_destinationAddress;
}

common::Size DMAController::length() const noexcept {
    return m_length;
}

common::Size DMAController::transferredBytes() const noexcept {
    return m_transferredBytes;
}

std::uint8_t DMAController::interruptId() const noexcept {
    return m_interruptId;
}

bool DMAController::sourceIncrement() const noexcept {
    return m_srcIncrement;
}

bool DMAController::destinationIncrement() const noexcept {
    return m_dstIncrement;
}

drivers::uart::UART* DMAController::uart() const noexcept {
    return m_uart;
}

} // namespace efs::drivers::dma
