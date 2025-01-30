#pragma once
#include <Pyramid/Core/Prerequisites.hpp>

namespace Pyramid {

/**
 * @brief Basic vertex structure with position and color
 */
struct Vertex {
    f32 x, y, z;    // Position
    f32 r, g, b, a; // Color

    Vertex(f32 x = 0.0f, f32 y = 0.0f, f32 z = 0.0f, 
           f32 r = 1.0f, f32 g = 1.0f, f32 b = 1.0f, f32 a = 1.0f)
        : x(x), y(y), z(z), r(r), g(g), b(b), a(a) {}
};

} // namespace Pyramid
