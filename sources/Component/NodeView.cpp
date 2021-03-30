#include "NodeView.h"

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max

#include "Core/Application.h"
#include "Core/Maths.h"
#include "Core/Wire.h"
#include "Core/Log.h"		     // for LOG_DEBUG(...)
#include "Component/ComputeBase.h"
#include "Language/Common/Serializer.h"
#include "Node/AbstractCodeBlockNode.h"
#include "Node/NodeTraversal.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/GraphNode.h"
#include "Node/VariableNode.h"
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
std::vector<NodeView*> NodeView::s_instances;

const float ViewConstraint::s_viewSpacing = 15.0f;
const float ViewConstraint::s_viewSpeed   = 20.0f;

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
        return std::string(node->getShortLabel());
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

void NodeView::translate(ImVec2 _delta, bool _recurse)
{
	this->setPosition( position + _delta);

	if ( _recurse )
    {
	    for(auto eachInput : getOwner()->getInputs() )
        {
	        if ( NodeView* eachInputView = eachInput->getComponent<NodeView>() )
	        {
	            if ( !eachInputView->pinned )
                    eachInputView->translate(_delta, true);
	        }
        }
    }
}

void NodeView::arrangeRecursively(bool _smoothly)
{
    this->pinned = false;
	ArrangeRecursively(this, _smoothly);
}

bool NodeView::update()
{
	auto deltaTime = ImGui::GetIO().DeltaTime;

	return update(deltaTime);
}

