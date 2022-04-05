#include <nodable/app/NodeView.h>

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include <numeric>                // for std::accumulate
#include <vector>

#include <nodable/app/Settings.h>
#include <nodable/core/Serializer.h>
#include <nodable/app/App.h>
#include <nodable/core/Maths.h>
#include <nodable/core/Scope.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/GraphNode.h>
#include <nodable/app/NodeConnector.h>
#include <nodable/app/MemberConnector.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/app/AppContext.h>

#define NODE_VIEW_DEFAULT_SIZE vec2(10.0f, 35.0f)

using namespace Nodable;
using namespace Nodable::R;

NodeView*          NodeView::s_selected                        = nullptr;
NodeView*          NodeView::s_dragged                         = nullptr;
NodeViewDetail     NodeView::s_view_detail                     = NodeViewDetail::Default;
const float        NodeView::s_member_input_size_min           = 10.0f;
const vec2         NodeView::s_member_input_toggle_button_size = vec2(10.0, 25.0f);
NodeViewVec        NodeView::s_instances;

NodeView::NodeView(AppContext* _ctx)
        : Component()
        , View(_ctx)
        , m_context(_ctx)
        , m_position(500.0f, -1.0f)
        , m_size(NODE_VIEW_DEFAULT_SIZE)
        , m_opacity(1.0f)
        , m_expanded(true)
        , m_force_member_inputs_visible(false)
        , m_pinned(false)
        , m_border_radius(5.0f)
        , m_border_color_selected(1.0f, 1.0f, 1.0f)
        , m_exposed_this_member_view(nullptr)
        , m_children_slots(this)
        , m_input_slots(this)
        , m_output_slots(this)
        , m_successor_slots(this)
        , m_edition_enable(true)
{
    NodeView::s_instances.push_back(this);
}

NodeView::~NodeView()
{
    // delete MemberViews
    for ( auto& pair: m_exposed_members ) delete pair.second;

    // deselect
    if ( s_selected == this ) s_selected = nullptr;

    // delete NodeConnectors
    for( auto& conn : m_successors_node_connectors ) delete conn;
    for( auto& conn : m_predecessors_node_connnectors ) delete conn;

    // Erase instance in static vector
    auto found = std::find( s_instances.begin(), s_instances.end(), this);
    assert(found != s_instances.end() );
    s_instances.erase(found);
}

std::string NodeView::get_label()
{
    Node* node = get_owner();

    if (s_view_detail == NodeViewDetail::Minimalist )
    {
        // I always add an ICON_FA at the begining of any node label string (encoded in 4 bytes)
        return std::string(node->get_short_label());
    }
    return node->get_label();
}

void NodeView::expose(Member* _member)
{
    auto member_view = new MemberView(m_context, _member, this);

    if ( _member == get_owner()->get_this_member() )
    {
        member_view->m_out->m_display_side = MemberConnector::Side::Left; // force to be displayed on the left
        m_exposed_this_member_view = member_view;
    }
    else
    {
        if (_member->get_allowed_connection() == Way_In)
        {
            m_exposed_input_only_members.push_back(member_view);
        }
        else
        {
            m_exposed_out_or_inout_members.push_back(member_view);
        }
    }

    m_exposed_members.insert({_member, member_view});
}

