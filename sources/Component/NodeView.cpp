#include "NodeView.h"
#include "Log.h"		          // for LOG_DEBUG(...)
#include <imgui/imgui.h>
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include "Application.h"
#include "ComputeBase.h"
#include "NodeTraversal.h"

using namespace Nodable;

NodeView*   NodeView::s_selected              = nullptr;
NodeView*   NodeView::s_draggedNode               = nullptr;
DrawDetail_ NodeView::s_drawDetail            = Nodable::DrawDetail_Default;
const Connector*  NodeView::s_draggedConnector      = nullptr;
const Connector*  NodeView::s_hoveredConnector      = nullptr;

void NodeView::SetSelected(NodeView* _view)
{
	s_selected = _view;
}

NodeView* NodeView::GetSelected()
{
	return s_selected;
}

void NodeView::StartDragNode(NodeView* _view)
{
	if( s_draggedConnector == nullptr) // Prevent dragging node while dragging connector
		s_draggedNode = _view;
}

bool NodeView::IsANodeDragged() {
	return GetDragged() != nullptr;
}

NodeView* NodeView::GetDragged()
{
	return s_draggedNode;
}

bool NodeView::IsSelected(NodeView* _view)
{
	return s_selected == _view;
}

ImVec2 NodeView::getRoundedPosition()const
{
	ImVec2 roundedPosition;

	roundedPosition.x = std::round(position.x);
	roundedPosition.y = std::round(position.y);

	return roundedPosition;
}

ImVec2 NodeView::getConnectorPosition(const std::string& _name, Way _way)const
{
	auto pos = position;

	auto it = connectorOffsetPositionsY.find(_name);
	if (it != connectorOffsetPositionsY.end())
		pos.y += (*it).second;

	// Inputs are displayed on the left
	if (_way == Way_In)
		return ImVec2(pos.x - size.x * 0.5f, pos.y);

	// Outputs are displayed on the right
	else if (_way == Way_Out)
		return ImVec2(pos.x + size.x * 0.5f, pos.y);
	else {
		NODABLE_ASSERT(false); // _way should be only In or Out.
		return ImVec2();
	}
}

void NodeView::setPosition(ImVec2 _position)
{
	this->position.x =  _position.x;
	this->position.y =  _position.y;
}

void NodeView::translate(ImVec2 _delta)
{
	setPosition( position + _delta);
}

void NodeView::arrangeRecursively()
{
	ArrangeRecursively(this);
}

bool NodeView::update()
{
	auto deltaTime = ImGui::GetIO().DeltaTime;

	return update(deltaTime);
}

bool NodeView::update(float _deltaTime) {
	// Update opacity to reach 1.0f
	//-----------------------------

	if (opacity < 1.0f)
		opacity += (1.0f - opacity) * float(10) * _deltaTime;

	// Set background color according to node class 
	//---------------------------------------------
	auto node = getOwner();
	NODABLE_ASSERT(node != nullptr);

	if (node->hasComponent<ComputeBase>())
		setColor(ColorType_Fill, ImColor(0.7f, 0.7f, 0.9f));
	else if (dynamic_cast<Variable*>(node) != nullptr)
		setColor(ColorType_Fill, ImColor(0.7f, 0.9f, 0.7f));
	else
		setColor(ColorType_Fill, ImColor(0.9f, 0.9f, 0.7f));


	updateInputConnectedNodes(node, _deltaTime);

	return true;
}

