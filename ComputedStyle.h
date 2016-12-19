#include "LayoutRectOutsets.h"
namespace blink {
class ComputedStyle {
    public:

        void getBorderEdgeInfo(BorderEdge edges[], bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const
        {
            bool horizontal = isHorizontalWritingMode();
            edges[BSTop] = BorderEdge(m_border.borderTopWidth(),
                                      m_border.m_topColor,
                                      m_border.borderTopStyle(),
                                      horizontal || includeLogicalLeftEdge);

	    edges[BSRight] = BorderEdge(m_border.borderRightWidth(),
					m_border.m_rightColor,
					m_border.borderRightStyle(),
					!horizontal || includeLogicalRightEdge);

	    edges[BSBottom] = BorderEdge(m_border.borderBottomWidth(),
					 m_border.m_bottomColor,
					 m_border.borderBottomStyle(),
					 horizontal || includeLogicalRightEdge);

	    edges[BSLeft] = BorderEdge(m_border.borderLeftWidth(),
				       m_border.m_leftColor,
				       m_border.borderLeftStyle(),
				       !horizontal || includeLogicalLeftEdge);

        }

        FloatRoundedRect getRoundedInnerBorderFor(const LayoutRect& borderRect, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const {
	    bool horizontal = isHorizontalWritingMode();

	    int leftWidth = (!horizontal || includeLogicalLeftEdge) ? borderLeftWidth() : 0;
	    int rightWidth = (!horizontal || includeLogicalRightEdge) ? borderRightWidth() : 0;
	    int topWidth = (horizontal || includeLogicalLeftEdge) ? borderTopWidth() : 0;
	    int bottomWidth = (horizontal || includeLogicalRightEdge) ? borderBottomWidth() : 0;

	    return getRoundedInnerBorderFor(borderRect,
		LayoutRectOutsets(-topWidth, -rightWidth, -bottomWidth, -leftWidth),
		includeLogicalLeftEdge, includeLogicalRightEdge);
        }
	int borderLeftWidth() const { return m_border.borderLeftWidth(); }
	int borderRightWidth() const { return m_border.borderRightWidth(); }
	int borderTopWidth() const { return m_border.borderTopWidth(); }
	int borderBottomWidth() const { return m_border.borderBottomWidth(); }

        FloatRoundedRect getRoundedInnerBorderFor(const LayoutRect& borderRect,
                                                  const LayoutRectOutsets insets, bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const {
            LayoutRect innerRect(borderRect);
            innerRect.expand(insets);

            FloatRoundedRect roundedRect(pixelSnappedIntRect(innerRect));
            if (hasBorderRadius()) {
                FloatRoundedRect::Radii radii = getRoundedBorderFor(borderRect).getRadii();
                // Insets use negative values.
                radii.shrink(
                             -insets.top(),
                             -insets.bottom(),
                             -insets.left(),
                             -insets.right());
                roundedRect.includeLogicalEdges(radii, isHorizontalWritingMode(), includeLogicalLeftEdge, includeLogicalRightEdge);
            }
            return roundedRect;
        }
        FloatRoundedRect getRoundedBorderFor(const LayoutRect& borderRect,
                                             bool includeLogicalLeftEdge = true,
                                             bool includeLogicalRightEdge = true) const
        {
            FloatRoundedRect roundedRect(pixelSnappedIntRect(borderRect));
            if (hasBorderRadius()) {
                FloatRoundedRect::Radii radii = calcRadiiFor(m_border, borderRect.size());
                roundedRect.includeLogicalEdges(radii, isHorizontalWritingMode(), includeLogicalLeftEdge, includeLogicalRightEdge);
                roundedRect.constrainRadii();
            }
            return roundedRect;
        }

        bool hasBorderRadius() const { return m_border.hasBorderRadius(); }

        bool isHorizontalWritingMode() const { return true; }
        struct BorderData {
            FloatSize m_topLeft;
            FloatSize m_topRight;
            FloatSize m_bottomLeft;
            FloatSize m_bottomRight;
            int m_topWidth;
            int m_bottomWidth;
            int m_leftWidth;
            int m_rightWidth;

            Color m_topColor;
            Color m_bottomColor;
            Color m_leftColor;
            Color m_rightColor;

            EBorderStyle m_topStyle;
            EBorderStyle m_bottomStyle;
            EBorderStyle m_leftStyle;
            EBorderStyle m_rightStyle;

            int borderTopWidth() const { return m_topWidth; }
            int borderBottomWidth() const { return m_bottomWidth; }
            int borderLeftWidth() const { return m_leftWidth; }
            int borderRightWidth() const { return m_rightWidth; }

            EBorderStyle borderTopStyle() const { return m_topStyle; }
            EBorderStyle borderBottomStyle() const { return m_bottomStyle; }
            EBorderStyle borderLeftStyle() const { return m_leftStyle; }
            EBorderStyle borderRightStyle() const { return m_rightStyle; }

            FloatSize topLeft() const { return m_topLeft; }
            FloatSize topRight() const { return m_topRight; }
            FloatSize bottomLeft() const { return m_bottomLeft; }
            FloatSize bottomRight() const { return m_bottomRight; }
            bool hasBorderRadius() const
            {
                if (m_topLeft.width() != 0.)
                    return true;
                if (m_topRight.width() != 0.)
                    return true;
                if (m_bottomLeft.width() != 0.)
                    return true;
                if (m_bottomRight.width() != 0.)
                    return true;
                return false;
            }

        };

        BorderData  m_border;

        static float floatValueForLength(float x, float max) { if (x > max) return max; return x; }

        static FloatRoundedRect::Radii calcRadiiFor(const BorderData& border, FloatSize size)
        {
            return FloatRoundedRect::Radii(
                                           FloatSize(floatValueForLength(border.topLeft().width(), size.width()),
                                                     floatValueForLength(border.topLeft().height(), size.height())),
                                           FloatSize(floatValueForLength(border.topRight().width(), size.width()),
                                                     floatValueForLength(border.topRight().height(), size.height())),
                                           FloatSize(floatValueForLength(border.bottomLeft().width(), size.width()),
                                                     floatValueForLength(border.bottomLeft().height(), size.height())),
                                           FloatSize(floatValueForLength(border.bottomRight().width(), size.width()),
                                                     floatValueForLength(border.bottomRight().height(), size.height())));
        }
};
}