void NodeView::set_owner(Node *_node)
{
    Component::set_owner(_node);

    Settings*            settings = m_context->settings;
    std::vector<Member*> not_exposed;

    //  We expose first the members which allows input connections
    for(Member* each_member : _node->props()->by_id())
    {
        if (each_member->get_visibility() == Visibility::Always && each_member->allows_connection(Way_In) )
        {
            expose(each_member);
        }
        else
        {
            not_exposed.push_back(each_member);
        }
    }

    // Then we expose node which allows output connection (if they are not yet exposed)
    for (auto& member : not_exposed)
    {
        if (member->get_visibility() == Visibility::Always && member->allows_connection(Way_Out))
        {
            expose(member);
        }
    }

    if ( auto this_member = _node->get_this_member() )
    {
        expose(this_member);
    }

    // Determine a color depending on node type
    R::Class_ptr clss = _node->get_class();

    if (_node->has<InvokableComponent>())
    {
        setColor(Color_Fill, &settings->ui_node_invokableColor); // blue
    }
    else if (clss->is_child_of<VariableNode>() )
    {
        setColor(Color_Fill, &settings->ui_node_variableColor); // purple
    }
    else if (clss->is_child_of<LiteralNode>() )
    {
        setColor(Color_Fill, &settings->ui_node_literalColor);
    }
    else
    {
        setColor(Color_Fill, &settings->ui_node_instructionColor); // green
    }

    // NodeConnectors
    //---------------

    // add q successor connector per successor slot
    const size_t successor_max_count = _node->successor_slots().get_limit();
    for(size_t index = 0; index < successor_max_count; ++index )
    {
        m_successors_node_connectors.push_back(new NodeConnector(m_context, this, Way_Out, index, successor_max_count));
    }

    // add a single predecessor connector if node can be connected in this way
    if(_node->predecessor_slots().get_limit() != 0)
        m_predecessors_node_connnectors.push_back(new NodeConnector(m_context, this, Way_In));

    m_nodeRelationAddedObserver = _node->m_on_relation_added.createObserver(
        [this](Node* _other_node, EdgeType _relation )
        {
            NodeView* _other_node_view = _other_node->get<NodeView>();
            switch ( _relation )
            {
                case EdgeType::IS_CHILD_OF: children_slots().add(_other_node_view ); break;
                case EdgeType::IS_INPUT_OF: input_slots().add(_other_node_view ); break;
                case EdgeType::IS_OUTPUT_OF: output_slots().add(_other_node_view ); break;
                case EdgeType::IS_SUCCESSOR_OF: successor_slots().add(_other_node_view ); break;
                case EdgeType::IS_PREDECESSOR_OF: NODABLE_ASSERT(false); /* NOT HANDLED */break;
            }
        });

    m_nodeRelationRemovedObserver = _node->m_on_relation_removed.createObserver(
    [this](Node* _other_node, EdgeType _relation )
        {
            NodeView* _other_node_view = _other_node->get<NodeView>();
            switch ( _relation )
            {
                case EdgeType::IS_CHILD_OF: children_slots().remove(_other_node_view ); break;
                case EdgeType::IS_INPUT_OF: input_slots().remove(_other_node_view ); break;
                case EdgeType::IS_OUTPUT_OF: output_slots().remove(_other_node_view ); break;
                case EdgeType::IS_SUCCESSOR_OF: successor_slots().remove(_other_node_view ); break;
                case EdgeType::IS_PREDECESSOR_OF: NODABLE_ASSERT(false); /* NOT HANDLED */break;
            }
        });
}

void NodeView::set_selected(NodeView* _view)
{
	s_selected = _view;
}

NodeView* NodeView::get_selected()
{
	return s_selected;
}

void NodeView::start_drag(NodeView* _view)
{
	if(MemberConnector::get_gragged() == nullptr) // Prevent dragging node while dragging connector
		s_dragged = _view;
}

bool NodeView::is_any_dragged()
{
	return get_dragged() != nullptr;
}

NodeView* NodeView::get_dragged()
{
	return s_dragged;
}

bool NodeView::is_selected(NodeView* _view)
{
	return s_selected == _view;
}

const MemberView* NodeView::get_member_view(const Member* _member)const
{
    return m_exposed_members.at(_member);
}

void NodeView::set_position(vec2 _position)
{
	m_position = _position;
}

void NodeView::translate(vec2 _delta, bool _recurse)
{
    this->set_position(m_position + _delta);

	if ( _recurse )
    {
	    for(auto eachInput : get_owner()->input_slots() )
        {
	        if ( NodeView* eachInputView = eachInput->get<NodeView>() )
	        {
	            if (!eachInputView->m_pinned && eachInputView->should_follow_output(this) )
                    eachInputView->translate(_delta, true);
	        }
        }
    }
}

