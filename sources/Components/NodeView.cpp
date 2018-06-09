#include "NodeView.h"
#include "Log.h"		          // for LOG_DBG(...)
#include <imgui/imgui.h>
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include "Application.h"

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

ImVec2 NodeView::getPosition()const
{
	return ImVec2(position.x - size.x / 2.0f, position.y - size.y / 2.0f);
}

ImVec2 NodeView::getInputPosition(const std::string& _name)const
{
	auto pos = getPosition();

	auto it = membersOffsetPositionY.find(_name);
	if(it != membersOffsetPositionY.end())
		pos.y += (*it).second;

	return ImVec2(pos.x, pos.y + size.y * 0.5f);
}

ImVec2 NodeView::getOutputPosition(const std::string& _name)const
{
	auto pos = getPosition();

	auto it = membersOffsetPositionY.find(_name);
	if(it != membersOffsetPositionY.end())
		pos.y += (*it).second;

	return ImVec2(pos.x + size.x, pos.y + size.y * 0.5f);
}

void NodeView::setPosition(ImVec2 _position)
{
	this->position = _position;
	ImGui::SetWindowPos(std::to_string(size_t(this)).c_str(), _position);
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
	auto node = getOwner();
	NODABLE_ASSERT(node != nullptr);

	if (node->hasComponent("operation"))
		setColor(ColorType_Fill, ImColor(0.7f, 0.7f, 0.9f));
	else if (dynamic_cast<Variable*>(node) != nullptr)
		setColor(ColorType_Fill, ImColor(0.7f, 0.9f, 0.7f));
	else
		setColor(ColorType_Fill, ImColor(0.9f, 0.9f, 0.7f));

	// automatically moves input connected nodes
	//------------------------------------------

	// first we get the spacing distance between nodes sepending on drawDetail global variable

	float spacingDistBase = 150.0f;
	float distances[3]    = {spacingDistBase * 0.3f, spacingDistBase * 0.5f, spacingDistBase * 1.0f};
	float spacingDist     = distances[s_drawDetail];

	// then we constraint each input view

	auto wires            = node->getWires();
	auto inputIndex       = 0;

	// Compute the cumulated height and the size x max of the input node view:
	auto cumulatedHeight = 0.0f;
	auto maxSizeX        = 0.0f;
	for(auto eachWire : wires)
	{
		auto sourceNode    = eachWire->getSource()->getOwner()->getAs<Entity*>();
		bool isWireAnInput = node->hasMember(eachWire->getTarget());
		auto inputView     = (NodeView*)sourceNode->getComponent("view");
		if (isWireAnInput && !inputView->pinned )
		{
			cumulatedHeight += inputView->size.y;
			maxSizeX = std::max(maxSizeX, inputView->size.x);
		}
	}

	// Move each input node views :
	auto posY = getInputPosition("").y - cumulatedHeight / 2.0f;

	for(auto eachWire : wires)
	{
		bool isWireAnInput = node->hasMember(eachWire->getTarget());
		if (isWireAnInput)
		{
			auto sourceNode    = eachWire->getSource()->getOwner()->getAs<Entity*>();
			auto inputView     = (NodeView*)sourceNode->getComponent("view");

			if ( ! inputView->pinned )
			{
				// Compute new position for this input view
				ImVec2 newPos(getInputPosition("").x - maxSizeX - spacingDist, posY);

				// Compute a delta to apply to move to this new position
				auto currentPos = inputView->getPosition();
				auto factor     = 0.2f; // TODO: use frame time
				ImVec2 delta( (newPos.x - currentPos.x) * factor,  (newPos.y - currentPos.y) * factor);

				bool isDeltaTooSmall = delta.x*delta.x + delta.y*delta.y < 1.0f;
				if ( !isDeltaTooSmall )
					inputView->translate(delta);

				posY += inputView->size.y + 15.0f; // adding a 10 px vertical margin.
			}

			inputIndex++;
		}
	}
	
}

