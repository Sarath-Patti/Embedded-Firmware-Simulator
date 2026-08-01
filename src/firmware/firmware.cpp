#include "firmware/firmware.hpp"

namespace efs::firmware {

Firmware::~Firmware() = default;

void Firmware::execute() {
    update();
}

} // namespace efs::firmware
