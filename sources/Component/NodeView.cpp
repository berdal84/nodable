#include "NodeView.h"

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include <vector>
#include <Settings.h>
#include "IconsFontAwesome5.h"

#include "Core/Application.h"
#include "Core/Maths.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/VariableNode.h"
#include "Node/InstructionNode.h"
#include "Node/LiteralNode.h"

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

NodeView::NodeView():
        m_position(500.0f, -1.0f),
        m_size(NODE_VIEW_DEFAULT_SIZE),
        m_opacity(1.0f),
        m_childrenVisible(true),
        m_forceMemberInputVisible(false),
        m_pinned(false),
        m_borderRadius(5.0f),
        m_borderColorSelected(1.0f, 1.0f, 1.0f)
{
    NodeView::s_instances.push_back(this);
}

NodeView::~NodeView()
{
    // delete MemberViews
    for ( auto& pair: m_exposedMembers )
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
        m_exposedInputsMembers.push_back(memberView);
    }
    else // Way_Out
    {
        m_exposedOutputMembers.push_back(memberView);
    }

    m_exposedMembers.insert_or_assign(_member, memberView);
}

void NodeView::setOwner(Node* _node)
{
    std::vector<Member*> notExposedMembers;

    //  We expose first the members which allows input connections
    for(auto& m : _node->getProps()->getMembers())
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
    auto settings = Settings::GetCurrent();
    if (_node->hasComponent<ComputeBase>())
    {
        setColor(ColorType_Fill, &settings->ui.node.functionColor); // blue
    }
    else if ( _node->getClass() == mirror::GetClass<VariableNode>() )
    {
        setColor(ColorType_Fill, &settings->ui.node.variableColor); // purple
    }
    else if ( _node->getClass() == mirror::GetClass<LiteralNode>() )
    {
        setColor(ColorType_Fill, &settings->ui.node.literalColor);
    }
    else
    {
        setColor(ColorType_Fill, &settings->ui.node.instructionColor); // green
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
	return ImVec2(std::round(m_position.x), std::round(m_position.y));
}

const MemberView* NodeView::getMemberView(const Member* _member)const
{
    return m_exposedMembers.at(_member);
}

ImVec2 NodeView::getConnectorPosition(const Member *_member, Way _way)const
{
    ImVec2 pos = m_position;

	auto memberView = getMemberView(_member);
    if (memberView)
    {
        pos = memberView->screenPos;
    }

	auto nodeViewScreenPosition = View::CursorPosToScreenPos(m_position);

	// Input => Top
	if (_way == Way_In)
    {
		return ImVec2(pos.x , nodeViewScreenPosition.y - m_size.y * 0.5f);
    }
	// Outputs => Bottom
	return ImVec2(pos.x, nodeViewScreenPosition.y + m_size.y * 0.5f);
}

void NodeView::setPosition(ImVec2 _position)
{
	m_position = _position;
}

void NodeView::translate(ImVec2 _delta, bool _recurse)
{
	this->setPosition(m_position + _delta);

	if ( _recurse )
    {
	    for(auto eachInput : getOwner()->getInputs() )
        {
	        if ( NodeView* eachInputView = eachInput->getComponent<NodeView>() )
	        {
	            if (!eachInputView->m_pinned && eachInputView->shouldFollowOutput(this) )
                    eachInputView->translate(_delta, true);
	        }
        }
    }
}

void NodeView::arrangeRecursively(bool _smoothly)
{
    std::vector<NodeView*> views;

    for (auto inputView : m_inputs)
    {
        if ( inputView->shouldFollowOutput(this))
        {
            views.push_back(inputView);
            inputView->arrangeRecursively();
        }
    }

    for (auto eachChild : m_children)
    {
        views.push_back(eachChild);
        eachChild->arrangeRecursively();
    }

//     Force and update of input connected nodes with a delta time extra high
//     to ensure all nodes were well placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_pinned = false;
}

bool NodeView::update()
{
	auto deltaTime = ImGui::GetIO().DeltaTime;

	return update(deltaTime);
}

bool NodeView::update(float _deltaTime)
{
    Maths::linear_interpolation(m_opacity, 1.0f, 10.0f * _deltaTime);
    this->applyForces(_deltaTime, false);
	return true;
}

bool NodeView::draw()
{
	bool edited = false;
	auto node   = getOwner();

	auto settings = Settings::GetCurrent();

	NODABLE_ASSERT(node != nullptr);

	// Mouse interactions
	//-------------------

	if (GetDragged() == this && ImGui::IsMouseDragging(0))
	{
		translate(ImGui::GetMouseDragDelta(), true);
		ImGui::ResetMouseDragDelta();
        m_pinned = true;
	}

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
	const auto halfSize = m_size / 2.0;
	ImGui::SetCursorPos(getPosition() - halfSize );
	ImGui::PushID(this);
	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = View::CursorPosToScreenPos(getPosition() );

	// Draw the background of the Group
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	{			
		auto borderCol = IsSelected(this) ? m_borderColorSelected : getColor(ColorType_Border);

		auto itemRectMin = screenPosition - halfSize;
		auto itemRectMax = screenPosition + halfSize;

		// Draw the rectangle under everything
		View::DrawRectShadow(itemRectMin, itemRectMax, m_borderRadius, 4, ImVec2(1.0f), getColor(ColorType_Shadow));
		draw_list->AddRectFilled(itemRectMin, itemRectMax, getColor(ColorType_Fill), m_borderRadius);
		draw_list->AddRect(itemRectMin + ImVec2(1.0f), itemRectMax, getColor(ColorType_BorderHighlights), m_borderRadius);
		draw_list->AddRect(itemRectMin, itemRectMax, borderCol, m_borderRadius);

		// darken the background under the content
		draw_list->AddRectFilled(itemRectMin + ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + settings->ui.node.padding), itemRectMax, ImColor(0.0f, 0.0f, 0.0f, 0.1f), m_borderRadius, 4);

		// Draw an additionnal blinking rectangle when selected
		if (IsSelected(this))
		{
			auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
			float offset = 4.0f;
			draw_list->AddRect(itemRectMin - ImVec2(offset), itemRectMax + ImVec2(offset), ImColor(1.0f, 1.0f, 1.0f, float(alpha) ), m_borderRadius + offset, ~0, offset / 2.0f);
		}
	}

	// Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorPos(cursorPositionBeforeContent);
	ImGui::InvisibleButton("##", m_size);
	ImGui::SetItemAllowOverlap();
	hovered = ImGui::IsItemHovered();
	ImGui::SetCursorPos(cursorPositionBeforeContent + settings->ui.node.padding );

	// Draw the window content
	//------------------------
    ImGui::BeginGroup();
	ShadowedText(ImVec2(1.0f), getColor(ColorType_BorderHighlights), getLabel().c_str()); // text with a lighter shadow (incrust effect)

	// Draw inputs
    for( auto& memberView : m_exposedInputsMembers )
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(cursorPositionBeforeContent.y + 1.0f);
        edited |= drawMemberView(memberView);
    }

    // Draw outputs
    for( auto& memberView : m_exposedOutputMembers )
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(cursorPositionBeforeContent.y + 8.0f);
        edited |= drawMemberView(memberView);
    }

    // If needed, show a button to show/hide children and inputs. now accessible with "X" or Edit->Expand
