#include "drivers/spi/spi_controller.hpp"
#include "drivers/spi/spi_device.hpp"
#include "hal/spi_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    std::cout << "Starting Serial Peripheral Interface (SPI) Demo...\n";

    // 1. Initialize MMIO Bus and SystemBus
    efs::mmio::MMIOBus mmioBus;
    efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

    constexpr efs::common::Address SPI_BASE_ADDR = 0x40004000U;

    // 2. Instantiate SPI Controller peripheral and attach to SystemBus
    efs::drivers::spi::SPIController spi(mmioBus, SPI_BASE_ADDR, 4 /* clock divider */, 0 /* mode 0 */);
    systemBus.attachSPI(&spi);

    // 3. Create simulated SPI slave device (e.g. EEPROM / Flash Memory Chip simulator)
    efs::drivers::spi::SimulatedSPIDevice slaveDevice;
    spi.attachSlave(&slaveDevice);

    // 4. Attach SPI HAL for firmware access
    efs::hal::SPIHAL spiHAL(&spi);
    spiHAL.enable();
    spiHAL.configure(0 /* Mode 0 */, 8 /* Divider 8 */);

    std::cout << "SPI Peripheral enabled.\n";
    std::cout << "  Base Address:  0x" << std::hex << SPI_BASE_ADDR << std::dec << "\n";
    std::cout << "  Clock Divider: " << spi.clockDivider() << "\n";
    std::cout << "  SPI Mode:      " << static_cast<int>(spi.mode()) << "\n\n";

    // 5. Perform Full-Duplex Command-Response Transfer with SPI Slave Device
    // Simulate SPI FLASH Read Identification Command (0x9F) returning Manufacturer ID 0xC2, Device ID 0x20
    slaveDevice.queueResponseByte(0x00); // Dummy response during command byte
    slaveDevice.queueResponseByte(0xC2); // Manufacturer ID
    slaveDevice.queueResponseByte(0x20); // Device ID

    const std::vector<std::uint8_t> txCmd = {0x9F, 0x00, 0x00};
    std::vector<std::uint8_t> rxData;

    std::cout << "--- Demo 1: Full-Duplex SPI Command Transmission ---\n";
    for (std::uint8_t txByte : txCmd) {
        std::uint8_t rxByte = spiHAL.transfer(txByte);
        rxData.push_back(rxByte);
        std::cout << "  TX: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(txByte)
                  << " --> RX: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxByte) << "\n";
    }

    std::cout << "\nSlave Device Received Payload: ";
    for (std::uint8_t b : slaveDevice.receivedBytes()) {
        std::cout << "0x" << std::hex << std::setw(2) << static_cast<int>(b) << " ";
    }
    std::cout << "\n";

    // 6. Demonstrate Multi-byte Firmware Stream Write/Read via HAL
    std::cout << "\n--- Demo 2: Firmware Stream Buffer Transfer ---\n";
    slaveDevice.clearReceivedBytes();
    slaveDevice.clearResponseQueue();
    slaveDevice.setDefaultResponseByte(0xAA);

    const std::vector<std::uint8_t> streamData = {0x01, 0x02, 0x03, 0x04};
    for (std::uint8_t b : streamData) {
        spiHAL.writeByte(b);
    }

    std::cout << "Firmware read back received responses from SPI HAL:\n";
    while (spi.hasData()) {
        std::uint8_t r = spiHAL.readByte();
        std::cout << "  Received byte: 0x" << std::hex << std::setw(2) << static_cast<int>(r) << "\n";
    }

    // 7. SystemBus Reset Verification
    std::cout << "\nResetting SystemBus...\n";
    systemBus.reset();
    std::cout << "SPI Enabled after reset? " << (spiHAL.enabled() ? "Yes" : "No") << "\n";

    std::cout << "\nSPI Controller Demo completed successfully.\n";
    return 0;
}
