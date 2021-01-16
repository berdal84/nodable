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

		ImVec2 pos0 = View::CursorPosToScreenPos( sourceView->getConnectorPosition(wire->getSource(), Way_Out) );
		ImVec2 pos1 = View::CursorPosToScreenPos( targetView->getConnectorPosition(wire->getTarget(), Way_In) );


	    if (displayArrows) // if arrows are displayed we offset x to see the edge of the arrow.
	    	pos1.x -= 7.0f;

	    // Compute tangents
	    float dist = pos1.y - pos0.y;
	    float positiveDist = dist < 0.0f ? -dist : dist;
        positiveDist = positiveDist < 200.0f ? 200.0f : positiveDist;

	    extern float bezierCurveOutRoundness;
	    extern float bezierCurveInRoundness;
	    extern float bezierThickness;
	    extern bool displayArrows;    
	    ImVec2 arrowSize(8.0f, 12.0f); 

	    ImVec2 cp0(pos0.x , pos0.y + positiveDist * bezierCurveOutRoundness);
	    ImVec2 cp1(pos1.x , pos1.y - positiveDist * bezierCurveInRoundness);

	    // draw bezier curve
	    ImVec2 arrowPos(pos1.x, pos1.y);
	    ImVec2 shadowOffset(1.0f, 2.0f);
	    draw_list->AddBezierCurve(  pos0 + shadowOffset,
	    							cp0  + shadowOffset,
	    							cp1  + shadowOffset,
	    							arrowPos + shadowOffset,
	    							getColor(ColorType_Shadow),
	    							bezierThickness); // shadow

		draw_list->AddBezierCurve(pos0, cp0, cp1, arrowPos, getColor(ColorType_Fill), bezierThickness); // fill
	
		
		// dot at the output position
		draw_list->AddCircleFilled(pos0, connectorRadius, sourceView->getColor(ColorType_Fill));
		draw_list->AddCircle      (pos0, connectorRadius, sourceView->getColor(ColorType_Border));

		if (displayArrows)
		{
			// Arrow at the input position
	    	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y + arrowSize.y/2.0f), arrowPos, getColor(ColorType_Fill), bezierThickness);
	    	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y - arrowSize.y/2.0f), arrowPos, getColor(ColorType_Fill), bezierThickness);
	    }else{        
	    	// dot at the input position
	    	draw_list->AddCircleFilled(pos1, connectorRadius, targetView->getColor(ColorType_Fill));
	    	draw_list->AddCircle      (pos1, connectorRadius, targetView->getColor(ColorType_Border));
	    }


	    // function to draw source and target texts
	    ImGui::SetWindowFontScale(0.5f);
	    auto drawSourceAndTargetTexts = [&](const char* _source, const char* _target)
	    {
	    	float offset = 6.0f;

		    // Draw source text
		    {
		    	auto textSize = ImGui::CalcTextSize(_source);
				ImGui::SetCursorScreenPos(ImVec2(pos0.x + offset, pos0.y + offset));

		    	ColoredShadowedText(ImVec2(1.0f, 1.0f),getColor(ColorType_Fill), getColor(ColorType_Shadow), _source);
			}

		    // Draw target text
		    {
		    	auto textSize = ImGui::CalcTextSize( _target);
				ImGui::SetCursorScreenPos(ImVec2(pos1.x + offset, pos1.y - offset - textSize.y));

		    	ColoredShadowedText(ImVec2(1.0f, 1.0f),getColor(ColorType_Fill),  getColor(ColorType_Shadow), _target);
			}
	    };
        ImGui::SetWindowFontScale(1.0f);

	    // Draw source and target texts depending on DrawDetail_
        auto sourceName = wire->getSource()->getName();
        auto targetName = wire->getTarget()->getName();

	    switch(NodeView::s_drawDetail)
	    {
	    	case DrawDetail_Complex:
	    	{
	    		std::string sourceStr = sourceName + " (" + wire->getSource()->getTypeAsString() + ")";
	    		std::string targetStr = targetName + " (" + wire->getTarget()->getTypeAsString() + ")";
				drawSourceAndTargetTexts(sourceStr.c_str(), targetStr.c_str());
				break;
			}

	    	case DrawDetail_Advanced:
	    	{
                const char *label;
                label = (sourceName.substr(0, 3) + "..").c_str();
				drawSourceAndTargetTexts(label, targetName.c_str());
				break;
			}

			default:
			{
				// Draw nothing by default !
			}
		}
    }  

    return false;
}