#include "NodeView.h"
#include "Log.h"		// for LOG_DBG(...)
#include <imgui.h>
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"
#include "View.h"

using namespace Nodable;

NodeView* NodeView::s_selected = nullptr;
DrawMode_ NodeView::s_drawMode = DrawMode_Default;

void NodeView::SetSelected(NodeView* _view)
{
	s_selected = _view;
}

NodeView* NodeView::GetSelected()
{
	return s_selected;
}


bool NodeView::IsSelected(NodeView* _view)
{
	return s_selected == _view;
}

NodeView::NodeView(Node* _node)
{
	LOG_DBG("Node::Node()\n");
	this->node = _node;
	this->name = std::string("Node###") + std::to_string((size_t)this);
}

NodeView::~NodeView()
{
}

Node* NodeView::getNode()const
{
	return node;
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

void NodeView::setVisible(bool _b)
{
	visible = _b;
}

void NodeView::translate(ImVec2 _delta)
{
	setPosition(ImVec2(position.x + _delta.x, position.y + _delta.y));
}

void NodeView::arrange()
{
	ArrangeRecursive(this, position);
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

	if (dynamic_cast<Node_BinaryOperation*>(node) != nullptr)
		this->backgroundColor = ImColor(0.7f, 0.7f, 0.9f);

	else if (dynamic_cast<Node_Variable*>(node) != nullptr)
		this->backgroundColor = ImColor(0.7f, 0.9f, 0.7f);
	else
		this->backgroundColor = ImColor(0.9f, 0.9f, 0.7f);
}

void NodeView::imguiBegin()
{

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);	
	
	switch ( s_drawMode)
	{
		case DrawMode_AsWindow:
		{
			ImGui::SetNextWindowSize(size, ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowPos(getPosition(), ImGuiSetCond_FirstUseEver);	
			ImGui::Begin(name.c_str(), &visible, window_flags);		
			break;
		}

		case DrawMode_AsGroup:
		{
			ImGui::SetCursorPos(position);
			ImGui::BeginGroup();
			auto cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + 10.0f, cursor.y + 10.0f));

			// Draw the background of the Group
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			{
				auto color = IsSelected(this) ? borderColorSelected : borderColor;
				auto itemRectMin = position;
				auto itemRectMax = ImVec2(position.x + size.x, position.y + size.y);

				View::DrawRectShadow(itemRectMin, itemRectMax, borderRadius, 4, ImVec2(1.0f, 1.0f));

				draw_list->AddRectFilled(itemRectMin, itemRectMax,backgroundColor, borderRadius);
				draw_list->AddRect(itemRectMin, itemRectMax,color, borderRadius);				

				// Draw an additionnal rectangle when selected
				if (IsSelected(this))
				{
					draw_list->AddRect(ImVec2(itemRectMin.x - 3.0f, itemRectMin.y - 3.0f), ImVec2(itemRectMax.x + 3.0f, itemRectMax.y + 3.0f), ImColor(1.0f, 1.0f, 1.0f, 0.5f), borderRadius + 3.0f, ~0, 3.0f);
				}
			}
			break;
		}
	}

	ImGui::PushItemWidth(150.0f);
}


void NodeView::imguiEnd()
{
	ImGui::PopItemWidth();

	switch (s_drawMode)
	{
		case DrawMode_AsWindow:
		{
			hovered = ImGui::IsWindowHovered();
			ImGui::End();
			break;
		}
		case DrawMode_AsGroup:
		{
			auto cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + 10.0f, cursor.y + 10.0f));ImGui::SameLine(); ImGui::Text(" ");
			ImGui::EndGroup();
			hovered = ImGui::IsItemHoveredRect();

			if ( ! dragged)
				dragged = hovered && ImGui::IsMouseClicked(0);
			else if ( ImGui::IsMouseReleased(0))
				dragged = false;
			
			if ( hovered && ImGui::IsMouseClicked(0))
				SetSelected(this);

			showDetails ^= hovered && ImGui::IsMouseDoubleClicked(0);

			size = ImGui::GetItemRectSize();			

			break;
		}
	}

	ImGui::PopStyleVar();
}


void NodeView::imguiDraw()
{
		// Mouse interactions
	if (dragged)
	{
		translate(ImGui::GetMouseDragDelta());
		ImGui::ResetMouseDragDelta();
	}

	imguiBegin();

	switch (s_drawMode)
	{
		case DrawMode_AsWindow:
		{
			setPosition(ImGui::GetWindowPos());
			break;
		}

		case DrawMode_AsGroup:
		{
			break;
		}
	}

	this->size     = ImGui::GetWindowSize();
	ImGui::Text("%s", node->getLabel());	

	if (showDetails)
	{
		if(node->getInputs() != nullptr)
		{
			ImGui::Text("Inputs:");
			node->getInputs()->drawLabelOnly();
		}

		if(node->getOutputs() != nullptr)
		{
			ImGui::Text("Outputs:");
			node->getOutputs()->drawLabelOnly();
		}

		if(node->getMembers() != nullptr)
		{
			ImGui::Text("Members:");
			node->getMembers()->drawLabelOnly();
		}

		std::string parentName = "NULL";
		if ( node->getParent() )
			parentName = node->getParent()->getName();

		ImGui::Text("Parent: %s", parentName.c_str());
	}

	imguiEnd();	
}

void NodeView::drawWires()
{
	// Draw wires to its output
	auto out = node->getOutputs()->getVariables();

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Compute the origin
    ImVec2 origin;
	switch (s_drawMode)
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

	for(auto each : out)
	{
        // Compute start and end point
        ImVec2 pos0 = getOutputPosition();     
        ImVec2 pos1 = each->getValueAsNode()->getView()->getInputPosition();
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
		draw_list->AddBezierCurve(pos0, cp0, cp1, arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
		
		// dot a the output position
		draw_list->AddCircleFilled(pos0, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		draw_list->AddCircle(pos0, 5.0f, borderColor);

		if (displayArrows)
		{
			// Arrow at the input position
        	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y + arrowSize.y/2.0f), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
        	draw_list->AddLine(ImVec2(arrowPos.x - arrowSize.x, pos1.y - arrowSize.y/2.0f), arrowPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), bezierThickness);
        }else{        
        	// dot at the input position
        	draw_list->AddCircleFilled(pos1, 5.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));   
        	draw_list->AddCircle(pos1, 5.0f, borderColor);
        }     
	}
}

bool NodeView::isHovered()const
{
	return hovered;
}

bool NodeView::isDragged()const
{
	return dragged;
}

void NodeView::ArrangeRecursive(NodeView* _view, ImVec2 _position)
{
	if ( _view == nullptr)
		return;	

	_view->setPosition(_position);

	// Arrange Input Nodes :

	auto inputs = _view->node->getInputs()->getVariables();
	int n = inputs.size();
	for(int i = 0; i < n ; i++ )
	{
		auto inputView = inputs[i]->getValueAsNode()->getView();

		ImVec2 inputPos(_position.x - inputView->size.x - 40.0f, _position.y);

		if ( n > 1)
			inputPos.y += (float(i)/float(n-1) - 0.5f ) * 150.0f; 

		ArrangeRecursive(inputView, inputPos);
	}
}

