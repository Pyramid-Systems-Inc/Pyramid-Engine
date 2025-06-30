#pragma once

// Backward compatibility header for the old logging system
// This file redirects to the new enhanced logging system in Utils

#include <Pyramid/Util/Log.hpp>

// Legacy namespace alias for backward compatibility
namespace Pyramid
{
    namespace Log
    {

        // Legacy Print function for backward compatibility
        template <typename... Args>
        void Print(std::ostream &os, const Args &...args)
        {
            ((os << args), ...);
        }

        // Legacy Print function without ostream (not used by new macros but kept for compatibility)
        template <typename... Args>
        void Print(const Args &...args)
        {
            // This function is kept for any legacy code that might call it directly
            // but the new macros don't use it
        }

    }
} // namespace Pyramid::Log

// Note: The old logging macros are now defined in Pyramid/Util/Log.hpp
// and provide enhanced functionality while maintaining backward compatibility.
//
// Old usage:
//   PYRAMID_LOG_INFO("Hello ", 123);
//   PYRAMID_LOG_ERROR("Failed to load: ", filename);
//   PYRAMID_ASSERT(ptr != nullptr, "Pointer was null!");
//
// Still works exactly the same, but now with enhanced features:
//   - Thread-safe logging
//   - File output with rotation
//   - Configurable log levels
//   - Source location information
//   - Structured logging support
//   - Performance optimizations
//
// New features available:
//   - Stream-style logging: PYRAMID_INFO_STREAM() << "Message: " << value;
//   - Structured logging: PYRAMID_LOG_STRUCTURED(level, message, fields);
//   - Runtime configuration: PYRAMID_CONFIGURE_LOGGER(config);
//   - Log level filtering: Logger::GetInstance().SetConsoleLevel(LogLevel::Warn);
//
// Migration guide:
//   1. Replace #include <Pyramid/Core/Log.hpp> with #include <Pyramid/Util/Log.hpp>
//   2. Existing logging calls continue to work without changes
//   3. Optionally configure the logger for enhanced features
//   4. Use new structured logging and stream-style APIs as needed
