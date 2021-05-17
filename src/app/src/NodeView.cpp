#include <nodable/NodeView.h>

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include <numeric>                // for std::accumulate
#include <vector>

#include <nodable/Settings.h>
#include <nodable/Serializer.h>
#include <nodable/Application.h>
#include <nodable/Maths.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/GraphNode.h>

#define NODE_VIEW_DEFAULT_SIZE ImVec2(10.0f, 35.0f)

using namespace Nodable::app;
using namespace Nodable::core;

NodeView*          NodeView::s_selected               = nullptr;
NodeView*          NodeView::s_draggedNode            = nullptr;
NodeViewDetail     NodeView::s_viewDetail             = NodeViewDetail::Default;
const MemberConnector*   MemberConnector::s_dragged = nullptr;
const MemberConnector*   MemberConnector::s_hovered = nullptr;
const NodeConnector*     NodeConnector::s_dragged   = nullptr;
const NodeConnector*     NodeConnector::s_hovered   = nullptr;
const float        NodeView::s_memberInputSizeMin     = 10.0f;
const ImVec2       NodeView::s_memberInputToggleButtonSize   = ImVec2(10.0, 25.0f);
std::vector<NodeView*> NodeView::s_instances;

NodeView::NodeView()
        : Component()
        , View()
        , m_position(500.0f, -1.0f)
        , m_size(NODE_VIEW_DEFAULT_SIZE)
        , m_opacity(1.0f)
        , m_childrenVisible(true)
        , m_forceMemberInputVisible(false)
        , m_pinned(false)
        , m_borderRadius(5.0f)
        , m_borderColorSelected(1.0f, 1.0f, 1.0f)
{
    NodeView::s_instances.push_back(this);
}

