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

	    auto sourceName = wire->getSource()->getName();
	    auto targetName = wire->getTarget()->getName();

		ImVec2 pos0 = View::CursorPosToScreenPos( sourceView->getConnectorPosition(sourceName, Way::Out) );
		ImVec2 pos1 = View::CursorPosToScreenPos( targetView->getConnectorPosition(targetName, Way::In) );


	    if (displayArrows) // if arrows are displayed we offset x to see the edge of the arrow.
	    	pos1.x -= 7.0f;

	    // Compute tangents
	    float distX = pos1.x - pos0.x;
	    float positiveDistX = distX < 0.0f ? -distX : distX;
	    positiveDistX = positiveDistX < 200.0f ? 200.0f : positiveDistX;        

	    extern float bezierCurveOutRoundness;
	    extern float bezierCurveInRoundness;
	    extern float bezierThickness;
	    extern bool displayArrows;    
	    ImVec2 arrowSize(8.0f, 12.0f); 

	    ImVec2 cp0(pos0.x + positiveDistX*bezierCurveOutRoundness, pos0.y);
	    ImVec2 cp1(pos1.x - positiveDistX*bezierCurveInRoundness, pos1.y);

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
	    auto drawSourceAndTargetTexts = [&](const char* _source, const char* _target)
	    {
	    	float offsetX = 6.0f;

		    // Draw source text
		    {
		    	auto textSize = ImGui::CalcTextSize(_source);
				ImGui::SetCursorScreenPos(ImVec2(pos0.x + offsetX, pos0.y ));

		    	ColoredShadowedText(ImVec2(1.0f, 1.0f),getColor(ColorType_Fill), getColor(ColorType_Shadow), _source);
			}

		    // Draw target text
		    {
		    	auto textSize = ImGui::CalcTextSize( _target);
				ImGui::SetCursorScreenPos(ImVec2(pos1.x - offsetX - textSize.x, pos1.y));

		    	ColoredShadowedText(ImVec2(1.0f, 1.0f),getColor(ColorType_Fill),  getColor(ColorType_Shadow), _target);
			}
	    };

	    // Draw source and target texts depending on DrawDetail_
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
				drawSourceAndTargetTexts(sourceName.c_str(), targetName.c_str());
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