//    if ( !getOwner()->getChildren().empty() || !getOwner()->getInputs().empty() )
//    {
//        ImGui::SameLine();
//        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
//        if ( ImGui::Button(m_childrenVisible ? ICON_FA_MINUS : ICON_FA_PLUS, ImVec2(20.0f, 20.0f)) )
//        {
//            toggleExpansion();
//        }
//        ImGui::PopStyleVar();
//    }

	ImGui::SameLine();

	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings->ui.node.padding );
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + settings->ui.node.padding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    m_size.x = std::ceil(ImGui::GetItemRectSize().x );
    m_size.y = std::max(NODE_VIEW_DEFAULT_SIZE.y, std::ceil(ImGui::GetItemRectSize().y ));

	// Draw input connectors
    for( auto& memberView : m_exposedInputsMembers )
    {
        drawMemberConnectors(memberView->member, settings->ui.node.connectorRadius);
    }

	// Draw out connectors
    for( auto& memberView : m_exposedOutputMembers )
    {
        drawMemberConnectors(memberView->member, settings->ui.node.connectorRadius);
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

        ImGui::MenuItem("Pinned", "", &m_pinned, true);
		ImGui::MenuItem("Collapsed", "", &m_forceMemberInputVisible, true);
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
		m_forceMemberInputVisible = !m_forceMemberInputVisible;

        for( auto& pair : m_exposedMembers )
        {
            auto& eachMemberView = pair.second;
            eachMemberView->touched = m_forceMemberInputVisible;
            eachMemberView->showInput = m_forceMemberInputVisible;
        }
	}

	ImGui::PopStyleVar();
	ImGui::PopID();

	if( edited )
	    getOwner()->setDirty();

	return edited;
}

