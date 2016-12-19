#include "Color.h"
#include <algorithm>
#include <math.h>

int clampTo(int a, int min, int max)
{
    if (a < min)
	return min;
    if (a > max)
        return max;
    return a;
}

namespace blink {
static const RGBA32 darkenedWhite = 0xFFABABAB;
void Color::getRGBA(float& r, float& g, float& b, float& a) const
{
            r = red() / 255.0f;
                g = green() / 255.0f;
                    b = blue() / 255.0f;
                        a = alpha() / 255.0f;
}


RGBA32 makeRGBA(int r, int g, int b, int a)
{
    return clampTo(a, 0, 255) << 24 | clampTo(r, 0, 255) << 16 | clampTo(g, 0, 255) << 8 | clampTo(b, 0, 255);
}

Color Color::dark() const
{
    // Hardcode this common case for speed.
    if (m_color == white)
        return darkenedWhite;

    const float scaleFactor = nextafterf(256.0f, 0.0f);

    float r, g, b, a;
    getRGBA(r, g, b, a);

    float v = std::max(r, std::max(g, b));
    float multiplier = std::max(0.0f, (v - 0.33f) / v);

    return Color(static_cast<int>(multiplier * r * scaleFactor),
                 static_cast<int>(multiplier * g * scaleFactor),
                 static_cast<int>(multiplier * b * scaleFactor),
                 alpha());
}
}
