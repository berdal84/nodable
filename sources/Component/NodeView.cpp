#include "NodeView.h"
#include "Log.h"		          // for LOG_DEBUG(...)
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include "Application.h"
#include "ComputeBase.h"
#include "NodeTraversal.h"
#include <Language/Common/Serializer.h>

#define NODE_VIEW_DEFAULT_SIZE ImVec2(10.0f, 35.0f)

using namespace Nodable;

NodeView*   NodeView::s_selected    = nullptr;
NodeView*   NodeView::s_draggedNode = nullptr;
DrawDetail_ NodeView::s_drawDetail  = Nodable::DrawDetail_Default;
const Connector*  NodeView::s_draggedConnector      = nullptr;
const Connector*  NodeView::s_hoveredConnector      = nullptr;
const float NodeView::s_memberInputSizeMin = 10.0f;
const ImVec2 NodeView::s_toggleBtnSize(10.0, 25.0f);

NodeView::NodeView():
        position(500.0f, -1.0f),
        size(NODE_VIEW_DEFAULT_SIZE),
        opacity(1.0f),
        collapsed(true),
        pinned(false),
        borderRadius(5.0f),
        borderColorSelected(1.0f, 1.0f, 1.0f)
{

}

NodeView::~NodeView()
{
    if ( s_selected == this )
    {
        s_selected = nullptr;
    }
}

void NodeView::exposeMember(Member* _member)
{
    this->exposedMemberViews.emplace_back(_member);
}

void NodeView::setOwner(Node* _node)
{
    //  We expose first the members which allows input connections
    for(auto& m : _node->getMembers())
    {
        auto member = m.second;
        if (member->getVisibility() == Visibility::Always && member->allowsConnection(Way_In) )
        {
            this->exposeMember(member);
        }
    }

    // Then we expose node which allows output connection (if they are not yet exposed)
    for (auto& m : _node->getMembers())
    {
        auto member = m.second;
        if (member->getVisibility() == Visibility::Always && member->allowsConnection(Way_Out) && !this->isMemberExposed(member) )
        {
            this->exposeMember(member);
        }
    }

    Component::setOwner(_node);
}

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

