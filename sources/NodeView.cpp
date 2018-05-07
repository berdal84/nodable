#include "NodeView.h"
#include "Log.h"		// for LOG_DBG(...)
#include <imgui.h>
#include "Node_Container.h"
#include "Node_Variable.h"

using namespace Nodable;

NodeView::NodeView(Node* _node)
{
	LOG_DBG("Node::Node()\n");
	this->node = _node;
	this->name = std::string("Node##") + std::to_string((size_t)this);
}

NodeView::~NodeView()
{
}

ImVec2 NodeView::getPosition()const
{
	return position;
}

ImVec2 NodeView::getInputPosition()const
{
	auto pos = getPosition();
	return ImVec2(pos.x, pos.y + size.y * 0.5f);
}

ImVec2 NodeView::getOutputPosition()const
{
	auto pos = getPosition();
	return ImVec2(pos.x + size.x, pos.y + size.y * 0.5f);
}

void NodeView::setPosition(ImVec2 _position)
{
	this->position = _position;
	ImGui::SetWindowPos(name.c_str(), _position);
}

void NodeView::translate(ImVec2 _delta)
{
	setPosition(ImVec2(position.x + _delta.x, position.y + _delta.y));
}

void NodeView::draw()
{
	update();

	if ( visible)
		imguiDraw();
}

void NodeView::update()
{
	if(opacity < 1.0f)
		opacity += (1.0f - opacity) * 0.05f;
}

void NodeView::imguiBegin()
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);	
	ImGui::SetNextWindowSize(size, ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowPos(getPosition(), ImGuiSetCond_FirstUseEver);	
	ImGui::Begin(name.c_str(), &visible, window_flags);
	ImGui::PushItemWidth(150.0f);
	hovered = ImGui::IsWindowHovered();
}

void NodeView::imguiDraw()
{
	imguiBegin();
	setPosition(ImGui::GetWindowPos());

	this->size     = ImGui::GetWindowSize();
	ImGui::Text("%s", node->getLabel());
	ImGui::SameLine();
	showDetails ^= ImGui::Button("...", ImVec2(20.0f, 20.0f));
	ImGui::SameLine();
	visible = !ImGui::Button("x", ImVec2(20.0f, 20.0f));

	if (showDetails)
	{
		if(node->getInputs() != nullptr)
		{
			if(ImGui::TreeNodeEx("Inputs", ImGuiTreeNodeFlags_Framed))
			{
				node->getInputs()->drawLabelOnly();
				ImGui::TreePop();
			}
		}

		if(node->getOutputs() != nullptr)
		{
			if(ImGui::TreeNodeEx("Outputs", ImGuiTreeNodeFlags_Framed))
			{
				node->getOutputs()->drawLabelOnly();
				ImGui::TreePop();
			}
		}

		if(node->getMembers() != nullptr)
		{
			if(ImGui::TreeNodeEx("Members", ImGuiTreeNodeFlags_Framed))
			{
				node->getMembers()->drawLabelOnly();
				ImGui::TreePop();
			}
		}

		std::string parentName = "NULL";
		if ( node->getParent() )
			parentName = node->getParent()->getName();

		ImGui::Text("Parent: %s", parentName.c_str());
	}
	imguiEnd();

	// Draw wires to its output
	auto out = node->getOutputs()->getVariables();

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRectFullScreen();

	for(auto each : out)
	{
        // Compute start and end point
        ImVec2 pos0 = getOutputPosition();     
        ImVec2 pos1 = each->getValueAsNode()->getView()->getInputPosition();

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

        // draw brzier curve
        ImVec2 arrowPos(pos1.x - 7.0f, pos1.y);
		draw_list->AddBezierCurve(pos0, cp0, cp1, arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
		
		// dot a the output position
		draw_list->AddCircleFilled(pos0, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));

		if (displayArrows)
		{
			// Arrow at the input position
        	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y + arrowSize.y/2.0f), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
        	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y - arrowSize.y/2.0f), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
        }
        
        // dot at the input position
        draw_list->AddCircleFilled(pos1, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));        
	}
	draw_list->PopClipRect();
}

void NodeView::imguiEnd()
{
	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::PopStyleVar();
}


void NodeView::ArrangeRecursive(NodeView* _view, ImVec2 _position)
{
	if ( _view == nullptr)
		return;	

	_view->setPosition(_position);

	// Draw its inputs
	auto inputs = _view->node->getInputs()->getVariables();
	int n = inputs.size();
	for(int i = 0; i < n ; i++ )
	{
		ImVec2 inputPos = _position;
		inputPos.x -= 190.0f;
		if ( n > 1)
			inputPos.y += (float(i)/float(n-1) - 0.5f ) * 150.0f; 

		ArrangeRecursive(inputs[i]->getValueAsNode()->getView(), inputPos);
	}
}