void NodeView::arrange_recursively(bool _smoothly)
{
    std::vector<NodeView*> views;

    for (auto inputView : m_input_slots)
    {
        if (inputView->should_follow_output(this))
        {
            views.push_back(inputView);
            inputView->arrange_recursively();
        }
    }

    for (auto eachChild : m_children_slots)
    {
        views.push_back(eachChild);
        eachChild->arrange_recursively();
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
    this->apply_forces(_deltaTime, false);
	return true;
}

bool NodeView::draw()
{
	bool edited = false;
	auto node   = get_owner();
	Settings* settings = m_context->settings;

	NODABLE_ASSERT(node != nullptr);

    // Draw Node connectors (in background)
    bool is_connector_hovered = false;
    {
        ImColor color = settings->ui_node_nodeConnectorColor;
        ImColor hoveredColor = settings->ui_node_nodeConnectorHoveredColor;

        auto drawConnectorAndHandleUserEvents = [&](NodeConnector *connector) {
            edited |= NodeConnector::draw(connector, color, hoveredColor, m_edition_enable);
            is_connector_hovered |= ImGui::IsItemHovered();
        };

        if ( m_expanded )
        {
            std::for_each(m_predecessors_node_connnectors.begin(), m_predecessors_node_connnectors.end(), drawConnectorAndHandleUserEvents);
            std::for_each(m_successors_node_connectors.begin(), m_successors_node_connectors.end(), drawConnectorAndHandleUserEvents);
        }
        else
        {
            std::for_each(m_predecessors_node_connnectors.begin(), m_predecessors_node_connnectors.begin()+1, drawConnectorAndHandleUserEvents);
            std::for_each(m_successors_node_connectors.begin(), m_successors_node_connectors.begin()+1, drawConnectorAndHandleUserEvents);
        }
    }

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
	const auto halfSize = m_size / 2.0;
	ImGui::SetCursorPos(get_position_rounded() - halfSize );
	ImGui::PushID(this);
	vec2 cursor_pos_content_start = ImGui::GetCursorPos();
	vec2 screen_cursor_pos_content_start = ImGuiEx::CursorPosToScreenPos(get_position_rounded() );

	// Draw the background of the Group
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	{			
		auto borderCol = is_selected(this) ? m_border_color_selected : getColor(Color_Border);

		auto itemRectMin = screen_cursor_pos_content_start - halfSize;
		auto itemRectMax = screen_cursor_pos_content_start + halfSize;

		// Draw the rectangle under everything
		ImGuiEx::DrawRectShadow(itemRectMin, itemRectMax, m_border_radius, 4, vec2(1.0f), getColor(Color_Shadow));
		draw_list->AddRectFilled(itemRectMin, itemRectMax, getColor(Color_Fill), m_border_radius);
		draw_list->AddRect(itemRectMin + vec2(1.0f), itemRectMax, getColor(Color_BorderHighlights), m_border_radius);
		draw_list->AddRect(itemRectMin, itemRectMax, borderCol, m_border_radius);

		// darken the background under the content
		draw_list->AddRectFilled(itemRectMin + vec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + settings->ui_node_padding), itemRectMax, ImColor(0.0f, 0.0f, 0.0f, 0.1f), m_border_radius, 4);

		// Draw an additionnal blinking rectangle when selected
		if (is_selected(this))
		{
			auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
			float offset = 4.0f;
			draw_list->AddRect(itemRectMin - vec2(offset), itemRectMax + vec2(offset), ImColor(1.0f, 1.0f, 1.0f, float(alpha) ), m_border_radius + offset, ~0, offset / 2.0f);
		}
	}

	// Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorPos(cursor_pos_content_start);
	ImGui::InvisibleButton("node", m_size);
    ImGui::SetItemAllowOverlap();
	ImGui::SetCursorPos(cursor_pos_content_start);
	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings->ui_node_padding * 2.0f); // x2 padding to keep space for "this" connector
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + settings->ui_node_padding );
    bool is_node_hovered = ImGui::IsItemHovered();

	// Draw the window content
	//------------------------

    ImGui::BeginGroup();
        std::string label = get_label().empty() ? " " : get_label();                        // ensure a 1 char width, to be able to grab it
        if ( !m_expanded )
        {
            // symbolize the fact node view is not expanded
            //abel.insert(0, "<<");
            label.append(" " ICON_FA_OBJECT_GROUP);
        }
        ImGuiEx::ShadowedText(vec2(1.0f), getColor(Color_BorderHighlights), label.c_str()); // text with a lighter shadow (incrust effect)

        ImGui::SameLine();

        ImGui::BeginGroup();
        // Draw inputs
        for (auto &memberView : m_exposed_input_only_members)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(cursor_pos_content_start.y + 1.0f);
            edited |= draw(memberView);
        }

        // Draw outputs
        for (auto &memberView : m_exposed_out_or_inout_members)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(cursor_pos_content_start.y + 8.0f);
            edited |= draw(memberView);
        }

        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings->ui_node_padding * 2.0f);
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

        if ( m_exposed_this_member_view )
        {
            MemberConnector::draw(m_exposed_this_member_view->m_out, radius, color, borderCol, hoverCol, m_edition_enable);
            is_connector_hovered |= ImGui::IsItemHovered();
        }

        for( auto& memberView : m_exposed_input_only_members )
        {
            MemberConnector::draw(memberView->m_in, radius, color, borderCol, hoverCol, m_edition_enable);
            is_connector_hovered |= ImGui::IsItemHovered();
        }

        for( auto& memberView : m_exposed_out_or_inout_members )
        {
            if ( memberView->m_in)
            {
                MemberConnector::draw(memberView->m_in, radius, color, borderCol, hoverCol, m_edition_enable);
                is_connector_hovered |= ImGui::IsItemHovered();
            }

            if ( memberView->m_out)
            {
                MemberConnector::draw(memberView->m_out, radius, color, borderCol, hoverCol, m_edition_enable);
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
            this->arrange_recursively();
        }

        ImGui::MenuItem("Pinned", "", &m_pinned, true);

		if ( ImGui::MenuItem("Expanded", "", &m_expanded, true) )
        {
		    set_expanded(m_expanded);
        }

        ImGui::Separator();

        if( ImGui::Selectable("Delete", !m_edition_enable ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
        {
            node->flag_for_deletion();
        }

        ImGui::EndPopup();
    }

	// Selection by mouse (left or right click)
	if ( is_node_hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
	{
        set_selected(this);
    }

	// Mouse dragging
	if (get_dragged() != this)
	{
		if(get_dragged() == nullptr && ImGui::IsMouseDown(0) && is_node_hovered && ImGui::IsMouseDragPastThreshold(0))
        {
            start_drag(this);
        }
	}
	else if ( ImGui::IsMouseReleased(0))
	{
        start_drag(nullptr);
	}		

	// Collapse on/off
	if( is_node_hovered && ImGui::IsMouseDoubleClicked(0))
	{
        expand_toggle();
	}

	ImGui::PopStyleVar();
	ImGui::PopID();

	if( edited )
        get_owner()->set_dirty();

	hovered = is_node_hovered || is_connector_hovered;

	return edited;
}

bool NodeView::draw(MemberView* _view )
{
    bool edited = false;
    Member* member = _view->m_member;

    const R::Class_ptr owner_class = member->get_owner()->get_class();

    /*
     * Handle input visibility
     */
    bool show;

    if ( _view->m_touched )  // in case user touched it, we keep the current state
    {
        show = _view->m_showInput;
    }
    else if( s_view_detail == NodeViewDetail::Exhaustive )
    {
        show = true;
    }
    else if( member->get_owner()->is<LiteralNode>() )
    {
        show = true;                               // we always show literal´s
    }
    else if( member->get_owner()->is<VariableNode>() )
    {
        show = member->get_data()->is_defined();
    }
    else if( !member->has_input_connected() )
    {
        show = member->get_data()->is_defined();   // we always show a defined unconnected member
    }
    else if ( member->get_meta_type()->has_qualifier(R::Qualifier::Pointer) )
    {
        show = member->is_connected_to_variable();
    }
    else if ( member->is_connected_to_variable() )
    {
        show = true;
    }
    else
    {
        show = member->get_data()->is_defined();
    }

    _view->m_showInput = show;

    vec2 new_relative_pos = ImGui::GetCursorScreenPos() - get_screen_position();

    // input
    float input_size = NodeView::s_member_input_toggle_button_size.x;

    if ( _view->m_showInput )
    {
        bool limit_size = member->get_meta_type()->get_type() != R::Type::bool_t;

        if ( limit_size )
        {
            // try to draw an as small as possible input field
            std::string str;
            if (member->is_connected_to_variable())
            {
                str = member->get_connected_variable()->get_name();
            }
            else
            {
                str = member->convert_to<std::string>();
            }
            input_size = 5.0f + std::max(ImGui::CalcTextSize(str.c_str()).x, NodeView::s_member_input_size_min);
            ImGui::PushItemWidth(input_size);
        }
        edited = NodeView::draw_input(member);

        if ( limit_size )
        {
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Button("", NodeView::s_member_input_toggle_button_size);

        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s (%s)",
                        member->get_name().c_str(),
                        member->get_meta_type()->get_fullname().c_str());
            ImGui::EndTooltip();
        }

        if ( ImGui::IsItemClicked(0) )
        {
            _view->m_showInput = !_view->m_showInput;
            _view->m_touched = true;
        }
    }

    new_relative_pos.x += input_size * 0.5f; // center over input
    _view->relative_pos(new_relative_pos);

    return edited;
}

bool NodeView::draw_input(Member *_member, const char* _label )
{
    bool edited = false;

    Node* node  = _member->get_owner();

    // Create a label (everything after ## will not be displayed)
    std::string label;
    if ( _label != nullptr )
    {
        label.append(_label);
    }
    else
    {
        label.append("##" + _member->get_name());
    }

    auto inputFlags = ImGuiInputTextFlags_None;

    if( _member->has_input_connected() && _member->is_connected_to_variable() ) // if is a ref to a variable, we just draw variable name
    {
        char str[255];
        auto* variable = _member->get_input()->get_owner()->as<VariableNode>();
        snprintf(str, 255, "%s", variable->get_name() );

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)variable->get<NodeView>()->getColor(Color_Fill) );
        ImGui::InputText(label.c_str(), str, 255, inputFlags);
        ImGui::PopStyleColor();

    }
    else
    {
        /* Draw the member */
        switch (_member->get_meta_type()->get_type() )
        {
            case R::Type::i16_t:
            {
                auto i16 = (i16_t)*_member;

                if (ImGui::InputInt(label.c_str(), &i16, 0, 0, inputFlags ) && !_member->has_input_connected())
                {
                    _member->set(i16);
                    edited |= true;
                }
                break;
            }

            case R::Type::double_t:
            {
                auto d = (double)*_member;

                if (ImGui::InputDouble(label.c_str(), &d, 0.0F, 0.0F, "%g", inputFlags ) && !_member->has_input_connected())
                {
                    _member->set(d);
                    edited |= true;
                }
                break;
            }

            case R::Type::string_t:
            {
                char str[255];
                snprintf(str, 255, "%s", ((std::string)*_member).c_str() );

                if ( ImGui::InputText(label.c_str(), str, 255, inputFlags) && !_member->has_input_connected() )
                {
                    _member->set(str);
                    edited |= true;
                }
                break;
            }

            case R::Type::bool_t:
            {
                std::string checkBoxLabel = _member->get_name();

                auto b = (bool)*_member;

                if (ImGui::Checkbox(label.c_str(), &b ) && !_member->has_input_connected() )
                {
                    _member->set(b);
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
            Serializer* serializer = node->get_parent_graph()->get_language()->get_serializer();
            std::string buffer;
            serializer->serialize(buffer, _member);
            ImGui::Text("%s", buffer.c_str() );
            ImGui::EndTooltip();
        }
    }

    return edited;
}

bool NodeView::is_inside(NodeView* _nodeView, ImRect _rect) {
	auto nodeRect = _nodeView->get_rect();
	return _rect.Contains(nodeRect);
}

void NodeView::draw_as_properties_panel(NodeView* _view, bool* _show_advanced)
{
    const float labelColumnWidth = ImGui::GetContentRegionAvailWidth() / 2.0f;

    auto drawMember = [&](Member* _member)
    {
        // label (<name> (<way> <type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s, %s%s %s): ",
                _member->get_name().c_str(),
                WayToString(_member->get_allowed_connection()).c_str(),
                _member->get_meta_type()->get_fullname().c_str(),
                _member->is_connected_by(ConnectBy_Ref) ? "&" : "",
                _member->get_data()->is_defined() ? "" : ", undefined!");

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGui::IsItemHovered() )
        {
            ImGui::BeginTooltip();
            std::shared_ptr<Token> token = _member->get_src_token();
            ImGui::Text("Source token: \n{\n\tprefix: \"%s\",\n\tword: \"%s\",\n\tsuffix: \"%s\"\n}",
                        token->m_prefix.c_str(),
                        token->m_word.c_str(),
                        token->m_suffix.c_str()
            );
            ImGui::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
        bool edited = NodeView::draw_input(_member);
        if ( edited )
        {
            _member->get_owner()->set_dirty();
        }

    };

    Node* owner = _view->get_owner();
    ImGui::Text("Name:       \"%s\"", owner->get_label());
    ImGui::Text("Short Name: \"%s\"", owner->get_short_label());
    ImGui::Text("Class:      %s", owner->get_class()->get_name());

    // Draw exposed input members
    ImGui::Separator();
    ImGui::Text("Input(s):" );
    ImGui::Separator();
    ImGui::Indent();
    for (auto& eachView : _view->m_exposed_input_only_members )
    {
        drawMember(eachView->m_member);
        ImGui::Separator();
    }
    if( _view->m_exposed_input_only_members.empty() )
    {
        ImGui::Text("None.");
    }
    ImGui::Unindent();

    // Draw exposed output members
    ImGui::Text("Output(s):" );
    ImGui::Separator();
    ImGui::Indent();
    for (auto& eachView : _view->m_exposed_out_or_inout_members )
    {
        drawMember(eachView->m_member);
        ImGui::Separator();
    }

    if( _view->m_exposed_out_or_inout_members.empty() )
    {
        ImGui::Text("None.");
    }

    ImGui::Unindent();
    ImGui::Separator();
    ImGui::Checkbox( "Debug ?", _show_advanced );
    if ( *_show_advanced )
    {
        ImGui::Indent();

        // Draw exposed output members
        ImGui::Separator();
        ImGui::Text("Special members" );
        ImGui::Separator();
        ImGui::Indent();
        drawMember(_view->m_exposed_this_member_view->m_member);
        ImGui::Unindent();
        ImGui::Separator();

        // Advanced properties
        const Node *node = _view->get_owner();
        const float indent = 20.0f;

        // Components
        ImGui::Separator();
        ImGui::Text("Components :");
        for (auto &pair : node->get_components()) {
            Component *component = pair.second;
            ImGui::BulletText("%s", component->get_class()->get_name());
        }

        auto drawSlots = [](const char *label, const Slots<Node *> &slots) {
            ImGui::Text("%s", label);
            ImGui::Indent();
            if (!slots.empty()) {

                for (auto each : slots.content()) {
                    ImGui::Text("- %s", each->get_label());
                }
            } else {
                ImGui::TextUnformatted("None");
            }
            ImGui::Unindent();
        };

        ImGui::Separator();
        drawSlots("Inputs:", node->input_slots());
        ImGui::Separator();
        drawSlots("Outputs:", node->output_slots());
        ImGui::Separator();
        drawSlots("Predecessors:", node->predecessor_slots());
        ImGui::Separator();
        drawSlots("Successors:", node->successor_slots());
        ImGui::Separator();
        drawSlots("Children:", node->children_slots());
        ImGui::Separator();

        // Parent graph
        {
            std::string parentName = "NULL";

            if (node->get_parent_graph()) {
                parentName = node->get_parent_graph()->get_label();
                parentName.append(node->get_parent_graph()->is_dirty() ? " (dirty)" : "");

            }
            ImGui::Text("Parent graph is \"%s\"", parentName.c_str());
        }

        // Parent
        ImGui::Separator();
        {
            std::string parentName = "NULL";

            if (node->get_parent()) {
                parentName = node->get_parent()->get_label();
                parentName.append(node->get_parent()->is_dirty() ? " (dirty)" : "");
            }
            ImGui::Text("Parent node is \"%s\"", parentName.c_str());
        }

        // dirty state
        ImGui::Separator();
        bool b = _view->get_owner()->is_dirty();
        ImGui::Checkbox("Is dirty ?", &b);

        // Scope specific:
        ImGui::Separator();
        if (auto scope = node->get<Scope>()) {
            ImGui::Text("Variables:");
            auto vars = scope->get_variables();
            for (auto eachVar : vars) {
                ImGui::Text("%s: %s", eachVar->get_name(), eachVar->get_value()->convert_to<std::string>().c_str());
            }
        }
        ImGui::Separator();
    }

}

