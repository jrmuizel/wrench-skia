#define PLATFORM_EXPORT
#include "FloatPoint.h"
namespace blink {
bool findIntersection(const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& d1, const FloatPoint& d2, FloatPoint& intersection)
{
    float pxLength = p2.x() - p1.x();
    float pyLength = p2.y() - p1.y();

    float dxLength = d2.x() - d1.x();
    float dyLength = d2.y() - d1.y();

    float denom = pxLength * dyLength - pyLength * dxLength;
    if (!denom)
        return false;

    float param = ((d1.x() - p1.x()) * dyLength - (d1.y() - p1.y()) * dxLength) / denom;

    intersection.setX(p1.x() + param * pxLength);
    intersection.setY(p1.y() + param * pyLength);
    return true;
}
}

