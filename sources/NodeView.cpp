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

NodeView*   NodeView::s_selected              = nullptr;
NodeView*   NodeView::s_dragged               = nullptr;
DrawDetail_ NodeView::s_drawDetail            = DrawDetail_Default;
Member*     NodeView::s_draggedByMouseMember  = nullptr;
Member*     NodeView::s_hoveredByMouseMember  = nullptr;

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

bool NodeView::IsANodeDragged() {
	return GetDragged() != nullptr;
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
	ImVec2 topLeftCornerPosition(position - size / 2.0f);
	return topLeftCornerPosition;
}

ImVec2 NodeView::getMemberConnectorPosition(const std::string& _name, Connection_ _connection)const
{
	auto pos = getPosition();

	auto it = connectorOffsetPositionsY.find(_name);
	if (it != connectorOffsetPositionsY.end())
		pos.y += (*it).second;

	if (_connection == Connection_In)
		return ImVec2(pos.x, pos.y + size.y * 0.5f);
	else if (_connection == Connection_Out)
		return ImVec2(pos.x + size.x, pos.y + size.y * 0.5f);
	else {
		NODABLE_ASSERT(false); // _connection should be only In or Out.
		return ImVec2();
	}
}

void NodeView::setPosition(ImVec2 _position)
{
	this->position = _position;
}

void NodeView::translate(ImVec2 _delta)
{
	setPosition( position + _delta);
}

void NodeView::arrangeRecursively()
{
	ArrangeRecursively(this, position);
}

bool NodeView::update()
{
	auto deltaTime = ImGui::GetIO().DeltaTime;

	// Update opacity to reach 1.0f
	//-----------------------------

	if(opacity < 1.0f)
		opacity += (1.0f - opacity) * float(10) * deltaTime;

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

	
	updateInputConnectedNodes(node, deltaTime);
	return true;
}

void Nodable::NodeView::updateInputConnectedNodes(Nodable::Entity* node, float deltaTime)
{

	// automatically moves input connected nodes
	//------------------------------------------

	// first we get the spacing distance between nodes sepending on drawDetail global variable

	float spacingDistBase = 150.0f;
	float distances[3] = { spacingDistBase * 0.3f, spacingDistBase * 0.5f, spacingDistBase * 1.0f };
	float spacingDist = distances[s_drawDetail];

	// then we constraint each input view

	auto wires = node->getWires();
	auto inputIndex = 0;

	// Compute the cumulated height and the size x max of the input node view:
	auto cumulatedHeight = 0.0f;
	auto maxSizeX = 0.0f;
	for (auto eachWire : wires)
	{
		auto sourceNode = eachWire->getSource()->getOwner()->getAs<Entity*>();
		bool isWireAnInput = node->hasMember(eachWire->getTarget());
		auto inputView = reinterpret_cast<NodeView*>(sourceNode->getComponent("view"));
		if (isWireAnInput && !inputView->pinned)
		{
			cumulatedHeight += inputView->size.y;
			maxSizeX = std::max(maxSizeX, inputView->size.x);
		}
	}

	/*
	Update Views that are linked to this input views.
	This code maintain them stacked together with a little attenuated movement.
	*/

	auto posY = getMemberConnectorPosition("", Connection_In).y - cumulatedHeight / 2.0f;
	float nodeVerticalSpacing(10);

	for (auto eachWire : wires)
	{
		bool isWireAnInput = node->hasMember(eachWire->getTarget());
		if (isWireAnInput)
		{
			auto sourceNode = eachWire->getSource()->getOwner()->getAs<Entity*>();
			auto inputView = reinterpret_cast<NodeView*>(sourceNode->getComponent("view"));

			if (!inputView->pinned)
			{
				// Compute new position for this input view
				ImVec2 newPos(getMemberConnectorPosition("", Connection_In).x - maxSizeX - spacingDist, posY);
				posY += inputView->size.y + nodeVerticalSpacing;

				// Compute a delta to apply to move to this new position
				auto currentPos = inputView->getPosition();
				auto factor = std::min(1.0f, 10.f * deltaTime); // TODO: use frame time
				ImVec2 delta((newPos.x - currentPos.x) * factor, (newPos.y - currentPos.y) * factor);

				bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
				if (!isDeltaTooSmall)
					inputView->translate(delta);
			}

			inputIndex++;
		}
	}

}