ImVec2 NodeView::getConnectorPosition(const Member *_member, Way _way)const
{

	auto it = std::find_if(
	        exposedMemberViews.begin(),
	        exposedMemberViews.end(),
	        [&_member](const MemberView& _view)->bool
	        {
	            return _member == _view.member;
	        });

    auto pos = position;

	if (it != exposedMemberViews.end())
        pos = (*it).screenPos;

	auto nodeViewScreenPosition = View::CursorPosToScreenPos(position);

	// Input => Top
	if (_way == Way_In)
    {
		return ImVec2(pos.x , nodeViewScreenPosition.y - size.y * 0.5f);
    }
	// Outputs => Bottom
	else if (_way == Way_Out)
	{
        return ImVec2(pos.x, nodeViewScreenPosition.y + size.y * 0.5f);
    }

    NODABLE_ASSERT(false); // _way should be only In or Out.
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

	float spacingDistBase = 25.0f;
	float distances[3] = { spacingDistBase * 1.0f, spacingDistBase * 1.5f, spacingDistBase * 1.5f };
	float spacingDist = distances[s_drawDetail];

	// then we constraint each input view

	auto wires = node->getWires();
	auto inputIndex = 0;

	// Compute the cumulated height and the size x max of the input node view:
	auto cumulatedSize = 0.0f;
	auto sizeMax = 0.0f;
	for (auto eachWire : wires)
	{
		auto sourceNode    = eachWire->getSource()->getOwner()->as<Node>(); // TODO: add some checks
		bool isWireAnInput = node->has(eachWire->getTarget());
		auto inputView     = sourceNode->getComponent<NodeView>();

		if (isWireAnInput && !inputView->pinned)
		{
            cumulatedSize += inputView->size.x;
            sizeMax = std::max(sizeMax, inputView->size.x);
		}
	}

	/*
	Update Views that are linked to this input views.
	This code maintain them stacked together with a little attenuated movement.
	*/

	auto posX = position.x - cumulatedSize / 2.0f;

	float nodeSpacing(10);

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
				ImVec2 newPos(posX + inputView->size.x / 2.0f, position.y - spacingDist - inputView->size.y / 2.0f - size.y / 2.0f );
                posX += inputView->size.x + nodeSpacing;

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
	ImGui::SetCursorPos( getRoundedPosition() - halfSize );
	ImGui::PushID(this);
	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = View::CursorPosToScreenPos( getRoundedPosition() );

	// Draw the background of the Group
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	{			
		auto borderCol = IsSelected(this) ? borderColorSelected : getColor(ColorType_Border);

		auto itemRectMin = screenPosition - halfSize;
		auto itemRectMax = screenPosition + halfSize;

		// Draw the rectangle under everything
		View::DrawRectShadow(itemRectMin, itemRectMax, borderRadius, 4, ImVec2(1.0f), getColor(ColorType_Shadow));
		draw_list->AddRectFilled(itemRectMin, itemRectMax, getColor(ColorType_Fill), borderRadius);
		draw_list->AddRect(itemRectMin + ImVec2(1.0f),	itemRectMax, getColor(ColorType_BorderHighlights), borderRadius);
		draw_list->AddRect(itemRectMin, itemRectMax, borderCol, borderRadius);

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

	// Draw the window content
	//------------------------
    ImGui::BeginGroup();
	ShadowedText(ImVec2(1.0f), getColor(ColorType_BorderHighlights), node->getLabel()); // text with a lighter shadow (incrust effect)

	// Draw visible members
    for( auto& memberView : this->exposedMemberViews )
    {
        ImGui::SameLine();
        drawMemberView(memberView);
    }

	ImGui::SameLine();

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
                DrawMemberInput(member);
			}
		}	

		// Draw component names
		ImGui::NewLine();
		ImGui::Text("Components :");

		for (auto& pair : node->getComponents())
		{
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
        {
			parentName = node->getParentContainer()->getLabel();
		}

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

	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + nodePadding );
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + nodePadding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    size.x = std::ceil( ImGui::GetItemRectSize().x );
    size.y = std::max(NODE_VIEW_DEFAULT_SIZE.y, std::ceil( ImGui::GetItemRectSize().y ));

	// Draw connectors for always visible members
    for(auto& m : node->getMembers())
    {
        auto member = m.second;
        if (member->getVisibility() == Visibility::Always)
        {
            drawMemberConnectors(member);
        }
    }

    // Contextual menu (right click)
    if (hovered && ImGui::IsMouseReleased(1))
    {
        ImGui::OpenPopup("NodeViewContextualMenu");
    }

    if (ImGui::BeginPopup("NodeViewContextualMenu"))
    {
        if( ImGui::MenuItem("Arrange"))
        {
            this->arrangeRecursively();
        }

        ImGui::MenuItem("Pinned",    "", &this->pinned,    true);
		ImGui::MenuItem("Collapsed", "", &this->collapsed, true);
        ImGui::Separator();

        if(ImGui::Selectable("Delete"))
        {
            node->flagForDeletion();
        }

        if(ImGui::Selectable("Save to JSON"))
        {
            Application::SaveNode(node);
        }            
        ImGui::EndPopup();
    }

	// Selection by mouse (left or right click)
	if ( hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
	{
        SetSelected(this);
    }

	// Mouse dragging
	if ( GetDragged() != this)
	{
		if( GetDragged() == nullptr && ImGui::IsMouseDown(0) && hovered)
        {
			StartDragNode(this);
        }
	}
	else if ( ImGui::IsMouseReleased(0))
	{
		StartDragNode(nullptr);				
	}		

	// Collapse on/off
	if( hovered && ImGui::IsMouseDoubleClicked(0))
	{
		this->collapsed = !this->collapsed;
	}

    // interpolate size.y to fit with its content
	//size.y = std::max( 35.0f, cursorPosAfterContent.y - cursorPositionBeforeContent.y);

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

void NodeView::drawMemberConnectors(Member* _member)
{
    /*
    Draw the wire connectors (In or Out only)
   */

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (_member->allowsConnection(Way_In))
    {
        ImVec2      connectorPos = getConnectorPosition( _member, Way_In);
        drawConnector(connectorPos, _member->input(), draw_list);
    }

    if (_member->allowsConnection(Way_Out))
    {
        ImVec2      connectorPos = getConnectorPosition( _member, Way_Out);
        drawConnector(connectorPos, _member->output(), draw_list);
    }
}

bool NodeView::drawMemberView(MemberView &_memberView )
{
    bool edited = false;
    auto member = _memberView.member;

    if( !_memberView.touched )
    {
        const bool isAnInputUnconnected = member->getInputMember() != nullptr || !member->allowsConnection(Way_In);
        const bool isVariable = member->getOwner()->getClass() == Variable::GetClass();
        _memberView.showInput = !isAnInputUnconnected || isVariable;
    }

    _memberView.screenPos = ImGui::GetCursorScreenPos();

    // input
    if ( _memberView.showInput )
    {
        // try to draw an as small as possible input field
        float inputWidth = 5.0f + std::max( ImGui::CalcTextSize(((std::string)*member).c_str()).x, NodeView::s_memberInputSizeMin );
        _memberView.screenPos.x += inputWidth / 2.0f;
        ImGui::PushItemWidth(inputWidth);
        edited = NodeView::DrawMemberInput(member);
        ImGui::PopItemWidth();
    }
    else
    {
        ImGui::Button("", NodeView::s_toggleBtnSize);
        _memberView.screenPos.x += NodeView::s_toggleBtnSize.x / 2.0f;

        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s (%s)",
                        member->getName().c_str(),
                        member->getTypeAsString().c_str());
            ImGui::EndTooltip();
        }

        if ( ImGui::IsItemClicked(0) )
        {
            _memberView.showInput = !_memberView.showInput;
            _memberView.touched = true;
        }
    }

    return edited;
}

