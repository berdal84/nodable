#include "ImGuiEx.h"

ImVec2 ImGuiEx::CursorPosToScreenPos(ImVec2 _position)
{
    return _position + ToScreenPosOffset();
}

ImVec2 ImGuiEx::ToScreenPosOffset()
{
    return ImGui::GetCursorScreenPos() - ImGui::GetCursorPos();
}


void ImGuiEx::DrawRectShadow (ImVec2 _topLeftCorner, ImVec2 _bottomRightCorner, float _borderRadius, int _shadowRadius, ImVec2 _shadowOffset, ImColor _shadowColor)
{
    ImVec2 itemRectMin(_topLeftCorner.x + _shadowOffset.x, _topLeftCorner.y + _shadowOffset.y);
    ImVec2 itemRectMax(_bottomRightCorner.x + _shadowOffset.x, _bottomRightCorner.y + _shadowOffset.y);
    ImVec4 color       = _shadowColor;
    color.w /= _shadowRadius;
    auto borderRadius  = _borderRadius;

    // draw N concentric rectangles.
    for(int i = 0; i < _shadowRadius; i++)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(itemRectMin, itemRectMax, ImColor(color), borderRadius);

        itemRectMin.x -= 1.0f;
        itemRectMin.y -= 1.0f;

        itemRectMax.x += 1.0f;
        itemRectMax.y += 1.0f;

        borderRadius += 1.0f;
    }
}

void ImGuiEx::ShadowedText(ImVec2 _offset, ImColor _shadowColor, const char* _format, ...)
{
    // draw first the shadow
    auto p = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(p.x + _offset.x, p.y + _offset.y));

    va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
    ImGui::SetCursorPos(p);
    ImGui::Text(_format, args);
    va_end(args);
}

void ImGuiEx::ColoredShadowedText(ImVec2 _offset, ImColor _textColor, ImColor _shadowColor, const char* _format, ...)
{
    // draw first the shadow
    auto p = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(p.x + _offset.x, p.y + _offset.y));

    va_list args;
    va_start(args, _format);
    ImGui::TextColored(_shadowColor, _format, args);
    ImGui::SetCursorPos(p);
    ImGui::TextColored(_textColor, _format, args);
    va_end(args);
}

void ImGuiEx::DrawVerticalWire(
        ImDrawList *draw_list,
        ImVec2 pos0,
        ImVec2 pos1,
        ImColor color,
        ImColor shadowColor,
        float thickness,
        float roundness)
{
    // Compute tangents
    float roundedDist = std::abs(pos1.y - pos0.y) * roundness;

    ImVec2 cp0_fill(pos0.x , pos0.y + roundedDist);
    ImVec2 cp1_fill(pos1.x , pos1.y - roundedDist);

    ImVec2 pos_shadow_offset(1.f, 1.f);
    ImVec2 pos0_shadow(pos0 + pos_shadow_offset);
    ImVec2 pos1_shadow(pos1 + pos_shadow_offset);
    ImVec2 cp0_shadow(pos0_shadow.x , pos0_shadow.y + roundedDist * 1.05f);
    ImVec2 cp1_shadow(pos1_shadow.x , pos1_shadow.y - roundedDist * 0.95f);

    // shadow
    draw_list->AddBezierCurve( pos0_shadow, cp0_shadow, cp1_shadow, pos1_shadow, shadowColor, thickness);
    // fill
    draw_list->AddBezierCurve( pos0, cp0_fill, cp1_fill, pos1, color, thickness);
}

void ImGuiEx::DrawHorizontalWire(
        ImDrawList *draw_list,
        ImVec2 pos0,
        ImVec2 pos1,
        ImColor color,
        ImColor shadowColor,
        float thickness,
        float roundness)
{

    // Compute tangents
    float dist = std::max(std::abs(pos1.y - pos0.y), 200.0f);

    ImVec2 cp0(pos0.x + dist * roundness , pos0.y );
    ImVec2 cp1(pos1.x - dist * roundness , pos1.y );

    // draw bezier curve
    ImVec2 shadowOffset(1.0f, 2.0f);
    draw_list->AddBezierCurve(  pos0 + shadowOffset,
                                cp0  + shadowOffset,
                                cp1  + shadowOffset,
                                pos1 + shadowOffset,
                                shadowColor,
                                thickness); // shadow

    draw_list->AddBezierCurve(pos0, cp0, cp1, pos1, color, thickness); // fill
}