bool NodeView::update(float _deltaTime)
{
    Maths::linear_interpolation( opacity, 1.0f, 10.0f * _deltaTime);
    this->applyConstraints(_deltaTime);
    this->applyForces();
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

void NodeView::ArrangeRecursively(NodeView* _view, bool _smoothly)
{
    std::vector<NodeView*> views;

    // Get input views
    for (auto eachInput : _view->getOwner()->getInputs())
    {
        auto inputView = eachInput->getComponent<NodeView>();
        if ( inputView )
            views.push_back(inputView);
    }

    // Get input views
    for (auto eachChild : _view->getOwner()->getChildren())
    {
        auto inputView = eachChild->getComponent<NodeView>();
        if ( inputView )
            views.push_back(inputView);
    }

    // Recursive calls
    for(auto eachView : views)
    {
        eachView->pinned = false;
        ArrangeRecursively(eachView, _smoothly);
    }

    // Force and update of input connected nodes with a delta time extra high
    // to ensure all nodes were well placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        _view->update(float(1000));
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
                NodeTraversal traversal;
                traversal.setDirty(node);
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
                NodeTraversal traversal;
                traversal.setDirty(node);
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
                NodeTraversal traversal;
                traversal.setDirty(node);
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

    // Parent graph
    {
        ImGui::NewLine();
        std::string parentName = "NULL";

        if (node->getParentGraph() )
        {
            parentName = node->getParentGraph()->getLabel();
            parentName.append( node->getParentGraph()->isDirty() ? " (dirty)" : "");

        }
        ImGui::Text("Parent graph is \"%s\"", parentName.c_str());
    }

    // Parent
    {
        ImGui::NewLine();
        std::string parentName = "NULL";

        if (node->getParent() )
        {
            parentName = node->getParent()->getLabel();
            parentName.append( node->getParent()->isDirty() ? " (dirty)" : "");
        }
        ImGui::Text("Parent node is \"%s\"", parentName.c_str());

    }

    // dirty state
    ImGui::NewLine();
    bool b = getOwner()->isDirty();
    ImGui::Checkbox("Is dirty ?", &b);

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

ImVec2 NodeView::getScreenPos()
{
    ImVec2 offset(
            ImGui::GetCursorPos().x - ImGui::GetCursorScreenPos().x,
            ImGui::GetCursorPos().y - ImGui::GetCursorScreenPos().y);
    return position - offset;
}

ImRect NodeView::getRect(bool _recursively, bool _ignorePinned, bool _ignoreMultiConstrained, bool _ignoreSelf)
{

    if( !_recursively)
    {
        return ImRect(this->position - size * 0.5f, this->position + size * 0.5f);
    }

    std::vector<float> x;
    std::vector<float> y;

    if ( !_ignoreSelf)
    {
        ImRect rect = this->getRect(false);
        x.push_back(rect.Min.x);
        x.push_back(rect.Max.x);
        y.push_back(rect.Min.y);
        y.push_back(rect.Max.y);
    }

    for(Node* eachChild : this->getOwner()->getInputs() )
    {
        NodeView* childView = eachChild->getComponent<NodeView>();
        if ( childView &&
             !(childView->pinned && _ignorePinned) &&
             !(childView->getOwner()->getOutputs().size() > 1 && _ignoreMultiConstrained) )
        {
            ImRect childRect = childView->getRect(true, _ignorePinned, _ignoreMultiConstrained);
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
            ImVec2 (*minmax_x.second, *minmax_y.second) // max
    );
}

void NodeView::clearConstraints() {
    this->constraints.clear();
}

void NodeView::addConstraint(ViewConstraint _constraint) {
    this->constraints.push_back(std::move(_constraint));
}

void NodeView::applyConstraints(float _dt) {
    for ( ViewConstraint& eachConstraint : this->constraints)
    {
        eachConstraint.apply(_dt);
    }
}

bool NodeView::isPinned() const {
    return this->pinned;
}

ImVec2 NodeView::getSize() const {
    return this->size;
}

void NodeView::addForceToTranslateTo(ImVec2 desiredPos, float _factor, bool _recurse)
{
    ImVec2 delta(desiredPos - position);
    auto factor = std::min(1.0f, _factor);
    addForce(delta * factor, _recurse);
}

void NodeView::addForce(ImVec2 force, bool _recurse)
{
    forces += force;

    if ( _recurse )
    {
        for(auto eachInput : getOwner()->getInputs() )
        {
            if ( NodeView* eachInputView = eachInput->getComponent<NodeView>() )
            {
                if ( !eachInputView->pinned )
                    eachInputView->addForce(force, _recurse);
            }
        }
    }
}

void NodeView::applyForces(bool _recurse)
{
    // apply
    bool tooSmall = forces.x * forces.x + forces.y * forces.y < 0.1f;
    if (!tooSmall)
        this->translate(forces);

    // reset
    forces = ImVec2();
}

void NodeView::translateTo(ImVec2 desiredPos, float _factor, bool _recurse) {

    ImVec2 delta(desiredPos - position);

    bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
    if (!isDeltaTooSmall)
    {
        auto factor = std::min(1.0f, _factor);
        translate(delta * factor, _recurse);
    }
}

ImRect NodeView::GetRect(
        const std::vector<NodeView *>& _views,
        bool _recursive,
        bool _ignorePinned,
        bool _ignoreMultiConstrained) {

    std::vector<float> x_positions, y_positions;
    for (auto eachView : _views)
    {
        auto rect = eachView->getRect(_recursive, _ignorePinned, _ignoreMultiConstrained);
        x_positions.push_back(rect.Min.x );
        x_positions.push_back(rect.Max.x );
        y_positions.push_back(rect.Min.y );
        y_positions.push_back(rect.Max.y );
    }
    auto x_minmax = std::minmax_element(x_positions.begin(), x_positions.end());
    auto y_minmax = std::minmax_element(y_positions.begin(), y_positions.end());

    return ImRect(*x_minmax.first, *y_minmax.first, *x_minmax.second, *y_minmax.second );;
}

ViewConstraint::ViewConstraint(ViewConstraint::Type _type):type(_type) {}

void ViewConstraint::apply(float _dt) {

    LOG_VERBOSE("ViewConstraint", "applying constraint\n");
    auto master = masters.at(0);

    switch ( this->type )
    {
        case Type::AlignOnBBoxLeft:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() )
            {
                ImRect bbox = NodeView::GetRect(masters, true);
                ImVec2 newPos(bbox.GetCenter() - ImVec2(bbox.GetSize().x * 0.5 + s_viewSpacing + slave->getRect().GetSize().x * 0.5, 0 ));
                slave->addForceToTranslateTo(newPos + offset, _dt * s_viewSpeed);
            }

            break;
        }

        case Type::AlignOnBBoxTop:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() )
            {
                ImRect bbox = NodeView::GetRect(masters, true);
                ImVec2 newPos(bbox.GetCenter());
                newPos.y -= bbox.GetSize().y / 2.0f + s_viewSpacing + slave->getRect().GetSize().y / 2.0f;
                slave->addForceToTranslateTo(newPos + offset, _dt * s_viewSpeed);
            }

            break;
        }

        case Type::MakeRowAndAlignOnBBoxTop:
        {
            auto inputIndex = 0;

            // Compute the cumulated width and the size y max of the input node view:
            auto cumulatedSize = 0.0f;
            auto sizeMax = 0.0f;
            for (auto eachSlave : slaves) {
                if (!eachSlave->isPinned()) {
                    auto sx = eachSlave->getSize().x;
                    cumulatedSize += sx;
                    sizeMax = std::max(sizeMax, sx);
                }
            }

            auto posX = master->getPosition().x - cumulatedSize / 2.0f;

            // TODO: remove this "hack"
            if ( master->getOwner()->getClass() == mirror::GetClass<InstructionNode>())
            {
                posX += cumulatedSize / 2.0f + s_viewSpacing + master->getSize().x / 2.0f;
            }

            float nodeSpacing(10);

            for (auto eachSlave : slaves)
            {
                // Contrain only unpinned node that have only a single output connection
                if (!eachSlave->isPinned() && eachSlave->getOwner()->getOutputs().size() <= 1) {
                    // Compute new position for this input view
                    ImVec2 eachDrivenNewPos = ImVec2(
                            posX + eachSlave->getSize().x / 2.0f,
                            master->getPosition().y - s_viewSpacing - eachSlave->getSize().y / 2.0f - master->getSize().y / 2.0f
                    );
                    posX += eachSlave->getSize().x + nodeSpacing;
                    eachSlave->addForceToTranslateTo(eachDrivenNewPos + offset, _dt * s_viewSpeed, true);
                }
                inputIndex++;
            }
            break;
        }

        case Type::FollowWithChildren:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() )
            {
                // compute
                auto masterRect = master->getRect();
                auto slaveRect = slave->getRect(true,true, true);
                ImVec2 slaveMasterOffset(masterRect.Max - slaveRect.Min);
                ImVec2 newPos(master->getPosition().x, slave->getPosition().y + slaveMasterOffset.y + s_viewSpacing);

                // apply
                slave->addForceToTranslateTo(newPos + offset, _dt * s_viewSpeed * 0.8, true);
                break;
            }
        }

        case Type::Follow:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() )
            {
                // compute
                ImVec2 newPos(master->getPosition() + ImVec2(0.0f, master->getSize().y));
                newPos.y += s_viewSpacing + slave->getSize().y;

                // apply
                slave->addForceToTranslateTo(newPos + offset, _dt * s_viewSpeed);
                break;
            }
        }
    }
}

void ViewConstraint::addSlave(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->slaves.push_back(_subject);
}

void ViewConstraint::addMaster(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->masters.push_back(_subject);
}