bool NodeView::draw()
{
	bool edited = false;
	auto node   = getOwner();

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
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);

	
	const auto halfSize = size / 2.0;

	if ( position.x != -1.0f || position.y != -1.0f)
		ImGui::SetCursorPos( position - halfSize );
	else
		ImGui::SetCursorPos(ImVec2());

	ImGui::PushID(this);
	ImGui::BeginGroup();

	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = View::ConvertCursorPositionToScreenPosition( position );


	// Draw the background of the Group
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	{			
		auto borderCol = IsSelected(this) ? borderColorSelected : getColor(ColorType_Border);

		
		auto itemRectMin = screenPosition - halfSize;
		auto itemRectMax = screenPosition + halfSize;

		// Draw the rectangle under everything
		View::DrawRectShadow	(itemRectMin,					itemRectMax, borderRadius, 4, ImVec2(1.0f), getColor(ColorType_Shadow));
		draw_list->AddRectFilled(itemRectMin,					itemRectMax, getColor(ColorType_Fill), borderRadius);
		draw_list->AddRect		(itemRectMin + ImVec2(1.0f)	,	itemRectMax, getColor(ColorType_BorderHighlights), borderRadius);
		draw_list->AddRect		(itemRectMin,					itemRectMax, borderCol, borderRadius);				

		// darken the background under the content
		draw_list->AddRectFilled(itemRectMin + ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + nodePadding), itemRectMax, ImColor(0.0f,0.0f,0.0f, 0.1f), borderRadius, 4);

		// Draw an additionnal blinking rectangle when selected
		if (IsSelected(this))
		{
			float alpha  = sin(ImGui::GetTime() * 10.0f) * 0.25f + 0.5f;
			float offset = 4.0f;
			draw_list->AddRect(itemRectMin - ImVec2(offset), itemRectMax + ImVec2(offset), ImColor(1.0f, 1.0f, 1.0f, alpha), borderRadius + offset, ~0, offset / 2.0f);
		}
	}

	// Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorPos(cursorPositionBeforeContent);
	ImGui::InvisibleButton("##", size);
	ImGui::SetItemAllowOverlap();
	hovered = ImGui::IsItemHovered();
	ImGui::SetCursorPos(cursorPositionBeforeContent + nodePadding );

	ImGui::PushItemWidth(size.x - float(2) * nodePadding);

	// Draw the window content 
	//------------------------

	ShadowedText(ImVec2(1.0f), getColor(ColorType_BorderHighlights), node->getLabel()); // text with a lighter shadow (incrust effect)

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + nodePadding);
	ImGui::Indent(nodePadding);

	connectorOffsetPositionsY.clear();


	// Draw visible members
	{
		// Draw input only first
		for(auto& m : node->getMembers())
		{		
			auto member = m.second;
			if (member->getVisibility() == Visibility_AlwaysVisible && member->getConnection() == Connection_In)
			{
				drawMember(m.second);
			}
		}

		// Then draw the rest
		for (auto& m : node->getMembers())
		{
			auto member = m.second;
			if (member->getVisibility() == Visibility_AlwaysVisible && member->getConnection() != Connection_In)
			{
				drawMember(member);
			}
		}
	}

	// if needed draw additionnal infos 
	if (!collapsed)
	{	
		// Draw visible members
		for(auto& m : node->getMembers())
		{		
			auto member = m.second;

			if( member->getVisibility() == Visibility_VisibleOnlyWhenUncollapsed ||
				member->getVisibility() == Visibility_AlwaysHidden)
			{
				this->drawMember(member);
			}
		}	

		// Draw component names
		ImGui::NewLine();
		ImGui::Text("Components :");

		for (auto& pair : node->getComponents()) {

			auto component	= pair.second;
			auto name		= pair.first;
			auto className	= component->getMember("__class__")->getValueAsString();

			ImGui::Text("- %s (%s)", name.c_str(), className.c_str());
		}

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
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + nodePadding);

	auto cursorPosAfterContent = ImGui::GetCursorPos();

	// Ends the Window
	//----------------

	ImGui::EndGroup();

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
		if(GetDragged() == nullptr && ImGui::IsMouseClicked(0) && hovered && (s_draggedByMouseMember == nullptr))
			SetDragged(this);
	}else{				
		if ( ImGui::IsMouseReleased(0))
			SetDragged(nullptr);				
	}		

	// Collapse/uncollapse by double click (double/divide x size by 2)
	if( hovered && ImGui::IsMouseDoubleClicked(0))
	{
		if (collapsed) {
			collapsed = false;
			size.x   *= float(2);
		}
		else {
			collapsed  = true;
			size.x    /= float(2);
		}			
	}	

	// interpolate size.y to fit with its content
	size.y = 0.5f * size.y  + 0.5f * (cursorPosAfterContent.y - cursorPositionBeforeContent.y);


	ImGui::PopStyleVar();
	ImGui::PopID();

	return edited;
}