bool NodeView::DrawMemberInput( Member *_member, const char* _label )
{
    bool edited = false;

    Node* node  = _member->getOwner()->as<Node>();

    // Create a label (everything after ## will not be displayed)
    std::string label;
    if ( _label != nullptr )
    {
        label.append(_label);
    }
    else
    {
        label.append("##" + _member->getName());
    }

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
            ImGui::Text( "%s", ((std::string)*_member).c_str());
            break;
        }
    }

    /* If value is hovered, we draw a tooltip that print the source expression of the value*/
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        auto language = node->getParentContainer()->getLanguage();
        ImGui::Text("%s", language->getSerializer()->serialize(_member).c_str() );
        ImGui::EndTooltip();
    }

    return edited;
}

void NodeView::drawConnector(ImVec2& connnectorScreenPos, const Connector* _connector, ImDrawList* draw_list)
{
	// Unvisible Button on top of the Circle

    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

	auto invisibleButtonOffsetFactor = 1.2f;
	ImGui::SetCursorScreenPos(connnectorScreenPos - ImVec2(connectorRadius * invisibleButtonOffsetFactor));
	ImGui::PushID(_connector->member);
	bool clicked = ImGui::InvisibleButton("###", ImVec2(connectorRadius * 2.0f * invisibleButtonOffsetFactor, connectorRadius * 2.0f * invisibleButtonOffsetFactor));
	ImGui::PopID();
	ImGui::SetCursorScreenPos(cursorScreenPos);

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

	if (isItemHovered)
    {
		s_hoveredConnector = _connector;
        ImGui::BeginTooltip();
        ImGui::Text("%s", _connector->member->getName().c_str() );
        ImGui::EndTooltip();
    }
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

void Nodable::NodeView::DrawNodeViewAsPropertiesPanel(NodeView* _view)
{
    const float labelColumnWidth = ImGui::GetContentRegionAvailWidth() / 2.0f;

    // Then draw the rest
    for (auto& eachView : _view->exposedMemberViews )
    {
        // label (<name> (<way> <type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s, %s): ",
                eachView.member->getName().c_str(),
                WayToString(eachView.member->getConnectorWay()).c_str(),
                eachView.member->getTypeAsString().c_str());

        // input
        ImGui::SameLine(labelColumnWidth);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
        NodeView::DrawMemberInput(eachView.member);
    }
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

bool NodeView::isMemberExposed(Member *_member)
{
    return std::find_if(
            exposedMemberViews.begin(),
            exposedMemberViews.end(),
            [&_member](const MemberView& _eachMemberView)->bool
            {
                return _eachMemberView.member == _member;
            }) != exposedMemberViews.end();
}
