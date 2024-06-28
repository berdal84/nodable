#include "ImGuiEx.h"

#include <algorithm>
#include <cmath>

#include "tools/core/log.h"
#include "tools/core/assertions.h"

#include "tools/core/EventManager.h"
#include "Texture.h"
#include "Color.h"
#include "tools/gui/geometry/Segment.h"

#define DEBUG_BEZIER_ENABLE 0

using namespace tools;

struct Context
{
    bool    debug                 = false;
    bool    is_any_tooltip_open   = false;
    float   tooltip_delay_elapsed = 0.0f;
};

static Context g_ctx = {};

void ImGuiEx::set_debug( bool value )
{
    g_ctx.debug = value;
}

Rect ImGuiEx::GetContentRegion(Space origin)
{
    Rect region{ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax()};

     switch (origin) {
        case PARENT_SPACE:
             return { Vec2(), region.size() };
        case SCREEN_SPACE: {
            region.translate(ImGui::GetWindowPos());
            return region;
        }
        default:
            EXPECT(false,"OriginRef_ case not handled. Cannot compute GetContentRegion(..)")
    }
}

void ImGuiEx::DrawRectShadow (const Vec2& _topLeftCorner, const Vec2& _bottomRightCorner, float _borderRadius, int _shadowRadius, const Vec2& _shadowOffset, const Vec4& _shadowColor)
{
    Vec2 itemRectMin(_topLeftCorner.x + _shadowOffset.x, _topLeftCorner.y + _shadowOffset.y);
    Vec2 itemRectMax(_bottomRightCorner.x + _shadowOffset.x, _bottomRightCorner.y + _shadowOffset.y);
    Vec4 color       = _shadowColor;
    color.w /= float(_shadowRadius);
    auto borderRadius  = _borderRadius;

    // draw N concentric rectangles.
    for(int i = 0; i < _shadowRadius; i++)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(itemRectMin, itemRectMax, ImColor(color));

        itemRectMin.x -= 1.0f;
        itemRectMin.y -= 1.0f;

        itemRectMax.x += 1.0f;
        itemRectMax.y += 1.0f;

        borderRadius += 1.0f;
    }
}

void ImGuiEx::ShadowedText(const Vec2& _offset, const Vec4& _shadowColor, const char* _format, ...)
{
    va_list args;
    va_start(args, _format);
    auto p = ImGui::GetCursorPos();
    // shadow
    ImGui::SetCursorPos(Vec2(p.x + _offset.x, p.y + _offset.y));
    ImGui::TextColored(_shadowColor, _format, args);
    // text
    ImGui::SetCursorPos(p);
    ImGui::Text(_format, args);
    va_end(args);
}

void ImGuiEx::ColoredShadowedText(const Vec2& _offset, const Vec4& _textColor, const Vec4& _shadowColor, const char* _format, ...)
{
    // draw first the shadow
    auto p = ImGui::GetCursorPos();
    ImGui::SetCursorPos(Vec2(p.x + _offset.x, p.y + _offset.y));

    va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
    ImGui::SetCursorPos(p);
    ImGui::TextColored(_textColor, _format, args);
    va_end(args);
}

void ImGuiEx::DrawWire(
        ImGuiID id,
        ImDrawList *draw_list,
        const BezierCurveSegment& curve,
        const WireStyle& style
     )
{
    if ( style.color.z == 0)
        return;

    // 1) determine curves for fill and shadow

    // Line
    // Generate curve
    std::vector<Vec2> fill_path;
    BezierCurveSegment::tesselate(&fill_path, curve);

    if ( fill_path.size() == 1) return;

    // Shadow
    BezierCurveSegment shadow_curve = curve;
    shadow_curve.translate({ 1.f, 1.f });
    shadow_curve.p2 = curve.p2 + Vec2(0.f, 10.f);
    shadow_curve.p3 = curve.p3 + Vec2(0.f, 10.f);

    // Generate curve
    std::vector<Vec2> shadow_path;
    BezierCurveSegment::tesselate(&shadow_path, shadow_curve);

    // 2) draw the shadow

    if ( shadow_path.size() > 1)
        DrawPath(draw_list, &shadow_path, style.shadow_color, style.thickness);

    // 3) draw the curve

    // Mouse behavior
    MultiSegmentLineBehavior(id, &fill_path, BezierCurveSegment::bbox(curve), style.thickness );

    // Draw the path
    if ( ImGui::GetHoveredID() == id )
        DrawPath(draw_list, &fill_path, style.hover_color, CalcSegmentHoverMinDist(style.thickness) * 2.0f); // outline on hover
    DrawPath(draw_list, &fill_path, style.color, style.thickness);
}

