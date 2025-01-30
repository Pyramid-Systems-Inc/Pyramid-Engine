#pragma once
#include <cstdint>

namespace Pyramid {

// Basic types
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

// Graphics API enumeration
enum class GraphicsAPI {
    OpenGL,
    DirectX9,
    DirectX10,
    DirectX11,
    DirectX12,
    Vulkan,
    None
};

// Color structure
struct Color {
    f32 r, g, b, a;

    Color(f32 r = 0.0f, f32 g = 0.0f, f32 b = 0.0f, f32 a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

// Forward declarations
class Window;
class Game;
class IGraphicsDevice;

} // namespace Pyramid