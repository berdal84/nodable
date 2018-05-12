#include "WireView.h"
#include "NodeView.h"
#include "Wire.h"
#include "Node.h"

#include <imgui.h>

using namespace Nodable;

WireView::WireView(Wire* _wire)
{
	wire = _wire;
}

WireView::~WireView()
{
}

void WireView::draw()
{

	switch( wire->getState())
	{
		case Wire::State_Disconnected:
		{
			setColor(ImColor(1.0f, 0.0f, 0.0f));
			break;
		}

		case Wire::State_Misconnected:
		{
			setColor(ImColor(1.0f, 0.0f, 0.0f));
			break;
		}

		case Wire::State_Connected:
		{
			setColor(ImColor(0.8f, 0.8f, 0.8f));
			break;
		}
	}


	auto source = wire->getSource();
	auto target = wire->getTarget();

	if ( source != nullptr || target != nullptr )
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

	    // Compute the origin
	    ImVec2 origin;
		switch (NodeView::s_drawMode)
		{
			case DrawMode_AsWindow:
			{
				origin = ImVec2();
				break;
			}

			case DrawMode_AsGroup:
			{
				origin = ImGui::GetWindowPos();
				break;
			}
		}


	    // Compute start and end point
	    ImVec2 pos0 = source->getView()->getOutputPosition();     
	    ImVec2 pos1 = target->getView()->getInputPosition();
	    pos0.x += origin.x; pos0.y += origin.y;
	    pos1.x += origin.x; pos1.y += origin.y;

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
		draw_list->AddBezierCurve(pos0, cp0, cp1, arrowPos, getColor(), bezierThickness);
		
		// dot a the output position
		draw_list->AddCircleFilled(pos0, 5.0f, source->getView()->getColor());
		draw_list->AddCircle      (pos0, 5.0f, source->getView()->getBorderColor());

		if (displayArrows)
		{
			// Arrow at the input position
	    	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y + arrowSize.y/2.0f), arrowPos, getColor(), bezierThickness);
	    	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y - arrowSize.y/2.0f), arrowPos, getColor(), bezierThickness);
	    }else{        
	    	// dot at the input position
	    	draw_list->AddCircleFilled(pos1, 5.0f, target->getView()->getColor());   
	    	draw_list->AddCircle      (pos1, 5.0f, target->getView()->getBorderColor());
	    }

	    switch(NodeView::s_drawDetail)
	    {
	    	case DrawDetail_Complex:
	    	{
			    // Draw source text
			    {
			    	std::string s = std::string(wire->getSourceSlot()) + " (" + wire->getSourceSlotTypeAsString() + ")";
			    	auto textSize = ImGui::CalcTextSize( s.c_str());
					ImGui::SetCursorPos(ImVec2(pos0.x + 10.0f, pos0.y - textSize.y));
			    	ImGui::TextColored(getColor(), "%s", s.c_str());
				}

			    // Draw target text
			    {
			    	std::string s = std::string(wire->getTargetSlot()) + "  (" + wire->getTargetSlotTypeAsString() + ")";
			    	auto textSize = ImGui::CalcTextSize( s.c_str());
					ImGui::SetCursorPos(ImVec2(pos1.x - textSize.x - 10.0f, pos1.y));
			    	ImGui::TextColored(getColor(), "%s", s.c_str());
				}
				break;
			}

			case DrawDetail_Advanced:
	    	{
			    // Draw source text
			    {
			    	std::string s = std::string(wire->getSourceSlot());
			    	auto textSize = ImGui::CalcTextSize( s.c_str());
					ImGui::SetCursorPos(ImVec2(pos0.x + 10.0f, pos0.y - textSize.y));
			    	ImGui::TextColored(getColor(), "%s", s.c_str());
				}

			    // Draw target text
			    {
			    	std::string s = std::string(wire->getTargetSlot());
			    	auto textSize = ImGui::CalcTextSize( s.c_str());
					ImGui::SetCursorPos(ImVec2(pos1.x - textSize.x - 10.0f, pos1.y));
			    	ImGui::TextColored(getColor(), "%s", s.c_str());
				}
				break;
			}
			default:{}
		}
    }  

}