void NodeView::drawMemberConnectors(Member* _member, float _connectorRadius)
{
    /*
    Draw the wire connectors (In or Out only)
   */

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (_member->allowsConnection(Way_In))
    {
        ImVec2      connectorPos = getConnectorPosition( _member, Way_In);
        drawConnector(connectorPos, _member->input(), draw_list, _connectorRadius);
    }

    if (_member->allowsConnection(Way_Out))
    {
        ImVec2      connectorPos = getConnectorPosition( _member, Way_Out);
        drawConnector(connectorPos, _member->output(), draw_list, _connectorRadius);
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
        const bool isLiteral  = member->getOwner()->getClass() == LiteralNode::GetClass();
        _memberView->showInput = _memberView->member->isDefined() && (!isAnInputUnconnected || isLiteral || isVariable || s_viewDetail == NodeViewDetail::Exhaustive) ;
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

    Node* node  = _member->getOwner();

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
        case Type_Double:
        {
            auto f = (double)*_member;

            if (ImGui::InputDouble(label.c_str(), &f, 0.0F, 0.0F, "%g", inputFlags ) && !_member->hasInputConnected())
            {
                _member->set(f);
                GraphTraversal::TraverseAndSetDirty(node);
                edited |= true;
            }
            break;
        }

        case Type_String:
        {
            char str[255];
            snprintf(str, 255, "%s", ((std::string)*_member).c_str() );

            if ( ImGui::InputText(label.c_str(), str, 255, inputFlags) && !_member->hasInputConnected() )
            {
                _member->set(str);
                GraphTraversal::TraverseAndSetDirty(node);
                edited |= true;
            }
            break;
        }

        case Type_Boolean:
        {
            std::string checkBoxLabel = _member->getName();

            auto b = (bool)*_member;

            if (ImGui::Checkbox(label.c_str(), &b ) && !_member->hasInputConnected() )
            {
                _member->set(b);
                GraphTraversal::TraverseAndSetDirty(node);
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
        Serializer* serializer = node->getParentGraph()->getLanguage()->getSerializer();
        std::string buffer;
        serializer->serialize(buffer, _member);
        ImGui::Text("%s", buffer.c_str() );
        ImGui::EndTooltip();
    }

    return edited;
}

void NodeView::drawConnector(ImVec2& connnectorScreenPos, const Connector* _connector, ImDrawList* draw_list, float _connectorRadius)
{
	// Unvisible Button on top of the Circle

    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

	auto invisibleButtonOffsetFactor = 1.2f;
	ImGui::SetCursorScreenPos(connnectorScreenPos - ImVec2(_connectorRadius * invisibleButtonOffsetFactor));
	ImGui::PushID(_connector->member);
	bool clicked = ImGui::InvisibleButton("###", ImVec2(_connectorRadius * 2.0f * invisibleButtonOffsetFactor, _connectorRadius * 2.0f * invisibleButtonOffsetFactor));
	ImGui::PopID();
	ImGui::SetCursorScreenPos(cursorScreenPos);

	// Circle
	auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

	if (isItemHovered)
		draw_list->AddCircleFilled(connnectorScreenPos, _connectorRadius, getColor(ColorType_Highlighted));
	else
		draw_list->AddCircleFilled(connnectorScreenPos, _connectorRadius, getColor(ColorType_Fill));

	draw_list->AddCircle(connnectorScreenPos, _connectorRadius, getColor(ColorType_Border));


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
        ImGui::Text(R"(token: [%s,%s,%s])",
                    _member->getSourceToken()->m_prefix.c_str(),
                    _member->getSourceToken()->m_word.c_str(),
                    _member->getSourceToken()->m_suffix.c_str()
                    );
    };

    // Draw exposed input members
    ImGui::Text("Inputs:");
    ImGui::Indent();
    for (auto& eachView : _view->m_exposedInputsMembers )
    {
        drawMember(eachView->member);
    }
    ImGui::Unindent();

    // Draw exposed output members
    ImGui::NewLine();
    ImGui::Text("Outputs:");
    ImGui::Indent();
    for (auto& eachView : _view->m_exposedOutputMembers )
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
    return m_exposedMembers.find(_member) != m_exposedMembers.end();
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

    // Scope specific:

    if ( node->getClass()->isChildOf( mirror::GetClass<ScopedCodeBlockNode>() ))
    {
        ImGui::NewLine();
        ImGui::Text("Variables:");
        auto vars = node->as<ScopedCodeBlockNode>()->getVariables();
        for(auto eachVar : vars)
        {
            ImGui::Text("%s: %s", eachVar->getName(), ((std::string)*eachVar->value()).c_str());
        }
    }
}

void NodeView::SetDetail(NodeViewDetail _viewDetail)
{
    NodeView::s_viewDetail = _viewDetail;

    for( auto& eachView : NodeView::s_instances)
    {
        for( auto& eachPair : eachView->m_exposedMembers )
        {
            MemberView* memberView = eachPair.second;
            memberView->reset();
        }
    }
}

ImVec2 NodeView::getScreenPos()
{
    return m_position - (ImGui::GetCursorPos() - ImGui::GetCursorScreenPos());
}

ImRect NodeView::getRect(bool _recursively, bool _ignorePinned, bool _ignoreMultiConstrained, bool _ignoreSelf)
{

    if( !_recursively)
    {
        return ImRect(m_position - m_size * 0.5f, m_position + m_size * 0.5f);
    }

    ImRect rect(
            ImVec2(std::numeric_limits<float>().max()),
            ImVec2(-std::numeric_limits<float>().max()) );

    auto enlarge_to_fit = [&rect](const ImRect& other) {
        if( other.Min.x < rect.Min.x) rect.Min.x = other.Min.x;
        if( other.Min.y < rect.Min.y) rect.Min.y = other.Min.y;
        if( other.Max.x > rect.Max.x) rect.Max.x = other.Max.x;
        if( other.Max.y > rect.Max.y) rect.Max.y = other.Max.y;
    };

    if ( !_ignoreSelf)
    {
        ImRect self_rect = getRect(false);
        enlarge_to_fit(self_rect);
    }

    auto enlarge_to_fit_all = [&](NodeView* eachView) {
        if (eachView && eachView->isVisible() &&
            !(eachView->m_pinned && _ignorePinned) &&
            eachView->shouldFollowOutput(this)) {
            ImRect childRect = eachView->getRect(true, _ignorePinned, _ignoreMultiConstrained);
            enlarge_to_fit(childRect);
        }
    };

    std::for_each(m_children.begin(), m_children.end(), enlarge_to_fit_all);
    std::for_each(m_inputs.begin(), m_inputs.end(), enlarge_to_fit_all);

    return rect;
}

void NodeView::clearConstraints() {
    m_constraints.clear();
}

void NodeView::addConstraint(ViewConstraint _constraint) {
    m_constraints.push_back(std::move(_constraint));
}

void NodeView::applyConstraints(float _dt) {
    for ( ViewConstraint& eachConstraint : m_constraints)
    {
        eachConstraint.apply(_dt);
    }
}

bool NodeView::isPinned() const {
    return this->m_pinned;
}

ImVec2 NodeView::getSize() const {
    return this->m_size;
}

void NodeView::addForceToTranslateTo(ImVec2 desiredPos, float _factor, bool _recurse)
{
    ImVec2 delta(desiredPos - m_position);
    auto factor = std::min(1.0f, _factor);
    addForce(delta * factor, _recurse);
}

void NodeView::addForce(ImVec2 force, bool _recurse)
{
    m_forces += force;

    if ( _recurse )
    {
        for ( auto eachInputView : m_inputs )
        {
            if ( !eachInputView->m_pinned && eachInputView->shouldFollowOutput(this))
                eachInputView->addForce(force, _recurse);
        }
    }
}

void NodeView::applyForces(float _dt, bool _recurse) {
    //
    float mag = std::sqrt(m_forces.x * m_forces.x + m_forces.y * m_forces.y );

    // apply
    bool tooSmall = mag < 0.1f;
    if (!tooSmall)
    {
//        if ( mag * _dt > 200.0f)
//        {
//            forces.x *= 200.0f / mag;
//            forces.y *= 200.0f / mag;
//        }
        this->translate(m_forces, _recurse);

        // reset
    }
    m_forces = ImVec2();
}

void NodeView::translateTo(ImVec2 desiredPos, float _factor, bool _recurse) {

    ImVec2 delta(desiredPos - m_position);

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
        if ( eachView->isVisible())
        {
            auto rect = eachView->getRect(_recursive, _ignorePinned, _ignoreMultiConstrained);
            x_positions.push_back(rect.Min.x );
            x_positions.push_back(rect.Max.x );
            y_positions.push_back(rect.Min.y );
            y_positions.push_back(rect.Max.y );
        }
    }
    auto x_minmax = std::minmax_element(x_positions.begin(), x_positions.end());
    auto y_minmax = std::minmax_element(y_positions.begin(), y_positions.end());

    return ImRect(*x_minmax.first, *y_minmax.first, *x_minmax.second, *y_minmax.second );;
}

