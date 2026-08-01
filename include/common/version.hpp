#ifndef EFS_COMMON_VERSION_HPP
#define EFS_COMMON_VERSION_HPP

#include <cstdint>
#include <string_view>

namespace efs::common {

constexpr std::uint32_t VERSION_MAJOR = 1;
constexpr std::uint32_t VERSION_MINOR = 7;
constexpr std::uint32_t VERSION_PATCH = 0;

constexpr std::string_view VERSION_STRING = "v1.7.0";
constexpr std::string_view PROJECT_NAME = "Embedded Firmware Simulator";

} // namespace efs::common

#endif // EFS_COMMON_VERSION_HPP
