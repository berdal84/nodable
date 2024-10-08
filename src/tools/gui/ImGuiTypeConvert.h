#pragma once

#include "geometry/Rect.h"
#include "geometry/Vec2.h"
#include "geometry/Vec4.h"
#include "imgui_internal.h"

namespace tools
{
    inline ImRect toImGui(const Rect& r) { return { r.min, r.max }; };
    inline ImVec2 toImGui(const Vec2& v) { return { v.x, v.y }; };
    inline ImVec4 toImGui(const Vec4& v) { return { v.x, v.y, v.z, v.w }; };
}