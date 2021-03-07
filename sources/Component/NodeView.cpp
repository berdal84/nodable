#include "NodeView.h"
#include "Log.h"		          // for LOG_DEBUG(...)
#include "GraphNode.h"
#include "VariableNode.h"
#include "Wire.h"
#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include "Application.h"
#include "ComputeBase.h"
#include "NodeTraversal.h"
#include <Language/Common/Serializer.h>
#include <Node/AbstractCodeBlockNode.h>
#include "CodeBlockNode.h"
#include <Core/Maths.h>
#include <Node/CodeBlockNode.h>
#include "Node/InstructionNode.h"

#define NODE_VIEW_DEFAULT_SIZE ImVec2(10.0f, 35.0f)

using namespace Nodable;

NodeView*          NodeView::s_selected               = nullptr;
NodeView*          NodeView::s_draggedNode            = nullptr;
NodeViewDetail     NodeView::s_viewDetail             = NodeViewDetail::Default;
const Connector*   NodeView::s_draggedConnector       = nullptr;
const Connector*   NodeView::s_hoveredConnector       = nullptr;
const float        NodeView::s_memberInputSizeMin     = 10.0f;
const ImVec2       NodeView::s_memberInputToggleButtonSize   = ImVec2(10.0, 25.0f);
const float        NodeView::s_nodeSpacingDistance           = 25.0f;
std::vector<NodeView*> NodeView::s_instances;

NodeView::NodeView():
        position(500.0f, -1.0f),
        size(NODE_VIEW_DEFAULT_SIZE),
        opacity(1.0f),
        forceMemberInputVisible(false),
        pinned(false),
        borderRadius(5.0f),
        borderColorSelected(1.0f, 1.0f, 1.0f)
{
    NodeView::s_instances.push_back(this);
}

NodeView::~NodeView()
{
    // delete MemberViews
    for ( auto& pair: exposedMembers )
    {
        delete pair.second;
    }

    // deselect
    if ( s_selected == this )
    {
        s_selected = nullptr;
    }

    // Erase instance in static vector
    auto found = std::find( s_instances.begin(), s_instances.end(), this);
    assert(found != s_instances.end() );
    s_instances.erase(found);
}

std::string NodeView::getLabel()
{
    Node* node = getOwner();

    if (s_viewDetail == NodeViewDetail::Minimalist )
    {
        // I always add an ICON_FA at the begining of any node label string (encoded in 4 bytes)
        return std::string(node->getLabel()).substr(0,4);
    }
    return node->getLabel();
}

void NodeView::exposeMember(Member* _member, Way _way)
{
    assert(_way == Way_In || _way == Way_Out);

    MemberView* memberView = new MemberView(_member);

    if( _way == Way_In )
    {
        this->exposedInputsMembers.push_back(memberView);
    }
    else // Way_Out
    {
        this->exposedOutputMembers.push_back(memberView);
    }

    this->exposedMembers.insert_or_assign(_member, memberView);
}

