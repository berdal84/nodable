#include "NodeView.h"
#include "Log.h"		// for LOG_DBG(...)
#include <imgui.h>
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"
#include "View.h"
#include "Wire.h"
#include <cmath> // for sinus

using namespace Nodable;

NodeView*   NodeView::s_selected   = nullptr;
NodeView*   NodeView::s_dragged    = nullptr;

DrawMode_   NodeView::s_drawMode   = DrawMode_Default;
DrawDetail_ NodeView::s_drawDetail = DrawDetail_Default;

void NodeView::SetSelected(NodeView* _view)
{
	s_selected = _view;
}

NodeView* NodeView::GetSelected()
{
	return s_selected;
}

void NodeView::SetDragged(NodeView* _view)
{
	s_dragged = _view;
}

NodeView* NodeView::GetDragged()
{
	return s_dragged;
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

void NodeView::arrangeRecursively()
{
	ArrangeRecursively(this, position);
}

void NodeView::update()
{
	// Update opacity to reach 1.0f
	//-----------------------------

	if(opacity < 1.0f)
		opacity += (1.0f - opacity) * 0.05f; // TODO: use frame time

	// Set background color according to node class 
	//---------------------------------------------

	if (dynamic_cast<Node_BinaryOperation*>(node) != nullptr)
		setColor(ImColor(0.7f, 0.7f, 0.9f));
	else if (dynamic_cast<Node_Variable*>(node) != nullptr)
		setColor(ImColor(0.7f, 0.9f, 0.7f));
	else
		setColor(ImColor(0.9f, 0.9f, 0.7f));

	// automatically moves input connected nodes
	//------------------------------------------

	// first we get the spacing distance between nodes sepending on drawDetail global variable

	float spacingDistBase = 150.0f;
	float distances[3]    = {spacingDistBase * 0.3f, spacingDistBase * 0.7f, spacingDistBase * 1.0f};
	float spacingDist     = distances[s_drawDetail];

	// then we constraint each input view

	auto wires            = node->getWires();
	auto inputCount       = node->getInputWireCount();
	auto inputIndex       = 0;

	for(auto eachWire : wires)
	{
		bool isWireAndInput = eachWire->getTarget() == node;

		if (isWireAndInput)
		{
			auto inputView = eachWire->getSource()->getView();

			if ( ! inputView->pinned )
			{
				// Compute new position for this input view
				ImVec2 newPos(position.x - inputView->size.x - spacingDist, position.y);
				if ( inputCount > 1)
					newPos.y += (float(inputIndex)/float(inputCount-1) - 0.5f ) * spacingDist; 

				// Compute a delta to apply to move to this new position
				auto currentPos = inputView->getPosition();
				auto factor     = 0.2f; // TODO: use frame time
				ImVec2 delta( (newPos.x - currentPos.x) * factor,  (newPos.y - currentPos.y) * factor);

				bool isDeltaTooSmall = delta.x*delta.x + delta.y*delta.y < 1.0f;
				if ( !isDeltaTooSmall )
					inputView->translate(delta);
			}

			inputIndex++;
		}
	}
	
}

void NodeView::draw()
{
	// Mouse interactions
	//-------------------

	if (GetDragged() == this && ImGui::IsMouseDragging(0, 0.1f))
	{
		translate(ImGui::GetMouseDragDelta());
		ImGui::ResetMouseDragDelta();
		pinned = true;
	}

	// Begin the window
	//-----------------

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
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
			if ( position.x != -1.0f || position.y != -1.0f)
				ImGui::SetCursorPos(position);
			else
				ImGui::SetCursorPos(ImVec2());

			ImGui::PushID(this);
			ImGui::BeginGroup();
			auto cursor = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor.x + 10.0f, cursor.y + 10.0f));

			// Draw the background of the Group
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			{			
				auto borderCol = IsSelected(this) ? borderColorSelected : getBorderColor();
				auto itemRectMin = position;
				auto itemRectMax = ImVec2(position.x + size.x, position.y + size.y);

				// Draw the rectangle under everything
				View::DrawRectShadow(itemRectMin, itemRectMax, borderRadius, 4, ImVec2(1.0f, 1.0f));
				draw_list->AddRectFilled(itemRectMin, itemRectMax,getColor(), borderRadius);
				draw_list->AddRect(ImVec2(itemRectMin.x + 1.0f, itemRectMin.y + 1.0f), ImVec2(itemRectMax.x, itemRectMax.y),ImColor(1.0f,1.0f,1.0f,0.7f), borderRadius);	
				draw_list->AddRect(itemRectMin, itemRectMax,borderCol, borderRadius);				

				// Darken the bottom area to separate title and details
				if(!collapsed)
					draw_list->AddRectFilled(ImVec2(itemRectMin.x, itemRectMin.y + 35.0f), ImVec2(itemRectMax.x, itemRectMax.y), ImColor(0.0f,0.0f,0.0f, 0.1f), borderRadius, 4);

				// Draw an additionnal rectangle when selected
				if (IsSelected(this))
				{
					float alpha = sin(ImGui::GetTime() * 10.0f)*0.25f + 0.5f;
					float offset = 4.0f;
					draw_list->AddRect(ImVec2(itemRectMin.x - offset, itemRectMin.y - offset), ImVec2(itemRectMax.x + offset, itemRectMax.y + offset), ImColor(1.0f, 1.0f, 1.0f, alpha), borderRadius + offset, ~0, offset / 2.0f);
				}
			}
			break;
		}
	}

	ImGui::PushItemWidth(150.0f);

	// Draw the window content 
	//------------------------

	switch (s_drawMode)
	{
		case DrawMode_AsWindow:
		{
			setPosition(ImGui::GetWindowPos());
			this->size = ImGui::GetWindowSize();
			break;
		}

		case DrawMode_AsGroup:
		{
			break;
		}
	}

	ImGui::Indent();
	ShadowedText(ImVec2(1.0f, 1.0f), ImColor(1.0f,1.0f,1.0f,0.8f), node->getLabel());

	if (!collapsed)
	{
		ImGui::NewLine();

		for(auto& m : node->getMembers())
		{


			switch(m.second->getType())
			{
				case Type_Number:
				{
					float f(m.second->getValueAsNumber());
					if ( ImGui::InputFloat(m.first.c_str(), &f))
					{
						m.second->setValue(f);
						node->setDirty(true);
					}
					break;
				}
				default:
				{
					ImGui::Text("%s", m.first.c_str());
					ImGui::SameLine(100.0f);
					ImGui::Text("%s", m.second->getValueAsString().c_str());
					break;
				}
			}
		}
		

		std::string parentName = "NULL";
		if ( node->getParent() )
			parentName = node->getParent()->getName();
		ImGui::Text("Parent: %s", parentName.c_str());
		
		ImGui::Text("Dirty : %s", node->isDirty() ? "Yes":"No");

		if ( node->isDirty())
		{
			ImGui::SameLine();
			if ( ImGui::Button("update()"))
				node->update();
		}
	}

	ImGui::PopItemWidth();

	// Ends the Window
	//----------------

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
			// Add a margin at the bottom-right corner
			ImGui::EndGroup();

			hovered = ImGui::IsMouseHoveringRect(position, ImVec2(position.x + size.x, position.y + size.y), true);

			// Selection by mouse

			if ( hovered && ImGui::IsMouseClicked(0))
				SetSelected(this);

			// Dragging by mouse

			if ( GetDragged() != this)
			{
				if(GetDragged() == nullptr && ImGui::IsMouseClicked(0) && hovered)
					SetDragged(this);
			}else{				
				if ( ImGui::IsMouseReleased(0))
					SetDragged(nullptr);				
			}		

			// Collapse/uncollapse by double click
			collapsed ^= hovered && ImGui::IsMouseDoubleClicked(0);

			// memorize size with an offet (margin)
			size = ImGui::GetItemRectSize();
			size.x += 20.0f;
			size.y += 10.0f;

			break;
		}
	}

	ImGui::PopStyleVar();
	ImGui::PopID();
}

bool NodeView::isHovered()const
{
	return hovered;
}

void NodeView::ArrangeRecursively(NodeView* _view, ImVec2 _position)
{
	_view->setPosition(_position);

	// Arrange Input Nodes :
	auto wires = _view->getNode()->getWires();
	for(auto eachWire : wires)
	{
		if (eachWire != nullptr && eachWire->getTarget() == _view->getNode())
		{
			auto inputNode = eachWire->getSource();

			if ( inputNode != nullptr)
			{
				auto inputView = inputNode->getView();
				inputView->pinned = false;
				ArrangeRecursively(inputView, inputView->position);
			}
		}
	}
}

