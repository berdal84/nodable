#include "WireView.h"
#include "NodeView.h"
#include "Wire.h"
#include "Node.h"

#include <imgui/imgui.h>

using namespace Nodable;

bool WireView::draw()
{
	auto wire = getOwner()->as<Wire>();
	NODABLE_ASSERT(wire != nullptr);

	// Update fill color depending on current state 
	ImColor stateColors[Wire::State_COUNT] = {ImColor(1.0f, 0.0f, 0.0f), ImColor(0.8f, 0.8f, 0.8f)};
	setColor(ColorType_Fill, stateColors[wire->getState()]);

	// draw the wire
	auto source = wire->getSource();
	auto target = wire->getTarget();

	if ( source && target )
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

	    // Compute start and end point
	    auto sourceNode 	= source->getOwner()->as<Node>();
		auto targetNode 	= target->getOwner()->as<Node>();

		if (!sourceNode->hasComponent<View>() || // in case of of the node have no view we can't draw the wire.
			!targetNode->hasComponent<View>() )
			return false;

	    auto sourceView = sourceNode->getComponent<NodeView>();
	    auto targetView = targetNode->getComponent<NodeView>();

		if (!sourceView->isVisible() || !targetView->isVisible() ) // in case of of the node have hidden view we can't draw the wire.
			return false;

		ImVec2 pos0 = sourceView->getConnectorPosition(wire->getSource(), Way_Out);
		ImVec2 pos1 = targetView->getConnectorPosition(wire->getTarget(), Way_In);

        WireView::DrawLine(draw_list, pos0, pos1, getColor(ColorType_Fill), getColor(ColorType_Shadow));

        // dot at the output position
        draw_list->AddCircleFilled(pos0, connectorRadius, sourceView->getColor(ColorType_Fill));
        draw_list->AddCircle      (pos0, connectorRadius, sourceView->getColor(ColorType_Border));

        ImVec2 arrowSize(8.0f, 12.0f);
        if (displayArrows)
        {
            // Arrow at the input position
            draw_list->AddLine(ImVec2(pos1.x - arrowSize.x, pos1.y + arrowSize.y/2.0f), pos1, getColor(ColorType_Fill), bezierThickness);
            draw_list->AddLine(ImVec2(pos1.x - arrowSize.x, pos1.y - arrowSize.y/2.0f), pos1, getColor(ColorType_Fill), bezierThickness);
        }
        else
        {
            // dot at the input position
            draw_list->AddCircleFilled(pos1, connectorRadius, targetView->getColor(ColorType_Fill));
            draw_list->AddCircle      (pos1, connectorRadius, targetView->getColor(ColorType_Border));
        }
    }

    return false;
}

void WireView::DrawLine(ImDrawList *draw_list, ImVec2 pos0, ImVec2 pos1, ImColor color, ImColor shadowColor)
{
    if (displayArrows) // if arrows are displayed we offset x to see the edge of the arrow.
        pos1.x -= 7.0f;

    // Compute tangents
    float dist = pos1.y - pos0.y;
    float positiveDist = dist < 0.0f ? -dist : dist;
    positiveDist = positiveDist < 200.0f ? 200.0f : positiveDist;

    ImVec2 cp0(pos0.x , pos0.y + positiveDist * bezierCurveOutRoundness);
    ImVec2 cp1(pos1.x , pos1.y - positiveDist * bezierCurveInRoundness);

    // draw bezier curve
    ImVec2 shadowOffset(1.0f, 2.0f);
    draw_list->AddBezierCurve(  pos0 + shadowOffset,
                                cp0  + shadowOffset,
                                cp1  + shadowOffset,
                                pos1 + shadowOffset,
                                shadowColor,
                                bezierThickness); // shadow

    draw_list->AddBezierCurve(pos0, cp0, cp1, pos1, color, bezierThickness); // fill
}
