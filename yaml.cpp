/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../include/core/SkCanvas.h"
#include "../include/core/SkTypeface.h"
#include "../include/core/SkStream.h"
#include "../include/core/SkData.h"
#include "../include/core/SkSurface.h"
#include "../include/core/SkRefCnt.h"
#include "../include/effects/SkGradientShader.h"
#include "../include/gpu/GrContext.h"
#include "../include/gpu/gl/GrGLInterface.h"
//#include "../include/gpu/gl/GrGLDefines.h"
#include "../include/core/SkPictureRecorder.h"
#include "../tools/Resources.h"

// These headers are just handy for writing this example file.  Nothing Skia specific.
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <assert.h>

#include "yaml-cpp/yaml.h"
#include "BoxBorderPainter.h"
#include "GraphicsContext.h"
#include "ComputedStyle.h"
using namespace std;

// SkColor is typdef to unsigned int so we wrap
// it in a new type
struct SkColorW
{
    SkColor color;
};

namespace YAML {

template<>
struct convert<SkColorW> {
    static bool decode(const Node& node, SkColorW& rhs) {
        if (node.IsScalar()) {
            auto val = node.as<string>();
            if (val == "red") {
                rhs.color = SK_ColorRED;
                return true;
            }
            if (val == "green") {
                rhs.color = SK_ColorGREEN;
                return true;
            }
            if (val == "blue") {
                rhs.color = SK_ColorBLUE;
                return true;
            }
            if (val == "white") {
                rhs.color = SK_ColorWHITE;
                return true;
            }
            if (val == "black") {
                rhs.color = SK_ColorBLACK;
                return true;
            }

            auto vec = node.as<vector<double>>();
            if (vec.size() == 4) {
                SkColor4f color;
                color.fR = vec[0] / 255.;
                color.fG = vec[1] / 255.;
                color.fB = vec[2] / 255.;
                color.fA = vec[3];
                rhs.color = color.toSkColor();
                return true;
            } else if (vec.size() == 3) {
                SkColor4f color;
                color.fR = vec[0] / 255.;
                color.fG = vec[1] / 255.;
                color.fB = vec[2] / 255.;
                color.fA = 1.;
                rhs.color = color.toSkColor();
                return true;
            }
        }
        return false;
    }
};

template<>
struct convert<vector<double>> {
    static bool decode(const Node& node, vector<double>& rhs) {
        if (node.IsScalar()) {
            stringstream ss(node.as<string>());
            double token;
            vector<double> vec;
            while (ss >> token) {
                vec.push_back(token);
            }
            rhs = vec;
            return true;
        }
        return false;
    }
};

blink::EBorderStyle toStyle(string s)
{
    if (s == "none") { return blink::BorderStyleNone; }
    if (s == "hidden") { return blink::BorderStyleHidden; }
    if (s == "inset") { return blink::BorderStyleInset; }
    if (s == "groove") { return blink::BorderStyleGroove; }
    if (s == "outset") { return blink::BorderStyleOutset; }
    if (s == "ridge") { return blink::BorderStyleRidge; }
    if (s == "dotted") { return blink::BorderStyleDotted; }
    if (s == "dashed") { return blink::BorderStyleDashed; }
    if (s == "solid") { return blink::BorderStyleSolid; }
    if (s == "double") { return blink::BorderStyleDouble; }
    assert(false);
    return blink::BorderStyleNone;
}

template<>
struct convert<vector<blink::EBorderStyle>> {
    static bool decode(const Node& node, vector<blink::EBorderStyle>& rhs) {
        vector<blink::EBorderStyle> vec;
        if (node.IsScalar())
            vec.push_back(toStyle(node.as<string>()));
        else for (auto &n : node) {
            vec.push_back(toStyle(n.as<string>()));
        }
        rhs = vec;
        return true;
    }
};


template<>
struct convert<SkRect> {
    static bool decode(const Node& node, SkRect& rhs) {
        if (node.IsScalar()) {
            auto vec = node.as<vector<double>>();
            rhs = SkRect::MakeXYWH(vec[0],
                                   vec[1],
                                   vec[2],
                                   vec[3]);
            return true;
        }
        return false;
    }
};

struct BorderRadius
{
    blink::FloatSize top_left;
    blink::FloatSize top_right;
    blink::FloatSize bottom_left;
    blink::FloatSize bottom_right;
};

template<>
struct convert<blink::FloatSize> {
    static bool decode(const Node& node, blink::FloatSize& rhs) {
        using namespace blink;
        auto s = node.as<vector<double>>();
        rhs = FloatSize(s[0], s[1]);
        return true;
    }
};


template<>
struct convert<BorderRadius> {
    static bool decode(const Node& node, BorderRadius& rhs) {
        using namespace blink;

        if (node.IsScalar()) {
            auto val = node.as<double>();
            rhs.top_left = rhs.top_right = rhs.bottom_left = rhs.bottom_right = FloatSize(val,val);
            return true;
        }
        rhs.top_left = node["top_left"].as<FloatSize>();
        rhs.top_right = node["top_right"].as<FloatSize>();
        rhs.bottom_left = node["bottom_left"].as<FloatSize>();
        rhs.bottom_right = node["bottom_right"].as<FloatSize>();

        return true;
    }
};

}