void NodeView::setOwner(Node* _node)
{
    std::vector<Member*> notExposedMembers;

    //  We expose first the members which allows input connections
    for(auto& m : _node->getMembers())
    {
        auto member = m.second;
        if (member->getVisibility() == Visibility::Always && member->allowsConnection(Way_In) )
        {
           this->exposeMember(member, Way_In);
        }
        else
        {
            notExposedMembers.push_back(member);
        }
    }

    // Then we expose node which allows output connection (if they are not yet exposed)
    for (auto& member : notExposedMembers)
    {
        if (member->getVisibility() == Visibility::Always && member->allowsConnection(Way_Out))
        {
            this->exposeMember(member, Way_Out);
        }
    }

    // Determine a color depending on node type
    if (_node->hasComponent<ComputeBase>())
    {
        setColor(ColorType_Fill, ImColor(0.7f, 0.7f, 0.9f)); // blue
    }
    else if ( _node->getClass() == mirror::GetClass<VariableNode>() )
    {
        setColor(ColorType_Fill, ImColor(0.9f, 0.9f, 0.7f)); // purple
    }
    else
    {
        setColor(ColorType_Fill, ImColor(0.7f, 0.9f, 0.7f)); // green
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

bool NodeView::IsAnyDragged()
{
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

ImVec2 NodeView::getPosition()const
{
	return ImVec2(std::round(position.x), std::round(position.y));
}

const MemberView* NodeView::getMemberView(const Member* _member)const
{
    return exposedMembers.at(_member);
}

ImVec2 NodeView::getConnectorPosition(const Member *_member, Way _way)const
{
    ImVec2 pos = position;

	auto memberView = getMemberView(_member);
    if (memberView)
    {
        pos = memberView->screenPos;
    }

	auto nodeViewScreenPosition = View::CursorPosToScreenPos(position);

	// Input => Top
	if (_way == Way_In)
    {
		return ImVec2(pos.x , nodeViewScreenPosition.y - size.y * 0.5f);
    }
	// Outputs => Bottom
	return ImVec2(pos.x, nodeViewScreenPosition.y + size.y * 0.5f);
}

void NodeView::setPosition(ImVec2 _position)
{
	this->position = _position;
}

void NodeView::translate(ImVec2 _delta, bool _translateInputsRecursively)
{
	this->setPosition( position + _delta);

	if ( _translateInputsRecursively )
    {
	    for(auto eachInput : getOwner()->getInputs() )
        {
	        if ( NodeView* eachInputView = eachInput->getComponent<NodeView>() )
	        {
                eachInputView->translate(_delta, true);
	        }
        }
    }
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

bool NodeView::update(float _deltaTime)
{
    Maths::linear_interpolation( opacity, 1.0f, 10.0f * _deltaTime);

	auto node = getOwner();
	NODABLE_ASSERT(node != nullptr);

	/*
	 * Apply constraints to view, 3 possible ways
	 *
	 * 1 - move this to children average position
	 * 2 - move this to children's bounding box's top
	 * 3 - move input connected NodeViews recursively
	 */

	// 1 - move this to children average position
	if ( node->getClass() == mirror::GetClass<CodeBlockNode>() && !pinned)
    {
	    ImVec2 childrenPosSum;
	    auto codeBlockNode = node->as<CodeBlockNode>();
	    for( auto& children: codeBlockNode->getInstructions() )
        {
            childrenPosSum += children->getComponent<NodeView>()->getPosition();
        }
	    float count(codeBlockNode->getInstructions().size());
	    ImVec2 childrenPosAvg(childrenPosSum.x / count,childrenPosSum.y / count);
	    this->setPosition(childrenPosAvg + ImVec2(0,size.y * 2.0f));

    // 2 - move this to children's bounding box's top
    } else if ( node->getOutputWireCount() > 1 && !pinned )
    {
        // create an bounding box
        std::vector<float> x_positions, y_positions;
        for (auto eachWire : node->getWires())
        {
            bool isWireAnOutput = node->has(eachWire->getSource());
            if (isWireAnOutput)
            {
                auto targetNode = eachWire->getTarget()->getOwner()->as<Node>();
                auto targetNodeView  = targetNode->getComponent<NodeView>();
                x_positions.push_back( targetNodeView->position.x );
                y_positions.push_back( targetNodeView->position.y );
            }
        }
        constexpr float float_max = std::numeric_limits<float>::max();
        auto x_minmax = std::minmax_element(x_positions.begin(), x_positions.end());
        auto y_minmax = std::minmax_element(y_positions.begin(), y_positions.end());
        ImRect zone(*x_minmax.first, *y_minmax.first, *x_minmax.second, *y_minmax.second );

        // Compute a delta to apply to move to this new position and translate.
        ImVec2 newPos( zone.GetCenter().x, zone.Min.y - size.y * 2.0f);
        ImVec2 delta(newPos.x - position.x, newPos.y - position.y);
        bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
        if (!isDeltaTooSmall) {
            auto factor = std::min(1.0f, 10.f * _deltaTime);
            translate(delta * factor);
        }

    // 3 - move input connected NodeViews recursively
    } else if( node->getInputWireCount() > 0)
    {
        // then we constraint each input view
        auto wires = node->getWires();
        auto inputIndex = 0;

        // Compute the cumulated height and the size x max of the input node view:
        auto cumulatedSize = 0.0f;
        auto sizeMax = 0.0f;
        for (auto eachWire : wires)
        {
            auto sourceNode = eachWire->getSource()->getOwner()->as<Node>(); // TODO: add some checks
            bool isWireAnInput = node->has(eachWire->getTarget());
            auto inputView = sourceNode->getComponent<NodeView>();

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
                auto inputView = sourceNode->getComponent<NodeView>();

                // Contrain only unpinned node that have only a single output connection
                if (!inputView->pinned && inputView->getOwner()->getOutputWireCount() <= 1)
                {
                    // Compute new position for this input view
                    ImVec2 newPos(
                            posX + inputView->size.x / 2.0f,
                            position.y - s_nodeSpacingDistance - inputView->size.y / 2.0f - size.y / 2.0f
                            );
                    posX += inputView->size.x + nodeSpacing;

                    // Compute a delta to apply to move to this new position
                    auto currentPos = inputView->position;
                    ImVec2 delta((newPos.x - currentPos.x), (newPos.y - currentPos.y));

                    bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
                    if (!isDeltaTooSmall)
                    {
                        auto factor = std::min(1.0f, 10.f * _deltaTime);
                        inputView->translate(delta * factor);
                    }
                }

                inputIndex++;
            }
        }
    }

	// follow previous instruction
    if ( node->getClass() == mirror::GetClass<InstructionNode>() && !pinned)
    {
        if ( Node* codeBlock = node->getParent() )
        {
            auto children = codeBlock->getChildren();
            auto found = std::find( children.begin(), children.end(), node);
            if ( found != children.end() && found != children.begin() )
            {
                auto prevInstrView = (*(found - 1))->getComponent<NodeView>();
                auto prevInstrPos  = prevInstrView->getPosition();
                auto prevInstrRect = prevInstrView->computeBoundingRectRecursively();

                ImVec2 offset = ImVec2(prevInstrRect.Max.x - this->computeBoundingRectRecursively().Min.x, 0);
                ImVec2 newPos( position.x + offset.x + 20.0f, prevInstrPos.y);

                ImVec2 delta((newPos.x - position.x), (newPos.y - position.y));
                bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
                if (!isDeltaTooSmall)
                {
                    auto factor = std::min(1.0f, 10.f * _deltaTime);
                    translate(delta * factor, true);
                }
            }
        }
    }

	return true;
}

bool NodeView::draw()
{
	bool edited = false;
	auto node   = getOwner();

	NODABLE_ASSERT(node != nullptr);

	// Mouse interactions
	//-------------------

	if (GetDragged() == this && ImGui::IsMouseDragging(0))
	{
		translate(ImGui::GetMouseDragDelta());
		ImGui::ResetMouseDragDelta();
		pinned = true;
	}

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
	const auto halfSize = size / 2.0;
	ImGui::SetCursorPos(getPosition() - halfSize );
	ImGui::PushID(this);
	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = View::CursorPosToScreenPos(getPosition() );

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
	ShadowedText(ImVec2(1.0f), getColor(ColorType_BorderHighlights), getLabel().c_str()); // text with a lighter shadow (incrust effect)

	// Draw inputs
    for( auto& memberView : this->exposedInputsMembers )
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(cursorPositionBeforeContent.y + 1.0f);
        drawMemberView(memberView);
    }

    // Draw outputs
    for( auto& memberView : this->exposedOutputMembers )
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(cursorPositionBeforeContent.y + 8.0f);
        drawMemberView(memberView);
    }

	ImGui::SameLine();

	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + nodePadding );
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + nodePadding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    size.x = std::ceil( ImGui::GetItemRectSize().x );
    size.y = std::max(NODE_VIEW_DEFAULT_SIZE.y, std::ceil( ImGui::GetItemRectSize().y ));

	// Draw input connectors
    for( auto& memberView : exposedInputsMembers )
    {
        drawMemberConnectors(memberView->member);
    }

	// Draw out connectors
    for( auto& memberView : exposedOutputMembers )
    {
        drawMemberConnectors(memberView->member);
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
		ImGui::MenuItem("Collapsed", "", &this->forceMemberInputVisible, true);
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
		if( GetDragged() == nullptr && ImGui::IsMouseDown(0) && hovered && ImGui::IsMouseDragPastThreshold(0))
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
		this->forceMemberInputVisible = !this->forceMemberInputVisible;

        for( auto& pair : exposedMembers )
        {
            auto& eachMemberView = pair.second;
            eachMemberView->touched = forceMemberInputVisible;
            eachMemberView->showInput = forceMemberInputVisible;
        }
	}

	ImGui::PopStyleVar();
	ImGui::PopID();

	return edited;
}

