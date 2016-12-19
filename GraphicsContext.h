#include "Color.h"
#include "FloatRect.h"
#include "FloatRoundedRect.h"
#include "IntPoint.h"
#include "../include/core/SkPath.h"
#include "../include/core/SkPaint.h"
#include "../include/core/SkCanvas.h"
#include <vector>
namespace blink
{
enum StrokeStyle {
        NoStroke,
        SolidStroke,
        DottedStroke,
        DashedStroke,
        DoubleStroke,
        WavyStroke,
};

enum AntiAliasingMode {
            NotAntiAliased,
                AntiAliased
};

enum WindRule {
        RULE_NONZERO = SkPath::kWinding_FillType,
        RULE_EVENODD = SkPath::kEvenOdd_FillType
};

typedef float DashArrayElement;
typedef std::vector<DashArrayElement> DashArray;

struct GraphicsContext
{
        GraphicsContext(SkCanvas *c) : m_canvas(c) {}
        SkCanvas* m_canvas;
        bool mShouldAntialias = true;
        bool shouldAntialias() { return mShouldAntialias; }
        void setShouldAntialias(bool x) { mShouldAntialias = x; }

        StrokeStyle mStrokeStyle;
        void setStrokeStyle(StrokeStyle s) { mStrokeStyle = s; }
        StrokeStyle getStrokeStyle() { return mStrokeStyle; }

        Color mStrokeColor;
        void setStrokeColor(Color c) { mStrokeColor = c; }

        Color mFillColor;
        void setFillColor(Color c) { mFillColor = c; }

        void strokeRect(FloatRect r, float borderWidth) {
            SkPaint paint;
            paint.setColor(mStrokeColor.rgb());
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(borderWidth);
            m_canvas->drawRect(r, paint);
        }

        void drawPath(SkPath &path, SkPaint &paint) {
            m_canvas->drawPath(path, paint);
        }

        void drawRect(const SkRect &rect) {
            SkPaint paint;
            paint.setColor(mFillColor.rgb());
            m_canvas->drawRect(rect, paint);
        }

        void fillRect(const SkRect &rect, Color color) {
            SkPaint paint;
            paint.setColor(color.rgb());
            m_canvas->drawRect(rect, paint);
        }


        void drawLine(IntPoint start, IntPoint end) {
            SkPaint paint;
            paint.setColor(mStrokeColor.rgb());
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(m_strokeThickness);
            m_canvas->drawLine(start.x(), start.y(), end.x(), end.y(), paint);
        }

        void clip(const FloatRect& rect) { clipRect(rect); }

        SkPaint fillPaint() {
            SkPaint paint;
            paint.setColor(mFillColor.rgb());
            return paint;
        }

        void fillPath(SkPath &path) {
            SkPaint paint;
            paint.setColor(mFillColor.rgb());
            m_canvas->drawPath(path, paint);
        }

        void clipRect(const SkRect& rect, SkClipOp op = kIntersect_SkClipOp)
        {
            m_canvas->clipRect(rect, op, true);
        }

        void clipOutRoundedRect(const FloatRoundedRect& rect)
        {
            clipRoundedRect(rect, kDifference_SkClipOp);
        }

        void clipRoundedRect(const FloatRoundedRect& rrect, SkClipOp op = kIntersect_SkClipOp)
        {
            if (!rrect.isRounded()) {
                clipRect(rrect.rect(), op);
                return;
            }
            clipRRect(rrect, op);
        }

        void clipRRect(const SkRRect& rect, SkClipOp op = kIntersect_SkClipOp)
        {
            m_canvas->clipRRect(rect, op, true);
        }

        void clipPath(const SkPath& path, AntiAliasingMode aa)
        {
            m_canvas->clipPath(path, kIntersect_SkClipOp, aa == AntiAliased);
        }
        void fillDRRect(const FloatRoundedRect& outer,
                            const FloatRoundedRect& inner, const Color& color)
        {
            SkPaint paint;
            paint.setColor(color.rgb());
            m_canvas->drawDRRect(outer, inner, paint);
        }

        void save() { m_canvas->save(); }
        void restore() { m_canvas->restore(); }

        void beginLayer(float opacity) {
            SkPaint layerPaint;
            layerPaint.setAlpha(static_cast<unsigned char>(opacity * 255));
            //layerPaint.setXfermodeMode(xfermode);
            //layerPaint.setColorFilter(WebCoreColorFilterToSkiaColorFilter(colorFilter));
            //layerPaint.setImageFilter(std::move(imageFilter));

            m_canvas->saveLayer(nullptr, &layerPaint);
        }
        void endLayer() {
            m_canvas->restore();
        }

        void setLineDash(const DashArray& dashes, float dashOffset)
        {
        }

        float m_strokeThickness;
        void setStrokeThickness(float thickness) {
            m_strokeThickness = thickness;
        }
        void strokePath(const SkPath &path) {
            SkPaint paint;
            paint.setColor(mStrokeColor.rgb());
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(m_strokeThickness);
            m_canvas->drawPath(path, paint);
        }
};

struct GraphicsContextStateSaver
{
    GraphicsContextStateSaver(GraphicsContext& context, bool saveAndRestore = true)
        : m_context(context)
        , m_saveAndRestore(saveAndRestore)
    {
        if (m_saveAndRestore)
            m_context.save();
    }

    ~GraphicsContextStateSaver()
    {
        if (m_saveAndRestore)
            m_context.restore();
    }

    GraphicsContext& m_context;
    bool m_saveAndRestore;
};

}