void NodeView::setChildrenVisible(bool _visible, bool _recursive)
{
    m_childrenVisible = _visible;

    for( auto eachChild : m_children )
    {
        eachChild->setVisible(_visible);

        if ( _recursive)
        {
            eachChild->setChildrenVisible(_visible, true);
            eachChild->setInputsVisible(_visible, true);
        }
    }
}

bool NodeView::shouldFollowOutput(const NodeView* output)
{
    if ( m_outputs.empty())
        return true;

    return m_outputs[0] == output;
//    NodeView* higher = nullptr;
//    for( auto eachChild : outputs )
//    {
//        if( eachChild->isVisible() && (!higher || higher->getPosition().y > eachChild->getPosition().y ))
//            higher = eachChild;
//    }
//    return higher == other;
}

void NodeView::setInputsVisible(bool _visible, bool _recursive)
{

    for( auto each_input : m_inputs )
    {
        if( _visible || (getOutputs().empty() || each_input->shouldFollowOutput(this)) )
        {
            if ( _recursive)
            {
                each_input->setChildrenVisible(_visible, true);
                each_input->setInputsVisible(_visible, true);
            }
            each_input->setVisible(_visible);
        }
    }
}

void NodeView::getNext(std::vector<NodeView *>& out)
{
    for( auto& each : getOwner()->getNext())
    {
         if ( each )
        {
             if ( auto each_view = each->getComponent<NodeView>() )
                 out.push_back(each_view);
        }
     }
}

