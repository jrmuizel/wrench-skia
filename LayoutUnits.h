#ifndef LayoutUnits_h
#define LayoutUnits_h
//#include "FloatRect.h"
#include <math.h>
namespace blink {
        typedef float LayoutUnit;

        inline int snapSizeToPixel(LayoutUnit size, LayoutUnit location)
        {
            float i;
            LayoutUnit fraction = modff(location, &i);
            return floor((fraction + size)+.5) - floor(fraction + 0.5);
        }
}
#endif