void NodeView::draw()
{
	auto node = getOwner();
	NODABLE_ASSERT(node != nullptr);

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

	ImVec2 screenPosition = position;

	switch ( s_drawMode)
	{
		case DrawMode_AsWindow:
		{
			ImGui::SetNextWindowSize(size, ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowPos(getPosition(), ImGuiSetCond_FirstUseEver);	
			ImGui::Begin(std::to_string(size_t(this)).c_str(), &visible, window_flags);		
			break;
		}

		case DrawMode_AsGroup:
		{
			if ( position.x != -1.0f || position.y != -1.0f)
				ImGui::SetCursorPos(ImVec2(std::floor(position.x - size.x/2.0f), std::floor(position.y - size.y / 2.0f)));
			else
				ImGui::SetCursorPos(ImVec2());

			ImGui::PushID(this);
			ImGui::BeginGroup();
			ImVec2 cursorPos       = ImGui::GetCursorPos();
			ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
			screenPosition.x = position.x +  cursorScreenPos.x - cursorPos.x;
			screenPosition.y = position.y +  cursorScreenPos.y - cursorPos.y;

			ImGui::SetCursorPos(ImVec2(cursorPos.x + 10.0f, cursorPos.y + 10.0f));

			// Draw the background of the Group
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			{			
				auto borderCol = IsSelected(this) ? borderColorSelected : getColor(ColorType_Border);

				auto itemRectMin = ImVec2(screenPosition.x - size.x/2.0f, screenPosition.y - size.y/2.0f);
				auto itemRectMax = ImVec2(screenPosition.x + size.x/2.0f, screenPosition.y + size.y/2.0f);

				// Draw the rectangle under everything
				View::DrawRectShadow(itemRectMin, itemRectMax, borderRadius, 4, ImVec2(1.0f, 1.0f), getColor(ColorType_Shadow));
				draw_list->AddRectFilled(itemRectMin, itemRectMax,getColor(ColorType_Fill), borderRadius);
				draw_list->AddRect(ImVec2(itemRectMin.x + 1.0f, itemRectMin.y + 1.0f), ImVec2(itemRectMax.x, itemRectMax.y),getColor(ColorType_BorderHighlights), borderRadius);	
				draw_list->AddRect(itemRectMin, itemRectMax,borderCol, borderRadius);				

				// darken the background under the content
				draw_list->AddRectFilled(ImVec2(itemRectMin.x, itemRectMin.y + 35.0f), ImVec2(itemRectMax.x, itemRectMax.y), ImColor(0.0f,0.0f,0.0f, 0.1f), borderRadius, 4);

				// Draw an additionnal blinking rectangle when selected
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

	ImGui::PushItemWidth(100.0f);

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

		default:{}
	}

	ImGui::Indent();
	ShadowedText(ImVec2(1.0f, 1.0f), getColor(ColorType_BorderHighlights), node->getLabel()); // text with a lighter shadow (incrust effect)

	ImGui::NewLine();

	membersOffsetPositionY.clear();
	auto drawValue = [&](Value* _v)->void
	{
		auto memberTopPositionOffsetY 	= ImGui::GetCursorPos().y - position.y;

		switch(_v->getType())
		{
			case Type_Number:
			{
				float f(_v->getValueAsNumber());
				if ( ImGui::InputFloat(_v->getName().c_str(), &f))
				{
					_v->setValue(f);
					node->setDirty(true);
				}
				break;
			}
			default:
			{
				ImGui::Text("%s", _v->getName().c_str());
				ImGui::SameLine(100.0f);
				ImGui::Text("%s", _v->getValueAsString().c_str());
				break;
			}
		}
		
		/* If value is hovered, we draw a tooltip that print the source expression of the value*/
		if( ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Source expression: \"%s\"", _v->getSourceExpression().c_str());
			ImGui::EndTooltip();
		}

		auto memberBottomPositionOffsetY = ImGui::GetCursorPos().y - position.y;
		membersOffsetPositionY[_v->getName()] = (memberTopPositionOffsetY + memberBottomPositionOffsetY) / 2.0f;
	};

	// Draw visible members
	{
		for(auto& m : node->getMembers())
		{		
			if( m.second->getVisibility() == Visibility_Public)
			{
				drawValue(m.second);
			}
		}
	}
	
	// if needed draw additionnal infos 
	if (!collapsed)
	{	
		// Draw visible members
		for(auto& m : node->getMembers())
		{		
			if( m.second->getVisibility() == Visibility_Protected ||
				m.second->getVisibility() == Visibility_Private)
			{
				drawValue(m.second);
			}
		}

		// Draw component names
		ImGui::NewLine();
		ImGui::Text("Components :");
		for(auto& c : node->getComponents())
			ImGui::Text("- %s (%s)",c.first.c_str(),    c.second->getMember("__class__")->getValueAsString().c_str());

		// Draw parent's name
		ImGui::NewLine();
		ImGui::Text("Parameters :");
		std::string parentName = "NULL";
		if ( node->getParent() )
			parentName = node->getParent()->getMember("name")->getValueAsString();
		ImGui::Text("Parent: %s", parentName.c_str());
		
		// Draw dirty state 
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
			ImGui::EndGroup();

			hovered = ImGui::IsMouseHoveringRect(	ImVec2(screenPosition.x - size.x/2.0f, screenPosition.y - size.y/2.0f),
													ImVec2(screenPosition.x + size.x/2.0f, screenPosition.y + size.y/2.0f), true);

            if (hovered && ImGui::IsMouseReleased(1))
                ImGui::OpenPopup("NodeViewContextualMenu");

            if (ImGui::BeginPopup("NodeViewContextualMenu"))
            {
                if( ImGui::MenuItem("Arrange"))
                	this->arrangeRecursively();

                ImGui::MenuItem("Pinned",    "", &this->pinned,    true);
				ImGui::MenuItem("Collapsed", "", &this->collapsed, true);
                ImGui::Separator();
                if(ImGui::Selectable("Delete"))
                	node->deleteNextFrame();

                if(ImGui::Selectable("Save to JSON"))
                {
                	Application::SaveEntity(node);
                }

                ImGui::EndPopup();
            }

			// Selection by mouse

			if ( hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
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

void NodeView::ArrangeRecursively(NodeView* _view, ImVec2 _position)
{
	_view->setPosition(_position);

	// Arrange Input Nodes :
	auto wires = _view->getOwner()->getWires();
	for(auto eachWire : wires)
	{
		if (eachWire != nullptr && _view->getOwner()->hasMember(eachWire->getTarget()) )
		{

			if ( eachWire->getSource() != nullptr)
			{
				auto node         = dynamic_cast<Entity*>(eachWire->getSource()->getOwner());
				auto inputView    = node->getComponent("view")->getAs<NodeView*>();
				inputView->pinned = false;
				ArrangeRecursively(inputView, inputView->position);
			}
		}
	}
}