void NodeView::constraint_to_rect(NodeView* _view, ImRect _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect)) {

		_rect.Expand(vec2(-2, -2)); // shrink

		auto nodeRect = _view->get_rect();

		auto newPos = _view->get_position_rounded();

		auto left  = _rect.Min.x - nodeRect.Min.x;
		auto right = _rect.Max.x - nodeRect.Max.x;
		auto up    = _rect.Min.y - nodeRect.Min.y;
		auto down  = _rect.Max.y - nodeRect.Max.y;

		     if ( left > 0 )  nodeRect.TranslateX(left);
		else if ( right < 0 ) nodeRect.TranslateX(right);
			 
			 if ( up > 0 )    nodeRect.TranslateY(up);
		else if ( down < 0 )  nodeRect.TranslateY(down);

        _view->set_position(nodeRect.GetCenter());
	}

}

bool NodeView::is_exposed(const Member *_member)const
{
    return m_exposed_members.find(_member) != m_exposed_members.end();
}

void NodeView::set_view_detail(NodeViewDetail _viewDetail)
{
    NodeView::s_view_detail = _viewDetail;

    for( auto& eachView : NodeView::s_instances)
    {
        for( auto& eachPair : eachView->m_exposed_members )
        {
            MemberView* memberView = eachPair.second;
            memberView->reset();
        }
    }
}