static string gResPrefix;

string makeResourcePath(const string& rsrc) {
    if (gResPrefix.length() == 0) {
        return rsrc;
    }

    return gResPrefix + "/" + rsrc;
}

void drawText(SkCanvas *c, YAML::Node &item) {
    auto origin = item["origin"].as<vector<double>>();
    SkPaint paint;
    if (item["color"]) {
        auto color = item["color"].as<SkColorW>().color;
        paint.setColor(color);
    }
    if (item["size"]) {
        paint.setTextSize(item["size"].as<double>() * 16. / 12.);
    }

    auto text = item["text"].as<string>();
    paint.setAntiAlias(true);
    c->drawText(text.c_str(), strlen(text.c_str()),
                origin[0],
                origin[1],
                paint);

}
void drawRect(SkCanvas *c, YAML::Node &item) {
    // XXX: handle bounds
    SkRect bounds;
    if (item["rect"])
            bounds = item["rect"].as<SkRect>();
    else
            bounds = item["bounds"].as<SkRect>();
    SkPaint paint;
    if (item["color"]) {
        auto color = item["color"].as<SkColorW>().color;
        paint.setColor(color);
    }
    c->drawRect(bounds, paint);

}

template<typename T>
void broadcast(vector<T> &v, int num_items)
{
    while (v.size() < num_items) {
        v.push_back(v[0]);
    }
}

void drawBorder(SkCanvas *c, YAML::Node &item) {
    using namespace blink;
    SkRect bounds;
    if (item["rect"])
            bounds = item["rect"].as<SkRect>();
    else
            bounds = item["bounds"].as<SkRect>();

    vector<int> widths;
    if (item["width"].IsScalar()) {
        widths.push_back(item["width"].as<int>());
    } else {
        widths = item["width"].as<vector<int>>();
    }
    auto styles = item["style"].as<vector<blink::EBorderStyle>>();
    vector<SkColorW> colors;
    if (item["color"].IsScalar()) {
        colors.push_back(item["color"].as<SkColorW>());
    } else {
        colors = item["color"].as<vector<SkColorW>>();
    }
    broadcast(widths, 4);
    broadcast(styles, 4);
    broadcast(colors, 4);
/*    if (item["color"] && item["color"].IsScalar()) {
        auto color = item["color"].as<SkColorW>().color;
    } else {
*/
    blink::GraphicsContext context(c);
    blink::PaintInfo info(context);
    blink::ComputedStyle::BorderData b;

    b.m_topWidth = widths[0];
    b.m_leftWidth = widths[1];
    b.m_bottomWidth = widths[2];
    b.m_rightWidth = widths[3];

    b.m_topStyle = styles[0];
    b.m_leftStyle = styles[1];
    b.m_bottomStyle = styles[2];
    b.m_rightStyle = styles[3];

    b.m_topColor = colors[0].color;
    b.m_leftColor = colors[1].color;
    b.m_bottomColor = colors[2].color;
    b.m_rightColor = colors[3].color;

    if (item["radius"]) {
        auto radius = item["radius"].as<YAML::BorderRadius>();

        b.m_topLeft = radius.top_left;
        b.m_topRight = radius.top_right;
        b.m_bottomLeft = radius.bottom_left;
        b.m_bottomRight = radius.bottom_right;
    }

    blink::ComputedStyle style;
    style.m_border = b;
    blink::LayoutRect borderRect = bounds;
    BoxBorderPainter painter(borderRect, style, blink::BackgroundBleedNone,
                             true, true);
    painter.paintBorder(info, borderRect);
}

