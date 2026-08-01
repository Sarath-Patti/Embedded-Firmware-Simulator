#include "drivers/i2c/i2c_controller.hpp"
#include "drivers/i2c/i2c_device.hpp"
#include "hal/i2c_hal.hpp"
#include "mmio/mmio_bus.hpp"
#include "system/system_bus.hpp"
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    std::cout << "Starting Inter-Integrated Circuit (I2C) Demo...\n";

    // 1. Initialize MMIO Bus and SystemBus
    efs::mmio::MMIOBus mmioBus;
    efs::system::SystemBus systemBus(nullptr, &mmioBus, nullptr);

    constexpr efs::common::Address I2C_BASE_ADDR = 0x40005000U;

    // 2. Instantiate I2C Controller peripheral and attach to SystemBus
    efs::drivers::i2c::I2CController i2c(mmioBus, I2C_BASE_ADDR);
    systemBus.attachI2C(&i2c);

    // 3. Create simulated I2C slave devices (e.g. MPU6050 IMU Sensor at 0x68 and EEPROM at 0x50)
    efs::drivers::i2c::SimulatedI2CDevice imuSensor(0x68);
    imuSensor.queueResponseByte(0x68); // WHO_AM_I response
    i2c.attachDevice(&imuSensor);

    efs::drivers::i2c::SimulatedI2CDevice eeprom(0x50);
    i2c.attachDevice(&eeprom);

    // 4. Attach I2C HAL for firmware access
    efs::hal::I2CHAL i2cHAL(&i2c);
    i2cHAL.enable();

    std::cout << "I2C Peripheral enabled.\n";
    std::cout << "  Base Address: 0x" << std::hex << I2C_BASE_ADDR << std::dec << "\n";
    std::cout << "  Slaves Attached: IMU Sensor (0x68), EEPROM (0x50)\n\n";

    // 5. Demo 1: I2C Master Write Transaction to EEPROM (Address 0x50)
    std::cout << "--- Demo 1: I2C Master Write Transaction (EEPROM) ---\n";
    bool txOk = i2cHAL.beginTransmission(0x50);
    std::cout << "  START Address 0x50 ACK? " << (txOk ? "Yes (ACK)" : "No (NACK)") << "\n";

    i2cHAL.writeByte(0x00); // Memory address high
    i2cHAL.writeByte(0x10); // Memory address low
    i2cHAL.writeByte(0xDE); // Data payload byte 1
    i2cHAL.writeByte(0xAD); // Data payload byte 2

    bool endOk = i2cHAL.endTransmission();
    std::cout << "  STOP Transaction Completed. ACK? " << (endOk ? "Yes" : "No") << "\n";
    std::cout << "  EEPROM Received Payload: ";
    for (std::uint8_t b : eeprom.receivedBytes()) {
        std::cout << "0x" << std::hex << std::setw(2) << static_cast<int>(b) << " ";
    }
    std::cout << "\n\n";

    // 6. Demo 2: I2C Master Read Transaction from IMU Sensor (Address 0x68)
    std::cout << "--- Demo 2: I2C Master Read Transaction (IMU Sensor) ---\n";
    std::size_t readCount = i2cHAL.requestFrom(0x68, 1);
    std::cout << "  Requested 1 byte from 0x68. Read count: " << readCount << "\n";

    if (i2cHAL.available() > 0) {
        std::uint8_t whoAmI = i2cHAL.readByte();
        std::cout << "  IMU WHO_AM_I Register: 0x" << std::hex << std::setw(2) << static_cast<int>(whoAmI) << "\n";
    }

    // 7. Demo 3: NACK Handling with Unattached Slave Address (0x3C)
    std::cout << "\n--- Demo 3: NACK Handling (Unattached Slave 0x3C) ---\n";
    bool nackOk = i2cHAL.beginTransmission(0x3C);
    std::cout << "  START Address 0x3C ACK? " << (nackOk ? "Yes (ACK)" : "No (NACK - Expected)") << "\n";
    i2cHAL.endTransmission();

    // 8. SystemBus Reset Verification
    std::cout << "\nResetting SystemBus...\n";
    systemBus.reset();
    std::cout << "I2C Enabled after reset? " << (i2cHAL.enabled() ? "Yes" : "No") << "\n";

    std::cout << "\nI2C Controller Demo completed successfully.\n";
    return 0;
}