vec2 NodeView::get_screen_position()
{
    return m_position - (ImGui::GetCursorPos() - ImGui::GetCursorScreenPos());
}

ImRect NodeView::get_rect(bool _recursively, bool _ignorePinned, bool _ignoreMultiConstrained, bool _ignoreSelf)
{

    if( !_recursively)
    {
        return ImRect(m_position - m_size * 0.5f, m_position + m_size * 0.5f);
    }

    ImRect rect(
            vec2(std::numeric_limits<float>().max()),
            vec2(-std::numeric_limits<float>().max()) );

    auto enlarge_to_fit = [&rect](const ImRect& other) {
        if( other.Min.x < rect.Min.x) rect.Min.x = other.Min.x;
        if( other.Min.y < rect.Min.y) rect.Min.y = other.Min.y;
        if( other.Max.x > rect.Max.x) rect.Max.x = other.Max.x;
        if( other.Max.y > rect.Max.y) rect.Max.y = other.Max.y;
    };

    if ( !_ignoreSelf)
    {
        ImRect self_rect = get_rect(false);
        enlarge_to_fit(self_rect);
    }

    auto enlarge_to_fit_all = [&](NodeView* eachView) {
        if (eachView && eachView->is_visible() && !(eachView->m_pinned && _ignorePinned) &&
            eachView->should_follow_output(this) )
        {
            ImRect childRect = eachView->get_rect(true, _ignorePinned, _ignoreMultiConstrained);
            enlarge_to_fit(childRect);
        }
    };

    std::for_each(m_children_slots.begin(), m_children_slots.end(), enlarge_to_fit_all);
    std::for_each(m_input_slots.begin(), m_input_slots.end(), enlarge_to_fit_all);

//    auto draw_list = ImGui::GetForegroundDrawList();
//    auto screen_rect = rect;
//    screen_rect.Translate( View::ToScreenPosOffset() );
//    if ( NodeView::IsSelected(this) )
//        draw_list->AddRect(screen_rect.Min, screen_rect.Max, ImColor(0,255,0));

    return rect;
}