bool ImGuiEx::BeginTooltip(float _delay, float _duration)
{
    if ( !ImGui::IsItemHovered() ) return false;

    g_ctx.is_any_tooltip_open = true;
    g_ctx.tooltip_delay_elapsed += ImGui::GetIO().DeltaTime;

    float fade = 0.f;
    if ( g_ctx.tooltip_delay_elapsed >= _delay )
    {
        fade = ( g_ctx.tooltip_delay_elapsed - _delay) / _duration;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, fade);
    ImGui::BeginTooltip();

    return true;
}

void ImGuiEx::EndTooltip()
{
    ImGui::EndTooltip();
    ImGui::PopStyleVar(); // ImGuiStyleVar_Alpha
}

void ImGuiEx::EndFrame()
{
    if( !g_ctx.is_any_tooltip_open )
        g_ctx.tooltip_delay_elapsed = 0.f;
}

void ImGuiEx::NewFrame()
{
    g_ctx.is_any_tooltip_open = false;
}

void ImGuiEx::BulletTextWrapped(const char* str)
{
    ImGui::Bullet(); ImGui::SameLine();
    ImGui::TextWrapped("%s", str);
}

void ImGuiEx::DebugRect(const Vec2& p_min, const Vec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness)
{
#ifdef TOOLS_DEBUG
    if(!g_ctx.debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddRect(p_min, p_max, col, rounding, flags, thickness);
#endif
}

void ImGuiEx::DebugCircle(const Vec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
#ifdef TOOLS_DEBUG
    if(!g_ctx.debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddCircle(center, radius, col, num_segments, thickness);
#endif
}

void ImGuiEx::DebugLine(const Vec2& p1, const Vec2& p2, ImU32 col, float thickness)
{
#ifdef TOOLS_DEBUG
    if(!g_ctx.debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddLine(p1, p2, col, thickness);
#endif
}

void ImGuiEx::Image(Texture* _texture)
{
    ImGui::Image((ImTextureID)_texture->id(), _texture->size());
}

void ImGuiEx::DrawPath(ImDrawList* draw_list, const std::vector<Vec2>* path, const Vec4& color, float thickness)
{
    // Push segments to ImGui's current path
    for(auto& p : *path)
        draw_list->PathLineTo(p + Vec2(0.5f, 0.5f));

    // Draw the path
    draw_list->PathStroke( ImColor(color), 0, thickness);
}

float ImGuiEx::CalcSegmentHoverMinDist(float line_thickness )
{
    return line_thickness < 1.f ? 1.5f
                           : line_thickness * 0.5f + 1.f;
}

void ImGuiEx::MultiSegmentLineBehavior(
    ImGuiID id,
    const std::vector<Vec2>* path,
    Rect bbox,
    float thickness)
{
    if ( path->size() == 1) return;

    const float hover_min_distance = ImGuiEx::CalcSegmentHoverMinDist(thickness);
    bbox.expand(Vec2{hover_min_distance});
    const Vec2 mouse_pos = ImGui::GetMousePos();

#if DEBUG_BEZIER_ENABLE
    DebugRect(bbox.min, bbox.max, ImColor(0,255,127));
#endif
    // bbox vs point
    if ( !Rect::contains( bbox, mouse_pos ) ) return;

    // test each segment
    int i = 0;
    bool hovered = false;
    while( hovered == false && i < path->size() - 1 )
    {
        const float mouse_distance = LineSegment::point_minimum_distance( LineSegment{(*path)[i], (*path)[i+1]}, mouse_pos );
        hovered = mouse_distance < hover_min_distance;
        ++i;
    }

    if( hovered )
    {
        ImGui::SetHoveredID(id);
    }
}