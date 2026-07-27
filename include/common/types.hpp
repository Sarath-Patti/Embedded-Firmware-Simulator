#ifndef EFS_COMMON_TYPES_HPP
#define EFS_COMMON_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace efs::common {

using Byte = std::uint8_t;
using Word = std::uint16_t;
using DWord = std::uint32_t;
using QWord = std::uint64_t;

using Address = std::uint64_t;
using Size = std::size_t;

} // namespace efs::common

#endif // EFS_COMMON_TYPES_HPP
