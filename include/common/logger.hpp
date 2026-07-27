#ifndef EFS_COMMON_LOGGER_HPP
#define EFS_COMMON_LOGGER_HPP

#include <string_view>

namespace efs::common {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static void log(LogLevel level, std::string_view message);
    static void debug(std::string_view message);
    static void info(std::string_view message);
    static void warning(std::string_view message);
    static void error(std::string_view message);
};

} // namespace efs::common

#endif // EFS_COMMON_LOGGER_HPP