void NodeView::clear_constraints() {
    m_constraints.clear();
}

void NodeView::add_constraint(NodeViewConstraint &_constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void NodeView::apply_constraints(float _dt) {
    for ( NodeViewConstraint& eachConstraint : m_constraints)
    {
        eachConstraint.apply(_dt);
    }
}

void NodeView::add_force_to_translate_to(vec2 desiredPos, float _factor, bool _recurse)
{
    vec2 delta(desiredPos - m_position);
    auto factor = std::max(0.0f, _factor);
    add_force(delta * factor, _recurse);
}

void NodeView::add_force(vec2 force, bool _recurse)
{
    m_forces_sum += force;

    if ( _recurse )
    {
        for ( auto eachInputView : m_input_slots )
        {
            if ( !eachInputView->m_pinned && eachInputView->should_follow_output(this))
                eachInputView->add_force(force, _recurse);
        }
    }
}

void NodeView::apply_forces(float _dt, bool _recurse) {
    //
    float magnitude = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );

    // apply
    constexpr float magnitude_max  = 100.0f;
    const float friction   = Maths::lerp (  0.0f, 0.5f, magnitude / magnitude_max);
    const vec2 avg_forces_sum = (m_forces_sum + m_last_frame_forces_sum) * 0.5f;
    this->translate( avg_forces_sum * ( 1.0f - friction) * _dt , _recurse);

    m_last_frame_forces_sum = m_forces_sum;
    m_forces_sum = vec2();
}