NodeView::~NodeView()
{
    // delete MemberViews
    for ( auto& pair: m_exposedMembers ) delete pair.second;

    // deselect
    if ( s_selected == this ) s_selected = nullptr;

    // delete NodeConnectors
    for( auto& conn : m_nextNodeConnectors ) delete conn;
    for( auto& conn : m_prevNodeConnnectors ) delete conn;

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

void NodeView::exposeMember(Member* _member)
{
    MemberView* memberView = new MemberView(_member, this);

    if( _member->getConnectorWay() == Way_In )
         m_exposedInputOnlyMembers.push_back(memberView);
    else
        m_exposedOutOrInOutMembers.push_back(memberView);

    m_exposedMembers.insert({_member, memberView});
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
           exposeMember(member);
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
            exposeMember(member);
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

    // NodeConnectors
    //---------------

    // a "next" connector per next slot
    auto nextMaxCount = _node->getNextMaxCount();
    for(size_t index = 0; index <  nextMaxCount; ++index )
    {
        m_nextNodeConnectors.push_back(new NodeConnector(this, Way_Out, index, nextMaxCount));
    }

    // a single "previous" connector if node can be connected in this way
    if( _node->getPrevMaxCount() != 0)
        m_prevNodeConnnectors.push_back(new NodeConnector(this, Way_In));

    m_nodeRelationAddedObserver = _node->m_onRelationAdded.createObserver([this](Node* otherNode, RelationType rel ) {
        LOG_MESSAGE("NodeView", "Event received");
        switch ( rel )
        {
            case RelationType::IS_CHILD_OF:
                addChild( otherNode->getComponent<NodeView>() );
                break;
            case RelationType::IS_INPUT_OF:
                addInput( otherNode->getComponent<NodeView>() );
                break;
        }
    });

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
	if(MemberConnector::GetDragged() == nullptr) // Prevent dragging node while dragging connector
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

const MemberView* NodeView::getMemberView(const Member* _member)const
{
    if ( m_exposedMembers.find(_member) != m_exposedMembers.end())
        return m_exposedMembers.at(_member);
    return nullptr;
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
    Maths::lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
    this->applyForces(_deltaTime, false);
	return true;
}

bool NodeView::draw()
{
	bool edited = false;
	auto node   = getOwner();

	auto settings = Settings::GetCurrent();

	NODABLE_ASSERT(node != nullptr);

    // Draw Node connectors (in background)
    ImColor color        = settings->ui.node.nodeConnectorColor;
    ImColor hoveredColor = settings->ui.node.nodeConnectorHoveredColor;

    auto drawConnectorAndHandleUserEvents = [color, hoveredColor](NodeConnector* connector)
    {
        NodeConnector::Draw(connector, color, hoveredColor);

    };
    std::for_each(m_prevNodeConnnectors.begin(), m_prevNodeConnnectors.end(), drawConnectorAndHandleUserEvents);
    std::for_each(m_nextNodeConnectors.begin(), m_nextNodeConnectors.end(), drawConnectorAndHandleUserEvents);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
	const auto halfSize = m_size / 2.0;
	ImGui::SetCursorPos(getPosRounded() - halfSize );
	ImGui::PushID(this);
	ImVec2 cursorPositionBeforeContent = ImGui::GetCursorPos();
	ImVec2 screenPosition  = ImGuiEx::CursorPosToScreenPos(getPosRounded() );

	// Draw the background of the Group
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	{			
		auto borderCol = IsSelected(this) ? m_borderColorSelected : getColor(ColorType_Border);

		auto itemRectMin = screenPosition - halfSize;
		auto itemRectMax = screenPosition + halfSize;

		// Draw the rectangle under everything
		ImGuiEx::DrawRectShadow(itemRectMin, itemRectMax, m_borderRadius, 4, ImVec2(1.0f), getColor(ColorType_Shadow));
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
	ImGuiEx::ShadowedText(ImVec2(1.0f), getColor(ColorType_BorderHighlights), getLabel().c_str()); // text with a lighter shadow (incrust effect)

	// Draw inputs
    for( auto& memberView : m_exposedInputOnlyMembers )
    {
        ImGui::SameLine();
        ImGui::SetCursorPosY(cursorPositionBeforeContent.y + 1.0f);
        edited |= drawMemberView(memberView);
    }

    // Draw outputs
    for( auto& memberView : m_exposedOutOrInOutMembers )
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

    // Draw Member in/out connectors
    {
        float radius    = settings->ui.node.memberConnectorRadius;
        float borderCol = getColor(ColorType_Border);
        float hoverCol  = getColor(ColorType_BorderHighlights);

        for( auto& memberView : m_exposedInputOnlyMembers )
        {
            MemberConnector::Draw(memberView->m_in, radius, color, borderCol, hoverCol);
        }

        for( auto& memberView : m_exposedOutOrInOutMembers )
        {
            if ( memberView->m_in)
                MemberConnector::Draw(memberView->m_in, radius, color, borderCol, hoverCol);
            if ( memberView->m_out)
                MemberConnector::Draw(memberView->m_out, radius, color, borderCol, hoverCol);
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
            eachMemberView->m_touched = m_forceMemberInputVisible;
            eachMemberView->m_showInput = m_forceMemberInputVisible;
        }
	}

	ImGui::PopStyleVar();
	ImGui::PopID();

	if( edited )
	    getOwner()->setDirty();

	if( hovered && !ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

	return edited;
}

bool NodeView::drawMemberView(MemberView* _memberView )
{
    bool edited = false;
    Member* member = _memberView->m_member;

    if( !_memberView->m_touched )
    {
        const bool isAnInputUnconnected = member->getInputMember() != nullptr || !member->allowsConnection(Way_In);
        const bool isVariable = member->getOwner()->getClass() == VariableNode::GetClass();
        const bool isLiteral  = member->getOwner()->getClass() == LiteralNode::GetClass();
        _memberView->m_showInput = _memberView->m_member->isDefined() && (!isAnInputUnconnected || isLiteral || isVariable || s_viewDetail == NodeViewDetail::Exhaustive) ;
    }

    _memberView->m_screenPos = ImGui::GetCursorScreenPos();

    // input
    if ( _memberView->m_showInput )
    {
        // try to draw an as small as possible input field
        float inputWidth = 5.0f + std::max( ImGui::CalcTextSize(((std::string)*member).c_str()).x, NodeView::s_memberInputSizeMin );
        _memberView->m_screenPos.x += inputWidth / 2.0f;
        ImGui::PushItemWidth(inputWidth);
        edited = NodeView::DrawMemberInput(member);
        ImGui::PopItemWidth();
    }
    else
    {
        ImGui::Button("", NodeView::s_memberInputToggleButtonSize);
        _memberView->m_screenPos.x += NodeView::s_memberInputToggleButtonSize.x / 2.0f;

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
            _memberView->m_showInput = !_memberView->m_showInput;
            _memberView->m_touched = true;
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

bool NodeView::IsInsideRect(NodeView* _nodeView, ImRect _rect) {
	auto nodeRect = _nodeView->getRect();
	return _rect.Contains(nodeRect);
}

void NodeView::DrawNodeViewAsPropertiesPanel(NodeView* _view)
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
    for (auto& eachView : _view->m_exposedInputOnlyMembers )
    {
        drawMember(eachView->m_member);
    }
    ImGui::Unindent();

    // Draw exposed output members
    ImGui::NewLine();
    ImGui::Text("Outputs:");
    ImGui::Indent();
    for (auto& eachView : _view->m_exposedOutOrInOutMembers )
    {
        drawMember(eachView->m_member);
    }
    ImGui::Unindent();

    // Advanced properties
    ImGui::NewLine();
   _view->drawAdvancedProperties();
}

void NodeView::ConstraintToRect(NodeView* _view, ImRect _rect)
{
	
	if ( !NodeView::IsInsideRect(_view, _rect)) {

		_rect.Expand(ImVec2(-2, -2)); // shrink

		auto nodeRect = _view->getRect();

		auto newPos = _view->getPosRounded();

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
        if (eachView && eachView->isVisible() && !(eachView->m_pinned && _ignorePinned) && eachView->shouldFollowOutput(this) )
        {
            ImRect childRect = eachView->getRect(true, _ignorePinned, _ignoreMultiConstrained);
            enlarge_to_fit(childRect);
        }
    };

    std::for_each(m_children.begin(), m_children.end(), enlarge_to_fit_all);
    std::for_each(m_inputs.begin(), m_inputs.end(), enlarge_to_fit_all);

//    auto draw_list = ImGui::GetForegroundDrawList();
//    auto screen_rect = rect;
//    screen_rect.Translate( View::ToScreenPosOffset() );
//    if ( NodeView::IsSelected(this) )
//        draw_list->AddRect(screen_rect.Min, screen_rect.Max, ImColor(0,255,0));

    return rect;
}

void NodeView::clearConstraints() {
    m_constraints.clear();
}

void NodeView::addConstraint(ViewConstraint &_constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void NodeView::applyConstraints(float _dt) {
    for ( ViewConstraint& eachConstraint : m_constraints)
    {
        eachConstraint.apply(_dt);
    }
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

    auto is_visible = [](NodeView* view) { return view->isVisible(); };
    if ( std::find_if(masters.begin(), masters.end(), is_visible) == masters.end()) return;
    if ( std::find_if(slaves.begin(), slaves.end(), is_visible) == slaves.end()) return;

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
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed);
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

                if ( newPos.y < slave->getPos().y )
                    slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed, true);
            }

            break;
        }

        case Type::MakeRowAndAlignOnBBoxTop:
        case Type::MakeRowAndAlignOnBBoxBottom:
        {
            // Compute size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = type == Type::MakeRowAndAlignOnBBoxBottom;
            for (auto eachSlave : slaves)
            {
                bool ignore = eachSlave->isPinned() || !eachSlave->isVisible();
                size_x.push_back( ignore ? 0.f : eachSlave->getRect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            float start_pos_x = master->getPos().x - size_x_total / 2.0f;
            auto masterClass = master->getOwner()->getClass();
            if (masterClass == mirror::GetClass<InstructionNode>() ||
                 ( masterClass == mirror::GetClass<ConditionalStructNode>() && type == Type::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = master->getPos().x + master->getSize().x / 2.0f + settings->ui.node.spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto eachSlave : slaves)
            {
                if (!eachSlave->isPinned() && eachSlave->isVisible() )
                {
                    // Compute new position for this input view
                    float verticalOffset = settings->ui.node.spacing + eachSlave->getSize().y / 2.0f + master->getSize().y / 2.0f;
                    if( type == MakeRowAndAlignOnBBoxTop )
                    {
                        verticalOffset *= -1.0f;
                    }

                    ImVec2 new_pos = ImVec2(start_pos_x + size_x[node_index] / 2.0f, master->getPos().y + verticalOffset);

                    if ( !eachSlave->shouldFollowOutput(master) )
                        new_pos.y = eachSlave->getPos().y; // remove constraint on Y axis

                    eachSlave->addForceToTranslateTo(new_pos + m_offset, _dt * settings->ui.node.speed, true);

                    start_pos_x += size_x[node_index] + settings->ui.node.spacing;
                    node_index++;
                }
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
                ImVec2 newPos(masterRect.GetCenter().x,slave->getPos().y + slaveMasterOffset.y + settings->ui.node.spacing);

                // apply
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed, true);
                break;
            }
        }

        case Type::Follow:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() && slave->isVisible() )
            {
                // compute
                ImVec2 newPos(master->getPos() + ImVec2(0.0f, master->getSize().y));
                newPos.y += settings->ui.node.spacing + slave->getSize().y;

                // apply
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed);
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

MemberView::MemberView(Member* _member, NodeView* _nodeView)
    : m_member(_member)
    , m_showInput(false)
    , m_touched(false)
    , m_in(nullptr)
    , m_out(nullptr)
    , m_nodeView(_nodeView)
{
    NODABLE_ASSERT(_member != nullptr); // Member must be defined
    if ( m_member->allowsConnection(Way_In) )   m_in  = new MemberConnector(this, Way_In);
    if ( m_member->allowsConnection(Way_Out) )  m_out = new MemberConnector(this, Way_Out);
}

MemberView::~MemberView()
{
    delete m_in;
    delete m_out;
}

ImVec2 MemberConnector::getPos()const
{
    ImVec2 pos                  = m_memberView->m_screenPos;
    auto nodeViewScreenPosition = ImGuiEx::CursorPosToScreenPos(m_memberView->m_nodeView->getPos());
    auto nodeSemiHeight         = m_memberView->m_nodeView->getSize().y * 0.5f;
    if (m_way == Way_In) nodeSemiHeight = -nodeSemiHeight;

    return ImVec2(pos.x, nodeViewScreenPosition.y + nodeSemiHeight);
}

bool MemberConnector::hasSameParentWith(const MemberConnector* other) const
{
    return getMember() == other->getMember();
}

bool MemberConnector::connect(const MemberConnector *other) const
{
    auto graph = getMember()->getOwner()->getParentGraph();
    // TODO: handle incompatibility
    graph->connect(getMember(), other->getMember());
    return true;
}

void MemberConnector::DropBehavior(bool &needsANewNode)
{
    if (s_dragged && ImGui::IsMouseReleased(0))
    {
        if ( s_hovered )
        {
            MemberConnector::Connect(s_dragged, s_hovered);
            s_dragged = s_hovered = nullptr;
        } else {
            needsANewNode = true;
        }
    }
}

void MemberConnector::Draw(
        const MemberConnector *_connector,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor)
{
    // draw
    //-----

    auto draw_list = ImGui::GetWindowDrawList();
    auto connnectorScreenPos = _connector->getPos();

    // Unvisible Button on top of the Circle
    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos(connnectorScreenPos - ImVec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_connector->m_memberView);
    bool clicked = ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);
    auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    auto color = isItemHovered ? _hoverColor : _color;
    draw_list->AddCircleFilled(connnectorScreenPos, _radius, _color);
    draw_list->AddCircle(connnectorScreenPos, _radius, _borderColor);

    // behavior
    //--------

    if (isItemHovered)
    {
        if (ImGui::IsMouseDown(0))
        {
            if ( s_dragged == nullptr && !NodeView::IsAnyDragged())
                MemberConnector::StartDrag(_connector);
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        s_hovered = _connector;
        ImGui::BeginTooltip();
        ImGui::Text("%s", _connector->getMember()->getName().c_str() );
        ImGui::EndTooltip();
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }
}

bool MemberConnector::Connect(const MemberConnector *_left, const MemberConnector *_right)
{
    if ( _left->hasSameParentWith(_right) )
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors from the same Member.\n" );
        return false;
    }

    if (_left->m_way == _right->m_way)
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors with the same nature (in and in, out and out)\n" );
        return false;
    }

    if ( s_dragged->m_way == Way_Out )
        return s_dragged->connect(s_hovered);
    return s_hovered->connect(s_dragged);
}

bool NodeConnector::Draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor)
{
    // draw
    float rounding = 6.0f;

    auto draw_list = ImGui::GetWindowDrawList();
    auto rect      = _connector->getRect();
    rect.Translate(ImGuiEx::ToScreenPosOffset());

    ImDrawCornerFlags cornerFlags = _connector->m_way == Way_Out ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    auto cursorScreenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(rect.GetTL());
    ImGui::PushID(_connector);
    bool clicked = ImGui::InvisibleButton("###", rect.GetSize());
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);

    ImColor color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hoveredColor : _color;
    draw_list->AddRectFilled(rect.Min, rect.Max, color, rounding, cornerFlags );
    draw_list->AddRect(rect.Min, rect.Max, ImColor(50,50, 50), rounding, cornerFlags );

    // behavior
    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        if (ImGui::IsMouseDown(0) && !IsDragging() && !NodeView::IsAnyDragged())
        {
            if ( _connector->m_way == Way_Out)
            {
                if ( _connector->getNode()->getNext().size() < _connector->getNode()->getNextMaxCount() )
                    StartDrag(_connector);
            }
            else
            {
                if ( _connector->getNode()->getPrev().size() < _connector->getNode()->getPrevMaxCount() )
                    StartDrag(_connector);
            }
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        s_hovered = _connector;
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }

    return clicked;
}