void NodeView::ArrangeRecursively(NodeView* _view)
{

    if ( _view->getOwner()->getClass() == mirror::GetClass<CodeBlockNode>() )
    {
        NodeView::ArrangeRecursively( _view->getOwner()->as<CodeBlockNode>());
    } else
    {

        // Force and update of input connected nodes with a delta time extra high
        // to ensure all nodes were well placed in a single call (no smooth moves)
        _view->update(float(1000));

        // Get wires that go outside from this node :
        auto wires = _view->getOwner()->getWires();

        for (auto eachWire : wires)
        {
            if (eachWire != nullptr && _view->getOwner()->has(eachWire->getTarget()))
            {

                if (eachWire->getSource() != nullptr)
                {
                    auto node = dynamic_cast<Node *>(eachWire->getSource()
                            ->getOwner());
                    auto inputView = node->getComponent<NodeView>();
                    inputView->pinned = false;
                    ArrangeRecursively(inputView);
                }
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

bool NodeView::drawMemberView(MemberView* _memberView )
{
    bool edited = false;
    Member* member = _memberView->member;

    if( !_memberView->touched )
    {
        const bool isAnInputUnconnected = member->getInputMember() != nullptr || !member->allowsConnection(Way_In);
        const bool isVariable = member->getOwner()->getClass() == VariableNode::GetClass();
        _memberView->showInput = _memberView->member->isDefined() && (!isAnInputUnconnected || isVariable || s_viewDetail == NodeViewDetail::Exhaustive) ;
    }

    _memberView->screenPos = ImGui::GetCursorScreenPos();

    // input
    if ( _memberView->showInput )
    {
        // try to draw an as small as possible input field
        float inputWidth = 5.0f + std::max( ImGui::CalcTextSize(((std::string)*member).c_str()).x, NodeView::s_memberInputSizeMin );
        _memberView->screenPos.x += inputWidth / 2.0f;
        ImGui::PushItemWidth(inputWidth);
        edited = NodeView::DrawMemberInput(member);
        ImGui::PopItemWidth();
    }
    else
    {
        ImGui::Button("", NodeView::s_memberInputToggleButtonSize);
        _memberView->screenPos.x += NodeView::s_memberInputToggleButtonSize.x / 2.0f;

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
            _memberView->showInput = !_memberView->showInput;
            _memberView->touched = true;
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

            if (ImGui::Checkbox(label.c_str(), &b ) && !_member->hasInputConnected() )
            {
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
        auto language = node->getParentGraph()->getLanguage();
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
	return ImRect(getPosition() - size * 0.5f, getPosition() + size * 0.5f);
}


bool Nodable::NodeView::IsInsideRect(NodeView* _nodeView, ImRect _rect) {
	auto nodeRect = _nodeView->getRect();
	return _rect.Contains(nodeRect);
}

void Nodable::NodeView::DrawNodeViewAsPropertiesPanel(NodeView* _view)
{
    const float labelColumnWidth = ImGui::GetContentRegionAvailWidth() / 2.0f;

    auto drawMember = [&](Member* _member)
    {
        // label (<name> (<way> <type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s, %s): ",
                _member->getName().c_str(),
                WayToString(_member->getConnectorWay()).c_str(),
                _member->getTypeAsString().c_str());

        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
        NodeView::DrawMemberInput(_member);
    };

    // Draw exposed input members
    ImGui::Text("Inputs:");
    ImGui::Indent();
    for (auto& eachView : _view->exposedInputsMembers )
    {
        drawMember(eachView->member);
    }
    ImGui::Unindent();

    // Draw exposed output members
    ImGui::NewLine();
    ImGui::Text("Outputs:");
    ImGui::Indent();
    for (auto& eachView : _view->exposedOutputMembers )
    {
        drawMember(eachView->member);
    }
    ImGui::Unindent();

    // Advanced properties
    ImGui::NewLine();
   _view->drawAdvancedProperties();
}

void Nodable::NodeView::ConstraintToRect(NodeView* _view, ImRect _rect)
{
	
	if ( !NodeView::IsInsideRect(_view, _rect)) {

		_rect.Expand(ImVec2(-2, -2)); // shrink

		auto nodeRect = _view->getRect();

		auto newPos = _view->getPosition();

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

bool NodeView::isMemberExposed(const Member *_member)const
{
    return exposedMembers.find(_member) != exposedMembers.end();
}

void NodeView::drawAdvancedProperties()
{
    const Node* node = getOwner();
    const float indent = 20.0f;

    // Components
    ImGui::Text("Components :");
    for (auto& pair : node->getComponents())
    {
        auto component	= pair.second;
        auto name		= pair.first;
        auto className	= component->getClass()->getName();
        ImGui::BulletText("%s", className);
    }

    // Parent container
    ImGui::NewLine();
    std::string parentName = "NULL";

    if (node->getParentGraph() )
    {
        parentName = node->getParentGraph()->getLabel();
    }

    ImGui::Indent();
    ImGui::Text("Parent is \"%s\"", parentName.c_str());
    // ImGui::Text("Is an instruction result: %s", node-> ? "YES" : "NO");
}

void NodeView::SetDetail(NodeViewDetail _viewDetail)
{
    NodeView::s_viewDetail = _viewDetail;

    for( auto& eachView : NodeView::s_instances)
    {
        for( auto& eachPair : eachView->exposedMembers )
        {
            MemberView* memberView = eachPair.second;
            memberView->reset();
        }
    }
}

void NodeView::ArrangeRecursively(CodeBlockNode* _block)
{
    for(auto& eachChild: _block->getChildren())
    {
        auto view = eachChild->getComponent<NodeView>();

        if ( view )
        {
            NodeView::ArrangeRecursively(view);
            view->pinned = false;
        }
    }
}

ImVec2 NodeView::getScreenPos()
{
    ImVec2 offset(
            ImGui::GetCursorPos().x - ImGui::GetCursorScreenPos().x,
            ImGui::GetCursorPos().y - ImGui::GetCursorScreenPos().y);
    return position - offset;
}

ImRect NodeView::computeBoundingRectRecursively(bool _ignorePinned)
{

    std::vector<float> x;
    std::vector<float> y;

    ImRect rect = this->getRect();
    x.push_back(rect.Min.x);
    x.push_back(rect.Max.x);
    y.push_back(rect.Min.y);
    y.push_back(rect.Max.y);


    for(Node* eachChild : this->getOwner()->getInputs() )
    {
        NodeView* childView = eachChild->getComponent<NodeView>();
        if ( childView && !( childView->pinned && _ignorePinned) )
        {
            ImRect childRect = childView->computeBoundingRectRecursively();
            x.push_back(childRect.Min.x);
            x.push_back(childRect.Max.x);
            y.push_back(childRect.Min.y);
            y.push_back(childRect.Max.y);
        }
    }

    auto minmax_x = std::minmax_element(x.begin(), x.end());
    auto minmax_y = std::minmax_element(y.begin(), y.end());

    return ImRect(
            ImVec2 (*minmax_x.first, *minmax_y.first), // min
            ImVec2 (*minmax_x.second, *minmax_x.second) // max
    );
}
