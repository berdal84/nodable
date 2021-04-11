#include "WireView.h"
#include "NodeView.h"
#include "Wire.h"
#include "Node.h"

#include <imgui/imgui.h>
#include <Settings.h>

using namespace Nodable;

void WireView::Draw(
        ImDrawList *draw_list,
        ImVec2 _from, ImVec2 _to,
        const NodeView *_fromNode, const NodeView *_toNode)
{
    auto settings = Settings::GetCurrent();

    WireView::DrawVerticalWire(draw_list, _from, _to, settings->ui.wire.fillColor, settings->ui.wire.shadowColor,
                               settings->ui.wire.bezier.thickness,
                               settings->ui.wire.bezier.roundness);

    // dot at the output position
    draw_list->AddCircleFilled(_from, settings->ui.node.connectorRadius, _fromNode->getColor(View::ColorType_Fill));
    draw_list->AddCircle      (_from, settings->ui.node.connectorRadius, _fromNode->getColor(View::ColorType_Border));

    ImVec2 arrowSize(8.0f, 12.0f);
    if (settings->ui.wire.displayArrows)
    {
        // Arrow at the input position
        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y + arrowSize.y/2.0f), _to, ImColor(settings->ui.wire.fillColor), settings->ui.wire.bezier.thickness);
        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y - arrowSize.y/2.0f), _to, ImColor(settings->ui.wire.fillColor), settings->ui.wire.bezier.thickness);
    }
    else
    {
        // dot at the input position
        draw_list->AddCircleFilled(_to, settings->ui.node.connectorRadius, _toNode->getColor(View::ColorType_Fill));
        draw_list->AddCircle      (_to, settings->ui.node.connectorRadius, _toNode->getColor(View::ColorType_Border));
    }

}

void WireView::DrawVerticalWire(
        ImDrawList *draw_list,
        ImVec2 pos0,
        ImVec2 pos1,
        ImColor color,
        ImColor shadowColor,
        float thickness,
        float roundness)
{
    // Compute tangents
    float dist = std::abs(pos1.y - pos0.y);

    ImVec2 cp0(pos0.x , pos0.y + dist * roundness);
    ImVec2 cp1(pos1.x , pos1.y - dist * roundness);

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

void WireView::DrawHorizontalWire(
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