ImRect NodeConnector::getRect() const
{
    ImVec2 leftCornerPos = m_way == Way_In ? m_nodeView->getRect().GetTL() : m_nodeView->getRect().GetBL();

    ImVec2 size(Settings::GetCurrent()->ui.codeFlow.lineWidthMax, Settings::GetCurrent()->ui.node.nodeConnectorHeight);
    ImRect rect(leftCornerPos, leftCornerPos + size);
    rect.Translate(ImVec2(size.x * float(m_index), -rect.GetSize().y * 0.5f) );
    rect.Expand(ImVec2(-Settings::GetCurrent()->ui.node.nodeConnectorPadding, 0.0f));
    return rect;
}

ImVec2 NodeConnector::getPos()const
{
    return getRect().GetCenter() + ImGuiEx::ToScreenPosOffset();
}

bool NodeConnector::connect(const NodeConnector* other) const
{
    auto graph = getNode()->getParentGraph();
    // TODO: handle incompatibility
    graph->connect(other->getNode(), getNode() , RelationType::IS_NEXT_OF );

    return true;
}

void NodeConnector::DropBehavior(bool &needsANewNode)
{
    if (s_dragged && ImGui::IsMouseReleased(0))
    {
        if ( s_hovered )
        {
            NodeConnector::Connect(s_dragged, s_hovered);
            s_dragged = s_hovered = nullptr;
        } else {
            needsANewNode = true;
        }
    }
}

bool NodeConnector::hasSameParentWith(const NodeConnector *other) const
{
    return getNode() == other->getNode();
}

bool NodeConnector::Connect(const NodeConnector *_left, const NodeConnector *_right)
{
    if ( _left->hasSameParentWith(_right) )
    {
        LOG_WARNING("NodeConnector", "Unable to connect these two Connectors from the same Node.\n");
        return false;
    }

    if( _left->m_way == _right->m_way )
    {
        LOG_WARNING("NodeConnector", "Unable to connect these two Node Connectors (must have different ways).\n");
        return false;
    }

    if ( _left->m_way == Way_Out )
        return _left->connect(_right);
    return _right->connect(_left);
}