void NodeView::translate_to(vec2 desiredPos, float _factor, bool _recurse) {

    vec2 delta(desiredPos - m_position);

    bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
    if (!isDeltaTooSmall)
    {
        auto factor = std::min(1.0f, _factor);
        translate(delta * factor, _recurse);
    }
}

ImRect NodeView::get_rect(
        const std::vector<NodeView *>& _views,
        bool _recursive,
        bool _ignorePinned,
        bool _ignoreMultiConstrained) {

    std::vector<float> x_positions, y_positions;
    for (auto eachView : _views)
    {
        if (eachView->is_visible())
        {
            auto rect = eachView->get_rect(_recursive, _ignorePinned, _ignoreMultiConstrained);
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

void NodeView::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);
    for( auto each_child : m_children_slots )
    {
        each_child->set_expanded_rec(_expanded);
    }
}

void NodeView::set_expanded(bool _expanded)
{
    m_expanded = _expanded;
    set_inputs_visible(_expanded, true);
    set_children_visible(_expanded, true);
}

bool NodeView::should_follow_output(const NodeView* output)
{
    if ( m_output_slots.empty())
    {
        return true;
    }
    return m_output_slots[0] == output;
}

void NodeView::set_inputs_visible(bool _visible, bool _recursive)
{
    for( auto each_input : m_input_slots )
    {
        if( _visible || (output_slots().empty() || each_input->should_follow_output(this)) )
        {
            if ( _recursive && each_input->m_expanded ) // propagate only if expanded
            {
                each_input->set_children_visible(_visible, true);
                each_input->set_inputs_visible(_visible, true);
            }
            each_input->set_visible(_visible);
        }
    }
}

void NodeView::set_children_visible(bool _visible, bool _recursive)
{
    for( auto each_child : m_children_slots )
    {
        if( _visible || (output_slots().empty() || each_child->should_follow_output(this)) )
        {
            if ( _recursive && each_child->m_expanded) // propagate only if expanded
            {
                each_child->set_children_visible(_visible, true);
                each_child->set_inputs_visible(_visible, true);
            }
            each_child->set_visible(_visible);
        }
    }
}

void NodeView::expand_toggle()
{
    set_expanded(!m_expanded);
}

NodeView* NodeView::substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive)
{
    if( !_view->is_visible() )
    {
        if ( Node* parent = _view->get_owner()->get_parent() )
        {
            if ( auto parent_view = parent->get<NodeView>() )
            {
                if (  _recursive )
                {
                    return substitute_with_parent_if_not_visible(parent_view, _recursive);
                }
                else
                {
                    return parent_view;
                }
            }
        }
    }
    return _view;
}

void NodeView::expand_toggle_rec()
{
    return set_expanded_rec(!m_expanded);
}
