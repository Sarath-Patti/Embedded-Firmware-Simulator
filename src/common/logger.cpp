#include "common/logger.hpp"
#include <iostream>

namespace efs::common {

void Logger::log(LogLevel level, std::string_view message) {
    switch (level) {
        case LogLevel::Debug:
            std::cout << "[DEBUG] " << message << "\n";
            break;
        case LogLevel::Info:
            std::cout << "[INFO] " << message << "\n";
            break;
        case LogLevel::Warning:
            std::cout << "[WARN] " << message << "\n";
            break;
        case LogLevel::Error:
            std::cerr << "[ERROR] " << message << "\n";
            break;
    }
}

void Logger::debug(std::string_view message) {
    log(LogLevel::Debug, message);
}

void Logger::info(std::string_view message) {
    log(LogLevel::Info, message);
}

void Logger::warning(std::string_view message) {
    log(LogLevel::Warning, message);
}

void Logger::error(std::string_view message) {
    log(LogLevel::Error, message);
}

} // namespace efs::common