void NodeView::updateInputConnectedNodes(Nodable::Node* node, float deltaTime)
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
		auto sourceNode    = eachWire->getSource()->getOwner()->as<Node>(); // TODO: add some checks
		bool isWireAnInput = node->has(eachWire->getTarget());
		auto inputView     = sourceNode->getComponent<NodeView>();

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

	auto posY = position.y - cumulatedHeight / 2.0f;

	float nodeVerticalSpacing(10);

	for (auto eachWire : wires)
	{
		bool isWireAnInput = node->has(eachWire->getTarget());
		if (isWireAnInput)
		{
			auto sourceNode = eachWire->getSource()->getOwner()->as<Node>();
			auto inputView  = sourceNode->getComponent<NodeView>();

			if (!inputView->pinned)
			{
				// Compute new position for this input view
				ImVec2 newPos( position.x - size.x / 2.0f - maxSizeX - spacingDist + inputView->size.x / 2.0f, posY + inputView->size.y / 2.0f);
				posY += inputView->size.y + nodeVerticalSpacing;

				// Compute a delta to apply to move to this new position
				auto currentPos = inputView->position;				
				ImVec2 delta((newPos.x - currentPos.x), (newPos.y - currentPos.y));

				bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
				if (!isDeltaTooSmall) {
					auto factor = std::min(1.0f, 10.f * deltaTime);
					inputView->translate(delta * factor);
				}
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
		ImGui::SetCursorPos( getRoundedPosition() - halfSize );
	else
		ImGui::SetCursorPos(ImVec2());

	ImGui::PushID(this);
	ImGui::BeginGroup();

	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = View::CursorPosToScreenPos( getRoundedPosition() );


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
			auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
			float offset = 4.0f;
			draw_list->AddRect(itemRectMin - ImVec2(offset), itemRectMax + ImVec2(offset), ImColor(1.0f, 1.0f, 1.0f, float(alpha) ), borderRadius + offset, ~0, offset / 2.0f);
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
			if (member->getVisibility() == Visibility::Always && member->getConnectorWay() == Way_In)
			{
				drawMember(m.second);
			}
		}

		// Then draw the rest
		for (auto& m : node->getMembers())
		{
			auto member = m.second;
			if (member->getVisibility() == Visibility::Always && member->getConnectorWay() != Way_In)
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

			if( member->getVisibility() == Visibility::OnlyWhenUncollapsed ||
				member->getVisibility() == Visibility::Hidden)
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
			auto className	= component->getClass()->getName();

			ImGui::Text("- %s (%s)", name.c_str(), className);
		}

		// Draw parentContainer's name
		ImGui::NewLine();
		ImGui::Text("Parameters :");
		std::string parentName = "NULL";
		if ( node->getParentContainer() )
			parentName = (std::string)*node->getParentContainer()->get("name");
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
            Application::SaveNode(node);
        }            
        ImGui::EndPopup();
    }

	// Selection by mouse

	if ( hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
		SetSelected(this);

	// Dragging by mouse

	if ( GetDragged() != this) {
		if( GetDragged() == nullptr && ImGui::IsMouseDown(0) && hovered)
			StartDragNode(this);

	} else if ( ImGui::IsMouseReleased(0)) {
		StartDragNode(nullptr);				
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
	size.y = (cursorPosAfterContent.y - cursorPositionBeforeContent.y);


	ImGui::PopStyleVar();
	ImGui::PopID();

	return edited;
}

void NodeView::ArrangeRecursively(NodeView* _view)
{

	// Force and update of input connected nodes with a delta time extra high
	// to ensure all nodes were well placed in a single call (no smooth moves)
	_view->update( float(1000) );

	// Get wires that go outside from this node :
	auto wires = _view->getOwner()->getWires();

	for(auto eachWire : wires)
	{
		if (eachWire != nullptr && _view->getOwner()->has(eachWire->getTarget()) )
		{

			if ( eachWire->getSource() != nullptr)
			{
				auto node         = dynamic_cast<Node*>(eachWire->getSource()->getOwner());
				auto inputView    = node->getComponent<NodeView>();
				inputView->pinned = false;
				ArrangeRecursively(inputView);
			}
		}
	}
}

bool NodeView::drawMember(Member* _member) {

	bool edited = false;
	auto node   = getOwner();
	auto memberTopPositionOffsetY = ImGui::GetCursorPos().y - getRoundedPosition().y;

	std::string label("##");
	label.append(_member->getName());

    auto inputFlags = ImGuiInputTextFlags_None;

	/* Draw the member */
	switch (_member->getType())
	{
	case Type::Double:
		{
			auto f = (double)*_member;

			if (ImGui::InputDouble(label.c_str(), &f, 0.0F, 0.0F, "%g", inputFlags ) && !_member->hasInputConnected())
			{
				_member->set(f);
				NodeTraversal::SetDirty(node);
				edited |= true;
			}
			break;
		}
	case Type::String:
		{				
			char str[255];
			snprintf(str, 255, "%s", ((std::string)*_member).c_str() );

			if ( ImGui::InputText(label.c_str(), str, 255, inputFlags) && !_member->hasInputConnected() )
			{
				_member->set(str);
				NodeTraversal::SetDirty(node);
				edited |= true;
			}
			break;
		}
	case Type::Boolean:
	{			
		std::string checkBoxLabel = _member->getName();

		auto b = (bool)*_member;

		if (ImGui::Checkbox( checkBoxLabel.c_str(), &b ) && !_member->hasInputConnected() ) {
			_member->set(b);
			NodeTraversal::SetDirty(node);
			edited |= true;
		}
		break;
	}
	default:
		{
			ImGui::Text( "%s", _member->getName().c_str());
			break;
		}
	}


	/* If value is hovered, we draw a tooltip that print the source expression of the value*/
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("%s", getOwner()->getParentContainer()->getLanguage()->serialize(_member).c_str() );
		ImGui::EndTooltip();
	}

	auto memberBottomPositionOffsetY = ImGui::GetCursorPos().y - getRoundedPosition().y;
	connectorOffsetPositionsY[_member->getName()] = (memberTopPositionOffsetY + memberBottomPositionOffsetY) / 2.0f; // store y axis middle

	/*
		Draw the wire connectors (In or Out only)
	*/

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto memberName       = _member->getName();

	if (_member->allowsConnection(Way_In)) {
		ImVec2      connectorPos = getConnectorPosition( memberName, Way_In);
		drawConnector(connectorPos, _member->input(), draw_list);
	}
		
	if (_member->allowsConnection(Way_Out)) {
		ImVec2      connectorPos = getConnectorPosition( memberName, Way_Out);
		drawConnector(connectorPos, _member->output(), draw_list);
	}

	return edited;
}