void NodeView::ArrangeRecursively(NodeView* _view, ImVec2 _position)
{
	_view->setPosition(_position);

	// Force and update of input connected nodes with a delta time extra high
	// to ensure all nodes were well placed in a single call (no smooth moves)
	_view->updateInputConnectedNodes(_view->getOwner(), float(1000) );

	// Get wires that go outside from this node :
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

bool NodeView::drawMember(Member* _member) {

	bool edited = false;
	auto node = getOwner();

	auto memberTopPositionOffsetY = ImGui::GetCursorPos().y - position.y;

	if (_member->isSet())
	{
		/* Draw the member */
		switch (_member->getType())
		{
		case Type_Number:
			{
				std::string label("##");
				label.append(_member->getName());
				float f(_member->getValueAsNumber());
				if (ImGui::InputFloat(label.c_str(), &f))
				{
					_member->setValue(f);
					node->setDirty(true);
					edited |= true;
				}
				break;
			}
		case Type_String:
			{
				std::string label("##");
				label.append(_member->getName());
				char str[255];
				sprintf(str, "%s", _member->getValueAsString().c_str());

				if (ImGui::InputText(label.c_str(), str, 255) )
				{
					_member->setValue(str);
					node->setDirty(true);
					edited |= true;
				}
				break;
			}
		default:
			{
				ImGui::Text("%s", _member->getName().c_str());
				ImGui::SameLine(100.0f);
				ImGui::Text("%s", _member->getValueAsString().c_str());
				break;
			}
		}

	}
	else {
		ImGui::Text("%s", _member->getName().c_str());
	}

	/* If value is hovered, we draw a tooltip that print the source expression of the value*/
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Source expression: \"%s\"", _member->getSourceExpression().c_str());
		ImGui::EndTooltip();
	}

	auto memberBottomPositionOffsetY = ImGui::GetCursorPos().y - position.y;
	connectorOffsetPositionsY[_member->getName()] = (memberTopPositionOffsetY + memberBottomPositionOffsetY) / 2.0f; // store y axis middle

	/*
		Draw the wire connectors (In or Out only)
	*/

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto memberName       = _member->getName();

	if (_member->allows(Connection_In)) {
		ImVec2      connectorPos = getMemberConnectorPosition( memberName, Connection_In);
		drawMemberConnector(connectorPos, _member, draw_list);
	}
		
	if (_member->allows(Connection_Out)) {
		ImVec2      connectorPos = getMemberConnectorPosition( memberName, Connection_Out);
		drawMemberConnector(connectorPos, _member, draw_list);
	}

	return edited;
}

void Nodable::NodeView::drawMemberConnector(ImVec2& connectorPos, Nodable::Member* _member, ImDrawList* draw_list)
{
	// Unvisible Button on top of the Circle

	ImVec2 cpos = ImGui::GetCursorPos();
	float invisibleButtonOffsetFactor(1.2);
	ImGui::SetCursorScreenPos(connectorPos - ImVec2(connectorRadius * invisibleButtonOffsetFactor) + ImGui::GetWindowPos());
	ImGui::PushID(_member);
	bool clicked = ImGui::InvisibleButton("###", ImVec2(connectorRadius * float(2) * invisibleButtonOffsetFactor, connectorRadius * float(2) * invisibleButtonOffsetFactor));
	ImGui::PopID();
	ImGui::SetCursorPos(cpos);

	// Circle
	auto isItemHovered = ImGui::IsItemHoveredRect();
	ImVec2 cursorPos = ImGui::GetCursorPos();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

	ImVec2 connnectorScreenPos = connectorPos + cursorScreenPos - cursorPos;

	if (isItemHovered)
		draw_list->AddCircleFilled(connnectorScreenPos, connectorRadius, getColor(ColorType_Highlighted));
	else
		draw_list->AddCircleFilled(connnectorScreenPos, connectorRadius, getColor(ColorType_Fill));

	draw_list->AddCircle(connnectorScreenPos, connectorRadius, getColor(ColorType_Border));


	// Manage mouse events in order to link two members by a Wire :

	// HOVERED
	if (isItemHovered)
		s_hoveredByMouseMember = _member;
	else if (s_hoveredByMouseMember == _member)
		s_hoveredByMouseMember = nullptr;

	// DRAG
	if (isItemHovered && ImGui::IsMouseDown(0) && s_draggedByMouseMember == nullptr)
		s_draggedByMouseMember = _member;
}
;


