#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <string>
#include <memory>
#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <cassert>

// Platform-specific includes for source location
#ifdef _MSC_VER
#include <intrin.h>
#define PYRAMID_FUNCTION_NAME __FUNCTION__
#define PYRAMID_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#include <cstdlib>
#define PYRAMID_FUNCTION_NAME __PRETTY_FUNCTION__
#define PYRAMID_DEBUG_BREAK() __builtin_trap()
#else
#define PYRAMID_FUNCTION_NAME __func__
#define PYRAMID_DEBUG_BREAK() std::abort()
#endif

namespace Pyramid
{
    namespace Util
    {

        // Log levels with priority ordering
        enum class LogLevel : int
        {
            Trace = 0,
            Debug = 1,
            Info = 2,
            Warn = 3,
            Error = 4,
            Critical = 5,
            Off = 6
        };

        // Convert log level to string
        const char *LogLevelToString(LogLevel level);

        // Convert string to log level
        LogLevel StringToLogLevel(const std::string &str);

        // Source location information
        struct SourceLocation
        {
            const char *file;
            const char *function;
            int line;

            SourceLocation(const char *file = __builtin_FILE(),
                           const char *function = nullptr,
                           int line = __builtin_LINE())
                : file(file), function(function ? function : "unknown"), line(line) {}
        };

        // Log entry structure for structured logging
        struct LogEntry
        {
            LogLevel level;
            std::chrono::system_clock::time_point timestamp;
            std::thread::id threadId;
            SourceLocation location;
            std::string message;
            std::unordered_map<std::string, std::string> fields;
        };

        // Forward declaration
        class Logger;

        // Logger configuration
        struct LoggerConfig
        {
            LogLevel consoleLevel = LogLevel::Info;
            LogLevel fileLevel = LogLevel::Debug;
            std::string logFilePath = "pyramid.log";
            size_t maxFileSize = 10 * 1024 * 1024; // 10MB
            size_t maxFiles = 5;
            bool enableConsole = true;
            bool enableFile = true;
            bool enableTimestamp = true;
            bool enableThreadId = true;
            bool enableSourceLocation = true;
            std::string timestampFormat = "%Y-%m-%d %H:%M:%S";
        };

        // Thread-safe logger implementation
        class Logger
        {
        public:
            static Logger &GetInstance();

            // Configuration
            void Configure(const LoggerConfig &config);
            LoggerConfig GetConfig() const;

            // Core logging function
            void Log(LogLevel level, const std::string &message,
                     const SourceLocation &location = SourceLocation{});

            // Structured logging
            void LogStructured(LogLevel level, const std::string &message,
                               const std::unordered_map<std::string, std::string> &fields,
                               const SourceLocation &location = SourceLocation{});

            // Template logging with variadic arguments
            template <typename... Args>
            void LogFormatted(LogLevel level, const SourceLocation &location, Args &&...args);

            // Flush all outputs
            void Flush();

            // Set log levels at runtime
            void SetConsoleLevel(LogLevel level);
            void SetFileLevel(LogLevel level);

            // Enable/disable outputs
            void EnableConsole(bool enable);
            void EnableFile(bool enable);

        private:
            Logger() = default;
            ~Logger();

            // Non-copyable, non-movable
            Logger(const Logger &) = delete;
            Logger &operator=(const Logger &) = delete;
            Logger(Logger &&) = delete;
            Logger &operator=(Logger &&) = delete;

            // Internal logging implementation
            void WriteToConsole(const LogEntry &entry);
            void WriteToFile(const LogEntry &entry);
            void RotateLogFile();
            std::string FormatLogEntry(const LogEntry &entry, bool includeColor = false);
            std::string GetTimestamp(const std::chrono::system_clock::time_point &time);
            std::string GetColorCode(LogLevel level);
            std::string GetResetColorCode();

            // Thread safety
            mutable std::mutex m_mutex;

            // Configuration
            LoggerConfig m_config;

            // File handling
            std::ofstream m_logFile;
            size_t m_currentFileSize = 0;

            // Performance optimization - pre-allocated buffer
            mutable std::ostringstream m_buffer;
        };

        // Helper class for stream-style logging
        class LogStream
        {
        public:
            LogStream(LogLevel level, const SourceLocation &location);
            ~LogStream();

            template <typename T>
            LogStream &operator<<(const T &value)
            {
                m_stream << value;
                return *this;
            }

        private:
            LogLevel m_level;
            SourceLocation m_location;
            std::ostringstream m_stream;
        };

        // Variadic template implementation for formatted logging
        template <typename... Args>
        void Logger::LogFormatted(LogLevel level, const SourceLocation &location, Args &&...args)
        {
            if (level < m_config.consoleLevel && level < m_config.fileLevel)
                return; // Early exit for performance

            std::lock_guard<std::mutex> lock(m_mutex);

            // Use fold expression to build message
            m_buffer.str("");
            m_buffer.clear();
            ((m_buffer << args), ...);

            Log(level, m_buffer.str(), location);
        }

    }
} // namespace Pyramid::Util

