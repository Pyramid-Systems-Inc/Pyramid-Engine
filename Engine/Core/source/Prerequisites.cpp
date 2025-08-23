#include <Pyramid/Core/Prerequisites.hpp>
#include <cmath>

namespace Pyramid {

// Color constants
const Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
const Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
const Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
const Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);
const Color Color::Clear(0.0f, 0.0f, 0.0f, 0.0f);

// Color comparison operators
bool Color::operator==(const Color& other) const {
    const f32 epsilon = 1e-6f;
    return std::abs(r - other.r) < epsilon &&
           std::abs(g - other.g) < epsilon &&
           std::abs(b - other.b) < epsilon &&
           std::abs(a - other.a) < epsilon;
}

bool Color::operator!=(const Color& other) const {
    return !(*this == other);
}

} // namespace Pyramid