void NodeView::drawConnector(ImVec2& connectorPos, const Connector* _connector, ImDrawList* draw_list)
{
	// Unvisible Button on top of the Circle

    ImVec2 cursorPos = ImGui::GetCursorPos();
    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    ImVec2 connnectorScreenPos = connectorPos + cursorScreenPos - cursorPos;

	auto invisibleButtonOffsetFactor = 1.2f;
	ImGui::SetCursorScreenPos(connnectorScreenPos - ImVec2(connectorRadius * invisibleButtonOffsetFactor));
	ImGui::PushID(_connector->member);
	bool clicked = ImGui::InvisibleButton("###", ImVec2(connectorRadius * 2.0f * invisibleButtonOffsetFactor, connectorRadius * 2.0f * invisibleButtonOffsetFactor));
	ImGui::PopID();
	ImGui::SetCursorPos(cursorPos);

	// Circle
	auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

	if (isItemHovered)
		draw_list->AddCircleFilled(connnectorScreenPos, connectorRadius, getColor(ColorType_Highlighted));
	else
		draw_list->AddCircleFilled(connnectorScreenPos, connectorRadius, getColor(ColorType_Fill));

	draw_list->AddCircle(connnectorScreenPos, connectorRadius, getColor(ColorType_Border));


	// Manage mouse events in order to link two members by a Wire :


	// DRAG
	if (isItemHovered && ImGui::IsMouseDown(0) && s_draggedConnector == nullptr) {
		StartDragConnector(_connector);
	}

	// HOVERED
	if (isItemHovered)
		s_hoveredConnector = _connector;

	else if (s_hoveredConnector != nullptr && s_hoveredConnector->equals(_connector))
	{
		s_hoveredConnector = nullptr;
	}
	
}

ImRect Nodable::NodeView::getRect() const {
	return ImRect(getRoundedPosition() - size * 0.5f, getRoundedPosition() + size * 0.5f);
}


bool Nodable::NodeView::IsInsideRect(NodeView* _nodeView, ImRect _rect) {
	auto nodeRect = _nodeView->getRect();
	return _rect.Contains(nodeRect);
}

void Nodable::NodeView::ConstraintToRect(NodeView* _view, ImRect _rect)
{
	
	if ( !NodeView::IsInsideRect(_view, _rect)) {

		_rect.Expand(ImVec2(-2, -2)); // shrink

		auto nodeRect = _view->getRect();

		auto newPos = _view->getRoundedPosition();

		auto left  = _rect.Min.x - nodeRect.Min.x;
		auto right = _rect.Max.x - nodeRect.Max.x;
		auto up    = _rect.Min.y - nodeRect.Min.y;
		auto down  = _rect.Max.y - nodeRect.Max.y;

		     if ( left > 0 )  nodeRect.TranslateX(left);
		else if ( right < 0 ) nodeRect.TranslateX(right);
			 
			 if ( up > 0 )    nodeRect.TranslateY(up);
		else if ( down < 0 )  nodeRect.TranslateY(down);

		_view->setPosition(nodeRect.GetCenter());
	}

}


