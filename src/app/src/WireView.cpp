#include "WireView.h"
#include "NodeView.h"
#include "Wire.h"
#include "Node.h"

#include <imgui/imgui.h>
#include <Settings.h>

using namespace Nodable;

// TODO: move this to ImGuiEx
void WireView::Draw(ImDrawList *draw_list, ImVec2 _from, ImVec2 _to)
{
    auto& wire = Settings::GetCurrent()->ui.wire;

    ImGuiEx::DrawVerticalWire(draw_list, _from, _to,
                               wire.fillColor,
                               wire.shadowColor,
                               wire.bezier.thickness,
                               wire.bezier.roundness);

    // dot at the output position
//    draw_list->AddCircleFilled(_from, settings->ui.node.connectorRadius, _fromNode->getColor(View::ColorType_Fill));
//    draw_list->AddCircle      (_from, settings->ui.node.connectorRadius, _fromNode->getColor(View::ColorType_Border));
//
//    ImVec2 arrowSize(8.0f, 12.0f);
//    if (settings->ui.wire.displayArrows)
//    {
//        // Arrow at the input position
//        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y + arrowSize.y/2.0f), _to, ImColor(settings->ui.wire.fillColor), settings->ui.wire.bezier.thickness);
//        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y - arrowSize.y/2.0f), _to, ImColor(settings->ui.wire.fillColor), settings->ui.wire.bezier.thickness);
//    }
//    else
//    {
//        // dot at the input position
//        draw_list->AddCircleFilled(_to, settings->ui.node.connectorRadius, _toNode->getColor(View::ColorType_Fill));
//        draw_list->AddCircle      (_to, settings->ui.node.connectorRadius, _toNode->getColor(View::ColorType_Border));
//    }

}
