#include "Pyramid/Util/Log.hpp"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace Pyramid
{
    namespace Util
    {

        const char *LogLevelToString(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::Trace:
                return "TRACE";
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Warn:
                return "WARN";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Critical:
                return "CRITICAL";
            case LogLevel::Off:
                return "OFF";
            default:
                return "UNKNOWN";
            }
        }

        LogLevel StringToLogLevel(const std::string &str)
        {
            std::string lower = str;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            if (lower == "trace")
                return LogLevel::Trace;
            if (lower == "debug")
                return LogLevel::Debug;
            if (lower == "info")
                return LogLevel::Info;
            if (lower == "warn")
                return LogLevel::Warn;
            if (lower == "error")
                return LogLevel::Error;
            if (lower == "critical")
                return LogLevel::Critical;
            if (lower == "off")
                return LogLevel::Off;

            return LogLevel::Info; // Default
        }

        Logger &Logger::GetInstance()
        {
            static Logger instance;
            return instance;
        }

        Logger::~Logger()
        {
            Flush();
            if (m_logFile.is_open())
            {
                m_logFile.close();
            }
        }

        void Logger::Configure(const LoggerConfig &config)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config = config;

            // Open log file if file logging is enabled
            if (m_config.enableFile)
            {
                if (m_logFile.is_open())
                {
                    m_logFile.close();
                }

                // Create directory if it doesn't exist
                std::filesystem::path logPath(m_config.logFilePath);
                if (logPath.has_parent_path())
                {
                    std::filesystem::create_directories(logPath.parent_path());
                }

                m_logFile.open(m_config.logFilePath, std::ios::app);
                if (m_logFile.is_open())
                {
                    // Get current file size
                    m_logFile.seekp(0, std::ios::end);
                    m_currentFileSize = static_cast<size_t>(m_logFile.tellp());
                }
            }
        }

        LoggerConfig Logger::GetConfig() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_config;
        }

        void Logger::Log(LogLevel level, const std::string &message, const SourceLocation &location)
        {
            LogEntry entry;
            entry.level = level;
            entry.timestamp = std::chrono::system_clock::now();
            entry.threadId = std::this_thread::get_id();
            entry.location = location;
            entry.message = message;

            std::lock_guard<std::mutex> lock(m_mutex);

            // Check levels after acquiring lock to avoid race conditions
            bool shouldWriteConsole = m_config.enableConsole && level >= m_config.consoleLevel;
            bool shouldWriteFile = m_config.enableFile && level >= m_config.fileLevel && m_logFile.is_open();

            if (!shouldWriteConsole && !shouldWriteFile)
                return; // Early exit if nothing to do

            // Write to console if enabled and level is sufficient
            if (shouldWriteConsole)
            {
                WriteToConsole(entry);
            }

            // Write to file if enabled and level is sufficient
            if (shouldWriteFile)
            {
                WriteToFile(entry);
            }
        }

        void Logger::LogStructured(LogLevel level, const std::string &message,
                                   const std::unordered_map<std::string, std::string> &fields,
                                   const SourceLocation &location)
        {
            LogEntry entry;
            entry.level = level;
            entry.timestamp = std::chrono::system_clock::now();
            entry.threadId = std::this_thread::get_id();
            entry.location = location;
            entry.message = message;
            entry.fields = fields;

            std::lock_guard<std::mutex> lock(m_mutex);

            // Check levels after acquiring lock to avoid race conditions
            bool shouldWriteConsole = m_config.enableConsole && level >= m_config.consoleLevel;
            bool shouldWriteFile = m_config.enableFile && level >= m_config.fileLevel && m_logFile.is_open();

            if (!shouldWriteConsole && !shouldWriteFile)
                return; // Early exit if nothing to do

            // Write to console if enabled and level is sufficient
            if (shouldWriteConsole)
            {
                WriteToConsole(entry);
            }

            // Write to file if enabled and level is sufficient
            if (shouldWriteFile)
            {
                WriteToFile(entry);
            }
        }

        void Logger::Flush()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_config.enableConsole)
            {
                std::cout.flush();
                std::cerr.flush();
            }

            if (m_config.enableFile && m_logFile.is_open())
            {
                m_logFile.flush();
            }
        }

        void Logger::SetConsoleLevel(LogLevel level)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config.consoleLevel = level;
        }

        void Logger::SetFileLevel(LogLevel level)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config.fileLevel = level;
        }

        void Logger::EnableConsole(bool enable)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config.enableConsole = enable;
        }

        void Logger::EnableFile(bool enable)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config.enableFile = enable;

            if (!enable && m_logFile.is_open())
            {
                m_logFile.close();
            }
            else if (enable && !m_logFile.is_open())
            {
                // Reopen file
                m_logFile.open(m_config.logFilePath, std::ios::app);
                if (m_logFile.is_open())
                {
                    m_logFile.seekp(0, std::ios::end);
                    m_currentFileSize = static_cast<size_t>(m_logFile.tellp());
                }
            }
        }

        void Logger::WriteToConsole(const LogEntry &entry)
        {
            std::string formatted = FormatLogEntry(entry, true); // Enable colors for console

            // Write to stderr for warnings and errors, stdout for others
            if (entry.level >= LogLevel::Warn)
            {
                std::cerr << formatted << std::endl;
            }
            else
            {
                std::cout << formatted << std::endl;
            }
        }

        void Logger::WriteToFile(const LogEntry &entry)
        {
            std::string formatted = FormatLogEntry(entry, false); // No colors for file
            m_logFile << formatted << std::endl;

            // Update file size and check for rotation
            m_currentFileSize += formatted.length() + 1; // +1 for newline

            if (m_currentFileSize >= m_config.maxFileSize)
            {
                RotateLogFile();
            }
        }

        void Logger::RotateLogFile()
        {
            if (!m_logFile.is_open())
                return;

            m_logFile.close();

            // Rotate existing files
            std::filesystem::path basePath(m_config.logFilePath);
            std::string basePathStr = basePath.string();

            // Remove oldest file if it exists
            std::string oldestFile = basePathStr + "." + std::to_string(m_config.maxFiles - 1);
            if (std::filesystem::exists(oldestFile))
            {
                std::filesystem::remove(oldestFile);
            }

            // Rotate files
            for (size_t i = m_config.maxFiles - 1; i > 0; --i)
            {
                std::string currentFile = basePathStr + "." + std::to_string(i - 1);
                std::string nextFile = basePathStr + "." + std::to_string(i);

                if (std::filesystem::exists(currentFile))
                {
                    std::filesystem::rename(currentFile, nextFile);
                }
            }

            // Move current log to .0
            if (std::filesystem::exists(basePathStr))
            {
                std::filesystem::rename(basePathStr, basePathStr + ".0");
            }

            // Open new log file
            m_logFile.open(m_config.logFilePath, std::ios::out | std::ios::trunc);
            m_currentFileSize = 0;
        }

        std::string Logger::FormatLogEntry(const LogEntry &entry, bool includeColor)
        {
            std::ostringstream buffer;

            // Add color if requested and supported
            if (includeColor)
            {
                buffer << GetColorCode(entry.level);
            }

            // Add timestamp
            if (m_config.enableTimestamp)
            {
                buffer << "[" << GetTimestamp(entry.timestamp) << "] ";
            }

            // Add log level
            buffer << "[" << LogLevelToString(entry.level) << "] ";

            // Add thread ID
            if (m_config.enableThreadId)
            {
                buffer << "[Thread:" << entry.threadId << "] ";
            }

            // Add source location
            if (m_config.enableSourceLocation)
            {
                std::filesystem::path filePath(entry.location.file);
                buffer << "[" << filePath.filename().string() << ":" << entry.location.line << "] ";
            }

            // Add message
            buffer << entry.message;

            // Add structured fields if any
            if (!entry.fields.empty())
            {
                buffer << " {";
                bool first = true;
                for (const auto &field : entry.fields)
                {
                    if (!first)
                        buffer << ", ";
                    buffer << field.first << "=" << field.second;
                    first = false;
                }
                buffer << "}";
            }

            // Reset color if requested
            if (includeColor)
            {
                buffer << GetResetColorCode();
            }

            return buffer.str();
        }

        std::string Logger::GetTimestamp(const std::chrono::system_clock::time_point &time)
        {
            auto timeT = std::chrono::system_clock::to_time_t(time);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          time.time_since_epoch()) %
                      1000;

            std::ostringstream oss;
            oss << std::put_time(std::localtime(&timeT), m_config.timestampFormat.c_str());
            oss << "." << std::setfill('0') << std::setw(3) << ms.count();

            return oss.str();
        }

        std::string Logger::GetColorCode(LogLevel level)
        {
            // Check if console supports colors
            static bool supportsColor = []()
            {
#ifdef _WIN32
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                DWORD mode;
                if (GetConsoleMode(hConsole, &mode))
                {
                    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                    return SetConsoleMode(hConsole, mode) != 0;
                }
                return false;
#else
                return isatty(fileno(stdout)) != 0;
#endif
            }();

            if (!supportsColor)
                return "";

            switch (level)
            {
            case LogLevel::Trace:
                return "\033[37m"; // White
            case LogLevel::Debug:
                return "\033[36m"; // Cyan
            case LogLevel::Info:
                return "\033[32m"; // Green
            case LogLevel::Warn:
                return "\033[33m"; // Yellow
            case LogLevel::Error:
                return "\033[31m"; // Red
            case LogLevel::Critical:
                return "\033[35;1m"; // Bright Magenta
            default:
                return "";
            }
        }

        std::string Logger::GetResetColorCode()
        {
            return "\033[0m";
        }

        // LogStream implementation
        LogStream::LogStream(LogLevel level, const SourceLocation &location)
            : m_level(level), m_location(location)
        {
        }

        LogStream::~LogStream()
        {
            Logger::GetInstance().Log(m_level, m_stream.str(), m_location);
        }

    }
} // namespace Pyramid::Util
