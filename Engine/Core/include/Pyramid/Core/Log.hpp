#pragma once

#include <Pyramid/Core/Prerequisites.hpp> // Include basic types if needed
#include <iostream> // For std::cout, std::cerr
#include <string>   // For std::string
#include <sstream>  // For basic formatting (can be replaced later)
#include <cassert>  // For assert()

// --- Basic Logging Macros ---
// (can be enhanced later with a proper logging library like spdlog)
// For now, just printing to console with level prefixes.

// TODO: Add proper formatting (timestamps, file/line numbers)
// TODO: Add ability to disable levels or redirect output

#define PYRAMID_LOG_INFO(...)    do { std::cout << "[INFO] "; \
                                      Pyramid::Log::Print(std::cout, __VA_ARGS__); \
                                      std::cout << std::endl; } while(0)

#define PYRAMID_LOG_WARN(...)    do { std::cerr << "[WARN] "; \
                                      Pyramid::Log::Print(std::cerr, __VA_ARGS__); \
                                      std::cerr << std::endl; } while(0)

#define PYRAMID_LOG_ERROR(...)   do { std::cerr << "[ERROR] "; \
                                      Pyramid::Log::Print(std::cerr, __VA_ARGS__); \
                                      std::cerr << std::endl; } while(0)

#define PYRAMID_LOG_CRITICAL(...) do { std::cerr << "[CRITICAL] "; \
                                       Pyramid::Log::Print(std::cerr, __VA_ARGS__); \
                                       std::cerr << std::endl; } while(0) // Separate assert below


// --- Basic Assertion Macros ---
// Enabled in Debug builds (when NDEBUG is not defined), disabled in Release builds.
#ifdef NDEBUG
    #define PYRAMID_ENABLE_ASSERTS 0
#else
    #define PYRAMID_ENABLE_ASSERTS 1
#endif

#if PYRAMID_ENABLE_ASSERTS
    // Log the message first, then assert. 
    // assert() itself doesn't take messages portably.
    #define PYRAMID_INTERNAL_ASSERT(check, type, ...) \
        do { \
            if(!(check)) { \
                PYRAMID_LOG_##type("Assertion Failed: ", __VA_ARGS__); \
                /* TODO: Add __debugbreak() or platform specific break here? */ \
                assert(check); \
            } \
        } while(0)

    // Client-side asserts (for game code) - Log as ERROR
    #define PYRAMID_ASSERT(check, ...) PYRAMID_INTERNAL_ASSERT(check, ERROR, __VA_ARGS__)
    
    // Core engine asserts - Log as CRITICAL
    #define PYRAMID_CORE_ASSERT(check, ...) PYRAMID_INTERNAL_ASSERT(check, CRITICAL, __VA_ARGS__)
#else
    #define PYRAMID_ASSERT(check, ...) do {} while(0)
    #define PYRAMID_CORE_ASSERT(check, ...) do {} while(0)
#endif


// --- Simple helper namespace for logging ---
namespace Pyramid { namespace Log {
    template<typename T>
    void PrintArg(std::ostream& os, const T& arg) {
        os << arg;
    }

    // Base case for recursion
    inline void Print(std::ostream& os) {}

    // Recursive variadic template function
    template<typename First, typename... Rest>
    void Print(std::ostream& os, const First& first, const Rest&... rest) {
        PrintArg(os, first);
        Print(os, rest...); // Recursive call
    }
    
    // Overload for the macros to use (directs to cout/cerr based on macro)
    template<typename... Args>
    void Print(const Args&... args) {
        // This function body isn't strictly needed if macros call Print(std::ostream&, ...) directly,
        // but it provides a single entry point if needed later.
        // The macros above call Print(std::cerr/cout, ...) directly for simplicity now.
    }

    // Specific implementation for macros to use std::cout/cerr directly
    // Takes ostream as first arg now
     template<typename... Args>
    void Print(std::ostream& os, const Args&... args) {
         // Use a fold expression (C++17) for cleaner printing
         ((os << args), ...);
    }

}} // namespace Pyramid::Log

// Example Usage:
// PYRAMID_LOG_INFO("Hello ", 123);
// PYRAMID_LOG_WARN("Something might be wrong: ", variable);
// PYRAMID_LOG_ERROR("Failed to load file: ", filepath);
// PYRAMID_CORE_ASSERT(ptr != nullptr, "Pointer was null!");
// PYRAMID_ASSERT(x > 0);