// Enhanced logging macros with backward compatibility
#define PYRAMID_LOG_TRACE(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Trace, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)
#define PYRAMID_LOG_DEBUG(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Debug, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)
#define PYRAMID_LOG_INFO(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Info, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)
#define PYRAMID_LOG_WARN(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Warn, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)
#define PYRAMID_LOG_ERROR(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Error, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)
#define PYRAMID_LOG_CRITICAL(...) ::Pyramid::Util::Logger::GetInstance().LogFormatted(::Pyramid::Util::LogLevel::Critical, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__}, __VA_ARGS__)

// Stream-style logging macros
#define PYRAMID_LOG_STREAM(level) ::Pyramid::Util::LogStream(level, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__})
#define PYRAMID_TRACE_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Trace)
#define PYRAMID_DEBUG_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Debug)
#define PYRAMID_INFO_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Info)
#define PYRAMID_WARN_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Warn)
#define PYRAMID_ERROR_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Error)
#define PYRAMID_CRITICAL_STREAM() PYRAMID_LOG_STREAM(::Pyramid::Util::LogLevel::Critical)

// Structured logging macros
#define PYRAMID_LOG_STRUCTURED(level, message, fields) \
    ::Pyramid::Util::Logger::GetInstance().LogStructured(level, message, fields, ::Pyramid::Util::SourceLocation{__FILE__, PYRAMID_FUNCTION_NAME, __LINE__})

// Conditional logging (only in debug builds)
#ifdef NDEBUG
#define PYRAMID_LOG_DEBUG_ONLY(...) \
    do                              \
    {                               \
    } while (0)
#define PYRAMID_LOG_TRACE_ONLY(...) \
    do                              \
    {                               \
    } while (0)
#else
#define PYRAMID_LOG_DEBUG_ONLY(...) PYRAMID_LOG_DEBUG(__VA_ARGS__)
#define PYRAMID_LOG_TRACE_ONLY(...) PYRAMID_LOG_TRACE(__VA_ARGS__)
#endif

// Enhanced assertion macros
#ifdef NDEBUG
#define PYRAMID_ENABLE_ASSERTS 0
#else
#define PYRAMID_ENABLE_ASSERTS 1
#endif

#if PYRAMID_ENABLE_ASSERTS
#define PYRAMID_INTERNAL_ASSERT(check, level, ...)                  \
    do                                                              \
    {                                                               \
        if (!(check))                                               \
        {                                                           \
            PYRAMID_LOG_##level("Assertion Failed: ", __VA_ARGS__); \
            PYRAMID_DEBUG_BREAK();                                  \
        }                                                           \
    } while (0)

// Client-side asserts (for game code) - Log as ERROR
#define PYRAMID_ASSERT(check, ...) PYRAMID_INTERNAL_ASSERT(check, ERROR, __VA_ARGS__)

// Core engine asserts - Log as CRITICAL
#define PYRAMID_CORE_ASSERT(check, ...) PYRAMID_INTERNAL_ASSERT(check, CRITICAL, __VA_ARGS__)
#else
#define PYRAMID_ASSERT(check, ...) \
    do                             \
    {                              \
    } while (0)
#define PYRAMID_CORE_ASSERT(check, ...) \
    do                                  \
    {                                   \
    } while (0)
#endif

// Performance macros for high-frequency logging
#define PYRAMID_LOG_IF(condition, level, ...) \
    do                                        \
    {                                         \
        if (condition)                        \
        {                                     \
            PYRAMID_LOG_##level(__VA_ARGS__); \
        }                                     \
    } while (0)

// Logger configuration helper
#define PYRAMID_CONFIGURE_LOGGER(config) \
    ::Pyramid::Util::Logger::GetInstance().Configure(config)

// Quick logger setup for common scenarios
#define PYRAMID_SETUP_CONSOLE_LOGGING(level)                    \
    do                                                          \
    {                                                           \
        ::Pyramid::Util::LoggerConfig config;                   \
        config.consoleLevel = ::Pyramid::Util::LogLevel::level; \
        config.enableFile = false;                              \
        PYRAMID_CONFIGURE_LOGGER(config);                       \
    } while (0)

#define PYRAMID_SETUP_FILE_LOGGING(level, filepath)          \
    do                                                       \
    {                                                        \
        ::Pyramid::Util::LoggerConfig config;                \
        config.fileLevel = ::Pyramid::Util::LogLevel::level; \
        config.logFilePath = filepath;                       \
        config.enableConsole = false;                        \
        PYRAMID_CONFIGURE_LOGGER(config);                    \
    } while (0)
