#pragma once

namespace Pyramid {

// Basic type definitions
using i8  = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PYRAMID_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PYRAMID_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define PYRAMID_PLATFORM_MACOS
#else
    #error "Unsupported platform"
#endif

// API export/import macros
#ifdef PYRAMID_PLATFORM_WINDOWS
    #ifdef PYRAMID_EXPORT
        #define PYRAMID_API __declspec(dllexport)
    #else
        #define PYRAMID_API __declspec(dllimport)
    #endif
#else
    #define PYRAMID_API
#endif

// Debug macros
#ifdef _DEBUG
    #define PYRAMID_DEBUG
#endif

} // namespace Pyramid
