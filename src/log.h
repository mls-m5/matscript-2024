#pragma once

#include <iostream>

namespace logging {

enum class LogLevel {
    Error,
    Warning,
    Info,
    Verbose,
    Debug,
};

inline LogLevel logLevel = LogLevel::Info;

} // namespace logging

// Verbose log
template <typename... Args>
std::ostream &vlog(const Args &...args) {
    if (logging::logLevel < logging::LogLevel::Verbose) {
        return std::cerr;
    }
    return ((std::cerr << args), ...) << "\n";
}

// Very verbose debug log
template <typename... Args>
std::ostream &dlog(const Args &...args) {
    if (logging::logLevel < logging::LogLevel::Debug) {
        return std::cerr;
    }
    return ((std::cerr << args), ...) << "\n";
}

// Very verbose debug log
template <typename... Args>
std::ostream &elog(const Args &...args) {
    if (logging::logLevel < logging::LogLevel::Error) {
        return std::cerr;
    }
    return ((std::cerr << args), ...) << "\n";
}
