#pragma once

#include "tools/core/geometry/Vec2.h"
#include "tools/core/geometry/Vec4.h"
#include "tools/core/geometry/Box2D.h"

//---- Use stb_truetype to build and rasterize the font atlas (default)
// The only purpose of this define is if you want force compilation of the stb_truetype backend ALONG with the FreeType backend.
#define IMGUI_ENABLE_STB_TRUETYPE

//---- Define constructor and implicit cast operators to convert back<>forth between your math types and ImVec2/ImVec4.
// This will be inlined as part of ImVec2 and ImVec4 class declarations.

#define IM_VEC2_CLASS_EXTRA \
        ImVec2( const tools::Vec2& v ): ImVec2(v.x, v.y) {} \
        ImVec2( const float f): ImVec2(f, f) {} \
        operator tools::Vec2() const { return {x,y}; }

#define IM_VEC4_CLASS_EXTRA \
        ImVec4(const tools::Vec4& v): ImVec4(v.x, v.y, v.z, v.w) {} \
        operator tools::Vec4() const { return {x,y,z,w}; }

#define IMGUI_DEFINE_MATH_OPERATORS