void NodeView::toggleExpansion()
{
    bool visibility = !m_childrenVisible;
    setChildrenVisible(visibility, true);
    setInputsVisible(visibility, true);
}

ViewConstraint::ViewConstraint(ViewConstraint::Type _type):type(_type) {}

void ViewConstraint::apply(float _dt) {

    auto settings = Settings::GetCurrent();

    LOG_VERBOSE("ViewConstraint", "applying constraint\n");
    auto master = masters.at(0);

    switch ( this->type )
    {
        case Type::AlignOnBBoxLeft:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() && slave->isVisible())
            {
                ImRect bbox = NodeView::GetRect(masters, true);
                ImVec2 newPos(bbox.GetCenter() - ImVec2(bbox.GetSize().x * 0.5 + settings->ui.node.spacing + slave->getRect().GetSize().x * 0.5, 0 ));
                slave->addForceToTranslateTo(newPos + offset, _dt * settings->ui.node.speed);
            }

            break;
        }

        case Type::AlignOnBBoxTop:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() && slave->isVisible() && slave->shouldFollowOutput(master))
            {
                ImRect bbox = NodeView::GetRect(masters);
                ImVec2 newPos(bbox.GetCenter() + ImVec2(0.0, -bbox.GetHeight() * 0.5f - settings->ui.node.spacing));
                newPos.y -= settings->ui.node.spacing + slave->getSize().y / 2.0f;
                newPos.x += settings->ui.node.spacing + slave->getSize().x / 2.0f;

                if ( newPos.y < slave->getPosition().y )
                    slave->addForceToTranslateTo(newPos + offset, _dt * settings->ui.node.speed, true);
            }

            break;
        }

        case Type::MakeRowAndAlignOnBBoxTop:
        case Type::MakeRowAndAlignOnBBoxBottom:
        {
            auto inputIndex = 0;

            // Compute the cumulated width and the size y max of the input node view:
            auto cumulatedSize = 0.0f;
            auto sizeMax = 0.0f;
            for (auto eachSlave : slaves)
            {
                if (!eachSlave->isPinned() && eachSlave->isVisible())
                {
                    float sx;
                    if ( type == Type::MakeRowAndAlignOnBBoxTop )
                        sx = eachSlave->getSize().x;
                    else
                        sx = eachSlave->getRect(true).GetSize().x;

                    cumulatedSize += sx;
                    sizeMax = std::max(sizeMax, sx);
                }
            }

            float posX;

            if ( type == Type::MakeRowAndAlignOnBBoxTop)
                posX = master->getPosition().x - cumulatedSize / 2.0f;
            else
                posX = master->getRect().GetBL().x;


            // TODO: remove this "hack"
            auto masterClass = master->getOwner()->getClass();
            if (masterClass == mirror::GetClass<InstructionNode>() ||
                 ( masterClass == mirror::GetClass<ConditionalStructNode>() && type == Type::MakeRowAndAlignOnBBoxTop))
            {
                posX += cumulatedSize / 2.0f + settings->ui.node.spacing + master->getSize().x / 2.0f;
            }

            float nodeSpacing(10);

            for (auto eachSlave : slaves)
            {
                // Contrain only unpinned node that have only a single output connection
                if (!eachSlave->isPinned() && eachSlave->isVisible() )
                {
                    // Compute new position for this input view
                    ImVec2 eachSlaveNewPos = ImVec2(
                            posX + eachSlave->getSize().x / 2.0f,
                            master->getPosition().y
                    );

                    float verticalOffset = settings->ui.node.spacing + eachSlave->getSize().y / 2.0f + master->getSize().y / 2.0f;
                    if( type == MakeRowAndAlignOnBBoxTop )
                    {
                        posX += eachSlave->getSize().x + nodeSpacing;
                        verticalOffset *= -1.0f;
                    }
                    else
                    {
                        float sx = eachSlave->getRect(true).GetSize().x;
                        posX += sx + nodeSpacing;
                    }
                    eachSlaveNewPos.y += verticalOffset;

                    if ( !eachSlave->shouldFollowOutput(master) )
                    {
                        eachSlaveNewPos.y = eachSlave->getPosition().y;
                    }
                    eachSlave->addForceToTranslateTo(eachSlaveNewPos + offset, _dt * settings->ui.node.speed, true);

                }
                inputIndex++;
            }
            break;
        }

        case Type::FollowWithChildren:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() && slave->isVisible() )
            {
                // compute
                auto masterRect = NodeView::GetRect(masters,false, true);
                auto slaveRect = slave->getRect(true,true );
                ImVec2 slaveMasterOffset(masterRect.Max - slaveRect.Min);
                ImVec2 newPos(masterRect.GetCenter().x, slave->getPosition().y + slaveMasterOffset.y + settings->ui.node.spacing);

                // apply
                slave->addForceToTranslateTo(newPos + offset, _dt * settings->ui.node.speed, true);
                break;
            }
        }

        case Type::Follow:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() && slave->isVisible() )
            {
                // compute
                ImVec2 newPos(master->getPosition() + ImVec2(0.0f, master->getSize().y));
                newPos.y += settings->ui.node.spacing + slave->getSize().y;

                // apply
                slave->addForceToTranslateTo(newPos + offset, _dt * settings->ui.node.speed);
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

void ViewConstraint::addSlaves(const std::vector<NodeView *> &vector)
{
    for(auto each : vector)
        addSlave(each);
}

void ViewConstraint::addMasters(const std::vector<NodeView *> &vector)
{
    for(auto each : vector)
        addMaster(each);
}