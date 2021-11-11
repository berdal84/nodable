#include <nodable/WireView.h>

#include <imgui/imgui.h>

#include <nodable/Node.h>
#include <nodable/Settings.h>

using namespace Nodable;

// TODO: move this to ImGuiEx
void WireView::Draw(ImDrawList *draw_list, ImVec2 _from, ImVec2 _to)
{
    auto conf = Settings::Get();

    ImGuiEx::DrawVerticalWire(draw_list, _from, _to,
                              conf->ui_wire_fillColor,
                              conf->ui_wire_shadowColor,
                              conf->ui_wire_bezier_thickness,
                              conf->ui_wire_bezier_roundness);

    // dot at the output position
//    draw_list->AddCircleFilled(_from, settings->ui_node.connectorRadius, _fromNode->getColor(View::ColorType_Fill));
//    draw_list->AddCircle      (_from, settings->ui_node.connectorRadius, _fromNode->getColor(View::ColorType_Border));
//
//    ImVec2 arrowSize(8.0f, 12.0f);
//    if (settings->ui_wire.displayArrows)
//    {
//        // Arrow at the input position
//        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y + arrowSize.y/2.0f), _to, ImColor(settings->ui_wire.fillColor), settings->ui_wire.bezier.thickness);
//        draw_list->AddLine(ImVec2(_to.x - arrowSize.x, _to.y - arrowSize.y/2.0f), _to, ImColor(settings->ui_wire.fillColor), settings->ui_wire.bezier.thickness);
//    }
//    else
//    {
//        // dot at the input position
//        draw_list->AddCircleFilled(_to, settings->ui_node.connectorRadius, _toNode->getColor(View::ColorType_Fill));
//        draw_list->AddCircle      (_to, settings->ui_node.connectorRadius, _toNode->getColor(View::ColorType_Border));
//    }

}
