#include <nodable/NodeView.h>

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include <numeric>                // for std::accumulate
#include <vector>

#include <nodable/Settings.h>
#include <nodable/Serializer.h>
#include <nodable/App.h>
#include <nodable/Maths.h>
#include <nodable/Scope.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/GraphNode.h>
#include <nodable/NodeConnector.h>
#include <nodable/MemberConnector.h>
#include <nodable/InvokableComponent.h>

#define NODE_VIEW_DEFAULT_SIZE ImVec2(10.0f, 35.0f)

using namespace Nodable;
using namespace Nodable::Reflect;

NodeView*          NodeView::s_selected               = nullptr;
NodeView*          NodeView::s_draggedNode            = nullptr;
NodeViewDetail     NodeView::s_viewDetail             = NodeViewDetail::Default;
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
    Node* node = get_owner();

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

void NodeView::set_owner(Node *_node)
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
    Settings* settings = Settings::Get();
    Reflect::Class* clss = _node->get_class();

    if (_node->has<InvokableComponent>())
    {
        setColor(ColorType_Fill, &settings->ui_node_invokableColor); // blue
    }
    else if ( clss->is<VariableNode>() )
    {
        setColor(ColorType_Fill, &settings->ui_node_variableColor); // purple
    }
    else if ( clss->is<LiteralNode>() )
    {
        setColor(ColorType_Fill, &settings->ui_node_literalColor);
    }
    else
    {
        setColor(ColorType_Fill, &settings->ui_node_instructionColor); // green
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

    m_nodeRelationAddedObserver = _node->m_onRelationAdded.createObserver([this](Node* otherNode, Relation_t rel ) {
        switch ( rel )
        {
            case Relation_t::IS_CHILD_OF:
                addChild(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_INPUT_OF:
                addInput(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_OUTPUT_OF:
                addOutput(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_NEXT_OF:
                LOG_ERROR("NodeView", "Can't add output for RelationType::IS_NEXT_OF.\n");
        }
    });

    m_nodeRelationRemovedObserver = _node->m_onRelationRemoved.createObserver([this](Node* otherNode, Relation_t rel ) {
        switch ( rel )
        {
            case Relation_t::IS_CHILD_OF:
                removeChild(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_INPUT_OF:
                removeInput(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_OUTPUT_OF:
                removeOutput(otherNode->get<NodeView>() );
                break;
            case Relation_t::IS_NEXT_OF:
                LOG_ERROR("NodeView", "Can't remove output for RelationType::IS_NEXT_OF.\n");
        }
    });

    Component::set_owner(_node);
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
	    for(auto eachInput : get_owner()->getInputs() )
        {
	        if ( NodeView* eachInputView = eachInput->get<NodeView>() )
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
	auto node   = get_owner();

	auto settings = Settings::Get();

	NODABLE_ASSERT(node != nullptr);

    // Draw Node connectors (in background)
    bool is_connector_hovered = false;
    {
        ImColor color = settings->ui_node_nodeConnectorColor;
        ImColor hoveredColor = settings->ui_node_nodeConnectorHoveredColor;

        auto drawConnectorAndHandleUserEvents = [&](NodeConnector *connector) {
            NodeConnector::Draw(connector, color, hoveredColor);
            is_connector_hovered |= ImGui::IsItemHovered();
        };

        std::for_each(m_prevNodeConnnectors.begin(), m_prevNodeConnnectors.end(), drawConnectorAndHandleUserEvents);
        std::for_each(m_nextNodeConnectors.begin(), m_nextNodeConnectors.end(), drawConnectorAndHandleUserEvents);
    }

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
		draw_list->AddRectFilled(itemRectMin + ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + settings->ui_node_padding), itemRectMax, ImColor(0.0f, 0.0f, 0.0f, 0.1f), m_borderRadius, 4);

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
	ImGui::InvisibleButton("node", m_size);
    ImGui::SetItemAllowOverlap();
	ImGui::SetCursorPos(cursorPositionBeforeContent + settings->ui_node_padding );
    bool is_node_hovered = ImGui::IsItemHovered();

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

	ImGui::SameLine();

	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings->ui_node_padding );
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + settings->ui_node_padding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    m_size.x = std::ceil(ImGui::GetItemRectSize().x );
    m_size.y = std::max(NODE_VIEW_DEFAULT_SIZE.y, std::ceil(ImGui::GetItemRectSize().y ));
    m_size.x = std::max( 1.0f, m_size.x); // to avoid 0 sized rectangle 
    m_size.y = std::max( 1.0f, m_size.y);

    // Draw Member in/out connectors
    {
        float radius      = settings->ui_node_memberConnectorRadius;
        ImColor color     = settings->ui_node_nodeConnectorColor;
        ImColor borderCol = settings->ui_node_borderColor;
        ImColor hoverCol  = settings->ui_node_nodeConnectorHoveredColor;

        for( auto& memberView : m_exposedInputOnlyMembers )
        {
            MemberConnector::Draw(memberView->m_in, radius, color, borderCol, hoverCol);
            is_connector_hovered |= ImGui::IsItemHovered();
        }

        for( auto& memberView : m_exposedOutOrInOutMembers )
        {
            if ( memberView->m_in)
            {
                MemberConnector::Draw(memberView->m_in, radius, color, borderCol, hoverCol);
                is_connector_hovered |= ImGui::IsItemHovered();
            }

            if ( memberView->m_out)
            {
                MemberConnector::Draw(memberView->m_out, radius, color, borderCol, hoverCol);
                is_connector_hovered |= ImGui::IsItemHovered();
            }
        }
    }

    // Contextual menu (right click)
    if ( is_node_hovered && !is_connector_hovered && ImGui::IsMouseReleased(1))
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
            App::SaveNode(node);
        }            
        ImGui::EndPopup();
    }

	// Selection by mouse (left or right click)
	if ( is_node_hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
	{
        SetSelected(this);
    }

	// Mouse dragging
	if ( GetDragged() != this)
	{
		if( GetDragged() == nullptr && ImGui::IsMouseDown(0) && is_node_hovered && ImGui::IsMouseDragPastThreshold(0))
        {
			StartDragNode(this);
        }
	}
	else if ( ImGui::IsMouseReleased(0))
	{
		StartDragNode(nullptr);				
	}		

	// Collapse on/off
	if( is_node_hovered && ImGui::IsMouseDoubleClicked(0))
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
        get_owner()->setDirty();

	hovered = is_node_hovered || is_connector_hovered;

	return edited;
}

bool NodeView::drawMemberView(MemberView* _view )
{
    bool edited = false;
    Member* member = _view->m_member;

    // show/hide
    const bool isAnInputUnconnected = member->getInput() != nullptr || !member->allowsConnection(Way_In);
    Reflect::Class* owner_class = member->getOwner()->get_class();
    const bool isVariable = owner_class->is<VariableNode>();
    const bool isLiteral  = owner_class->is<LiteralNode>();
    _view->m_showInput = _view->m_touched;
    _view->m_showInput |= member->isDefined() && (!isAnInputUnconnected || isLiteral || s_viewDetail == NodeViewDetail::Exhaustive);
    _view->m_showInput |= isVariable && member->isDefined() && member->getOwner()->as<VariableNode>()->isDeclared();

    _view->m_screenPos = ImGui::GetCursorScreenPos();

    // input
    if ( _view->m_showInput )
    {
        // try to draw an as small as possible input field
        float inputWidth = 5.0f + std::max( ImGui::CalcTextSize(((std::string)*member).c_str()).x, NodeView::s_memberInputSizeMin );
        _view->m_screenPos.x += inputWidth / 2.0f;
        ImGui::PushItemWidth(inputWidth);
        edited = NodeView::DrawMemberInput(member);
        ImGui::PopItemWidth();
    }
    else
    {
        ImGui::Button("", NodeView::s_memberInputToggleButtonSize);
        _view->m_screenPos.x += NodeView::s_memberInputToggleButtonSize.x / 2.0f;

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
            _view->m_showInput = !_view->m_showInput;
            _view->m_touched = true;
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
                "%s (%s, %s%s %s): ",
                _member->getName().c_str(),
                WayToString(_member->getConnectorWay()).c_str(),
                _member->is_connected_by(ConnectBy_Ref) ? "&" : "",
                _member->getTypeAsString().c_str(),
                _member->isDefined() ? "" : ", undefined!");

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::Text(R"(Source token: [prefix: "%s", word: "%s", suffix: "%s"])",
                        _member->getSourceToken()->m_prefix.c_str(),
                        _member->getSourceToken()->m_word.c_str(),
                        _member->getSourceToken()->m_suffix.c_str()
            );
            ImGui::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
        NodeView::DrawMemberInput(_member);

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
    const Node* node = get_owner();
    const float indent = 20.0f;

    // Components
    ImGui::Text("Components :");
    for (auto& pair : node->getComponents())
    {
        Component* component = pair.second;
        ImGui::BulletText("%s", component->get_class()->get_name() );
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

        if (node->get_parent() )
        {
            parentName = node->get_parent()->getLabel();
            parentName.append(node->get_parent()->isDirty() ? " (dirty)" : "");
        }
        ImGui::Text("Parent node is \"%s\"", parentName.c_str());

    }

    // dirty state
    ImGui::NewLine();
    bool b = get_owner()->isDirty();
    ImGui::Checkbox("Is dirty ?", &b);

    // Scope specific:

    if ( auto scope = node->get<Scope>() )
    {
        ImGui::NewLine();
        ImGui::Text("Variables:");
        auto vars = scope->get_variables();
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

void NodeView::addConstraint(NodeViewConstraint &_constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void NodeView::applyConstraints(float _dt) {
    for ( NodeViewConstraint& eachConstraint : m_constraints)
    {
        eachConstraint.apply(_dt);
    }
}

void NodeView::addForceToTranslateTo(ImVec2 desiredPos, float _factor, bool _recurse)
{
    ImVec2 delta(desiredPos - m_position);
    auto factor = std::max(0.0f, _factor);
    addForce(delta * factor, _recurse);
}

void NodeView::addForce(ImVec2 force, bool _recurse)
{
    m_forces_sum += force;

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
    float magnitude = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );

    // apply
    constexpr float magnitude_max  = 100.0f;
    const float friction   = Maths::lerp (  0.0f, 0.5f, magnitude / magnitude_max);
    const ImVec2 avg_forces_sum = (m_forces_sum + m_last_frame_forces_sum) * 0.5f;
    this->translate( avg_forces_sum * ( 1.0f - friction) * _dt , _recurse);

    m_last_frame_forces_sum = m_forces_sum;
    m_forces_sum = ImVec2();
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
    for( auto& each : get_owner()->getNext())
    {
         if ( each )
        {
             if ( auto each_view = each->get<NodeView>() )
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
