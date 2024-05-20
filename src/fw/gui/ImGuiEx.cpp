#include "ImGuiEx.h"

#include <algorithm>
#include <cmath>

#include "core/log.h"
#include "core/assertions.h"

#include "Texture.h"
#include "EventManager.h"

using namespace fw;

Rect ImGuiEx::GetContentRegion(Space origin)
{
    Rect region{ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax()};

     switch (origin) {
        case PARENT_SPACE:
             return { Vec2(), region.size() };
        case WORLD_SPACE: {
            region.translate(ImGui::GetWindowPos());
            return region;
        }
        default:
             FW_EXPECT(false,"OriginRef_ case not handled. Cannot compute GetContentRegion(..)")
    }
}

void ImGuiEx::DrawRectShadow (Vec2 _topLeftCorner, Vec2 _bottomRightCorner, float _borderRadius, int _shadowRadius, Vec2 _shadowOffset, Vec4 _shadowColor)
{
    Vec2 itemRectMin(_topLeftCorner.x + _shadowOffset.x, _topLeftCorner.y + _shadowOffset.y);
    Vec2 itemRectMax(_bottomRightCorner.x + _shadowOffset.x, _bottomRightCorner.y + _shadowOffset.y);
    Vec4 color       = _shadowColor;
    color.w /= _shadowRadius;
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

void ImGuiEx::ShadowedText(Vec2 _offset, Vec4 _shadowColor, const char* _format, ...)
{
    // draw first the shadow
    auto p = ImGui::GetCursorPos();
    ImGui::SetCursorPos(Vec2(p.x + _offset.x, p.y + _offset.y));

    va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
    ImGui::SetCursorPos(p);
    ImGui::Text(_format, args);
    va_end(args);
}

void ImGuiEx::ColoredShadowedText(Vec2 _offset, Vec4 _textColor, Vec4 _shadowColor, const char* _format, ...)
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
        ImDrawList *draw_list,
        Vec2 pos0,
        Vec2 pos1,
        Vec2 norm0,
        Vec2 norm1,
        Vec4 color,
        Vec4 shadowColor,
        float thickness,
        float roundness)
{
    // Compute tangents
    Vec2 roundedDist(
        std::abs( pos1.x - pos0.x ) * roundness,
        std::abs( pos1.y - pos0.y ) * roundness);

    Vec2 cp0_fill(pos0 + norm0 * roundedDist);
    Vec2 cp1_fill(pos1 + norm1 * roundedDist);

    Vec2 pos_shadow_offset(1.f, 1.f);
    Vec2 pos0_shadow(pos0 + pos_shadow_offset);
    Vec2 pos1_shadow(pos1 + pos_shadow_offset);
    Vec2 cp0_shadow(pos0_shadow + norm0 * roundedDist * 1.05f);
    Vec2 cp1_shadow(pos1_shadow + norm1 * roundedDist  * 0.95f);

    // shadow
    draw_list->AddBezierCurve( pos0_shadow, cp0_shadow, cp1_shadow, pos1_shadow, ImColor(shadowColor), thickness);
    // fill
    draw_list->AddBezierCurve( pos0, cp0_fill, cp1_fill, pos1, ImColor(color), thickness);
}

void ImGuiEx::DrawVerticalWire(
        ImDrawList *draw_list,
        Vec2 pos0,
        Vec2 pos1,
        Vec4 color,
        Vec4 shadowColor,
        float thickness,
        float roundness)
{
    DrawWire(
            draw_list,
            pos0,
            pos1,
            Vec2(0.0f, 1.0f),
            Vec2(0.0f, -1.0f),
            color,
            shadowColor,
            thickness,
            roundness );
}

void ImGuiEx::DrawHorizontalWire(
        ImDrawList *draw_list,
        Vec2 pos0,
        Vec2 pos1,
        Vec4 color,
        Vec4 shadowColor,
        float thickness,
        float roundness)
{

    // Compute tangents
    float dist = std::max(std::abs(pos1.y - pos0.y), 200.0f);

    Vec2 cp0(pos0.x + dist * roundness , pos0.y );
    Vec2 cp1(pos1.x - dist * roundness , pos1.y );

    // draw bezier curve
    Vec2 shadowOffset(1.0f, 2.0f);
    draw_list->AddBezierCurve(  pos0 + shadowOffset,
                                cp0  + shadowOffset,
                                cp1  + shadowOffset,
                                pos1 + shadowOffset,
                                ImColor(shadowColor),
                                thickness); // shadow

    draw_list->AddBezierCurve(pos0, cp0, cp1, pos1, ImColor(color), thickness); // fill
}

bool ImGuiEx::BeginTooltip(float _delay, float _duration)
{
    if ( !ImGui::IsItemHovered() ) return false;

    FW_EXPECT( is_in_a_frame, "Did you forgot to call ImGuiEx::BeginFrame/EndFrame ?");

    is_any_tooltip_open = true;
    tooltip_delay_elapsed += ImGui::GetIO().DeltaTime;

    float fade = 0.f;
    if ( tooltip_delay_elapsed >= _delay )
    {
        fade = ( tooltip_delay_elapsed - _delay) / _duration;
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
    FW_EXPECT( is_in_a_frame, "ImGuiEx::BeginFrame/EndFrame mismatch");
    if( !is_any_tooltip_open )
    {
        tooltip_delay_elapsed = 0.f;
    }
    is_in_a_frame = false;
}

void ImGuiEx::BeginFrame()
{
    FW_EXPECT(!is_in_a_frame, "ImGuiEx::BeginFrame/EndFrame mismatch");
    is_in_a_frame = true;
    is_any_tooltip_open = false;
}

void ImGuiEx::BulletTextWrapped(const char* str)
{
    ImGui::Bullet(); ImGui::SameLine();
    ImGui::TextWrapped("%s", str);
}

void ImGuiEx::DebugRect(const Vec2& p_min, const Vec2& p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness)
{
#ifdef FW_DEBUG
    if(!debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddRect(p_min, p_max, col, rounding, flags, thickness);
#endif
}

void ImGuiEx::DebugCircle(const Vec2& center, float radius, ImU32 col, int num_segments, float thickness)
{
#ifdef FW_DEBUG
    if(!debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddCircle(center, radius, col, num_segments, thickness);
#endif
}

void ImGuiEx::DebugLine(const Vec2& p1, const Vec2& p2, ImU32 col, float thickness)
{
#ifdef FW_DEBUG
    if(!debug) return;
    ImDrawList* list = ImGui::GetForegroundDrawList();
    list->AddLine(p1, p2, col, thickness);
#endif
}

void ImGuiEx::Image(Texture* _texture)
{
    ImGui::Image((ImTextureID)_texture->id(), _texture->size());
}