void drawGlyphs(SkCanvas *c, YAML::Node &item) {
    // XXX: handle bounds
    vector<uint16_t> indices;
    vector<SkPoint> offsets;
    for (auto i : item["glyphs"]) {
            indices.push_back(i.as<uint16_t>());
    }
    for (int i = 0; i < item["offsets"].size(); i+= 2) {
            SkPoint p;
            p.fX = item["offsets"][i].as<double>();
            p.fY = item["offsets"][i+1].as<double>();
            offsets.push_back(p);
    }

    SkPaint paint;
    if (item["size"]) {
        paint.setTextSize(item["size"].as<double>() * 16. / 12.);
    }

    SkFontStyle::Weight weight = SkFontStyle::kNormal_Weight;
    if (item["weight"]) {
            weight = (SkFontStyle::Weight)item["weight"].as<int>();
    }

    if (item["family"]) {
        sk_sp<SkTypeface> typeface = SkTypeface::MakeFromName(item["family"].as<string>().c_str(), SkFontStyle(weight,
                                                                                                               SkFontStyle::kNormal_Width,
                                                                                                               SkFontStyle::kUpright_Slant));
        paint.setTypeface(typeface);
    }

    if (item["color"]) {
        auto color = item["color"].as<SkColorW>().color;
        paint.setColor(color);
    }

    assert(indices.size() == offsets.size());
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    paint.setAntiAlias(true);
    c->drawPosText(indices.data(), indices.size()*2, offsets.data(), paint);

}

void drawImage(SkCanvas *c, YAML::Node &node) {
    auto path = makeResourcePath(node["image"].as<string>());
    auto bounds = node["bounds"].as<vector<double>>();
    sk_sp<SkImage> img = GetResourceAsImage(path.c_str());
    c->drawImage(img, bounds[0], bounds[1]);
}
void drawItem(SkCanvas *c, YAML::Node &node);
void drawStackingContext(SkCanvas *c, YAML::Node &node) {
    auto bounds = node["bounds"].as<vector<double>>();
    c->save();
    c->translate(bounds[0], bounds[1]);
    for (auto i : node["items"]) {
        drawItem(c, i);
    }
    c->restore();
}


void drawItem(SkCanvas *c, YAML::Node &node) {
    if (node["text"]) {
        drawText(c, node);
    } else if (node["rect"]) {
        drawRect(c, node);
    } else if (node["image"]) {
        drawImage(c, node);
    } else if (node["glyphs"]) {
        drawGlyphs(c, node);
    } else if (node["stacking_context"]) {
    } else if (node["type"]) {
        auto type = node["type"].as<string>();
        if (type == "stacking_context") {
            drawStackingContext(c, node);
        } else if (type == "rect") {
            drawRect(c, node);
        } else if (type == "border") {
            drawBorder(c, node);
        }

    }

}


YAML::Node loadYAMLFile(const char *filename_raw) {
    string filename(filename_raw);
    auto last_slash = filename.find_last_of("/\\");
    if (last_slash != string::npos) {
        gResPrefix = filename.substr(0, last_slash);
    }
    
    std::ifstream fin(filename);
    YAML::Node doc = YAML::Load(fin);
    return doc;
}

void drawYAMLFile(YAML::Node &doc, SkCanvas *canvas) {
    for (auto i : doc["root"]["items"]) {
        drawItem(canvas, i);
    }
}

