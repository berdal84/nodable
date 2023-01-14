#include <nodable/app/NodeView.h>

#include <cmath>                  // for sinus
#include <algorithm>              // for std::max
#include <vector>

#include <nodable/app/Settings.h>
#include <nodable/core/languages/NodableSerializer.h>
#include <nodable/app/App.h>
#include <nodable/core/math.h>
#include <nodable/core/Scope.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/GraphNode.h>
#include <nodable/app/NodeConnector.h>
#include <nodable/app/MemberConnector.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/app/IAppCtx.h>
#include <nodable/core/reflection/registration.h>

#define NODE_VIEW_DEFAULT_SIZE vec2(10.0f, 35.0f)

using namespace ndbl;

REGISTER
{
    registration::push_class<NodeView>("NodeView")
        .extends<Component>()
        .extends<View>();
}

NodeView*          NodeView::s_selected                        = nullptr;
NodeView*          NodeView::s_dragged                         = nullptr;
NodeViewDetail     NodeView::s_view_detail                     = NodeViewDetail::Default;
const float        NodeView::s_member_input_size_min           = 10.0f;
const vec2         NodeView::s_member_input_toggle_button_size = vec2(10.0, 25.0f);
NodeViewVec        NodeView::s_instances;

NodeView::NodeView(IAppCtx& _ctx)
        : Component()
        , View(_ctx)
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
        , m_apply_constraints(true)
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
    for( auto& conn : m_successors ) delete conn;
    for( auto& conn : m_predecessors ) delete conn;

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
        return node->get_short_label();
    }
    return node->get_label();
}

void NodeView::expose(Member* _member)
{
    auto member_view = new MemberView(m_ctx, _member, this);

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

    Settings&            settings = m_ctx.settings();
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
    type clss = _node->get_type();

    if (_node->has<InvokableComponent>())
    {
        set_color(Color_Fill, &settings.ui_node_invokableColor); // blue
    }
    else if (clss.is_child_of<VariableNode>())
    {
        set_color(Color_Fill, &settings.ui_node_variableColor); // purple
    }
    else if (clss.is_child_of<LiteralNode>())
    {
        set_color(Color_Fill, &settings.ui_node_literalColor);
    }
    else
    {
        set_color(Color_Fill, &settings.ui_node_instructionColor); // green
    }

    // NodeConnectors
    //---------------

    // add q successor connector per successor slot
    const size_t successor_max_count = _node->successors().get_limit();
    for(size_t index = 0; index < successor_max_count; ++index )
    {
        m_successors.push_back(new NodeConnector(m_ctx, *this, Way_Out, index, successor_max_count));
    }

    // add a single predecessor connector if node can be connected in this way
    if(_node->predecessors().get_limit() != 0)
        m_predecessors.push_back(new NodeConnector(m_ctx, *this, Way_In));

    m_nodeRelationAddedObserver = _node->m_on_relation_added.createObserver(
        [this](Node* _other_node, EdgeType _relation )
        {
            NodeView* _other_node_view = _other_node->get<NodeView>();
            switch ( _relation )
            {
                case EdgeType::IS_CHILD_OF:
                    children().add(_other_node_view ); break;
                case EdgeType::IS_INPUT_OF:
                    inputs().add(_other_node_view ); break;
                case EdgeType::IS_OUTPUT_OF:
                    outputs().add(_other_node_view ); break;
                case EdgeType::IS_SUCCESSOR_OF:
                    successors().add(_other_node_view ); break;
                case EdgeType::IS_PREDECESSOR_OF: NDBL_ASSERT(false); /* NOT HANDLED */break;
            }
        });

    m_nodeRelationRemovedObserver = _node->m_on_relation_removed.createObserver(
    [this](Node* _other_node, EdgeType _relation )
        {
            NodeView* _other_node_view = _other_node->get<NodeView>();
            switch ( _relation )
            {
                case EdgeType::IS_CHILD_OF:
                    children().remove(_other_node_view ); break;
                case EdgeType::IS_INPUT_OF:
                    inputs().remove(_other_node_view ); break;
                case EdgeType::IS_OUTPUT_OF:
                    outputs().remove(_other_node_view ); break;
                case EdgeType::IS_SUCCESSOR_OF:
                    successors().remove(_other_node_view ); break;
                case EdgeType::IS_PREDECESSOR_OF: NDBL_ASSERT(false); /* NOT HANDLED */break;
            }
        });
}

void NodeView::set_selected(NodeView* _view)
{
    if( s_selected == _view ) return;

    if( s_selected)
    {
        Event event{ EventType::node_view_deselected };
        event.node.view = s_selected;
        EventManager::push_event(event);
    }

    if( _view )
    {
        Event event{ EventType::node_view_selected };
        event.node.view = _view;
        EventManager::push_event(event);
    }

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
    auto found = m_exposed_members.find(_member);
    if( found == m_exposed_members.end() ) return nullptr;
    return found->second;
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
	    for(auto eachInput : get_owner()->inputs() )
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
    math::lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
    this->apply_forces(_deltaTime, false);
	return true;
}

bool NodeView::draw()
{
	bool      changed  = false;
	auto      node     = get_owner();
	Settings& settings = m_ctx.settings();

	NDBL_ASSERT(node != nullptr);

    // Draw Node connectors (in background)
    bool is_connector_hovered = false;
    {
        ImColor color = settings.ui_node_nodeConnectorColor;
        ImColor hoveredColor = settings.ui_node_nodeConnectorHoveredColor;

        auto draw_and_handle_evt = [&](NodeConnector *connector)
        {
            NodeConnector::draw(connector, color, hoveredColor, m_edition_enable);
            is_connector_hovered |= ImGui::IsItemHovered();
        };

        std::for_each(m_predecessors.begin(), m_predecessors.end(), draw_and_handle_evt);
        std::for_each(m_successors.begin()  , m_successors.end()  , draw_and_handle_evt);

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
		auto borderCol = is_selected(this) ? m_border_color_selected : get_color(Color_Border);

		auto itemRectMin = screen_cursor_pos_content_start - halfSize;
		auto itemRectMax = screen_cursor_pos_content_start + halfSize;

		// Draw the rectangle under everything
		ImGuiEx::DrawRectShadow(itemRectMin, itemRectMax, m_border_radius, 4, vec2(1.0f), get_color(Color_Shadow));
		draw_list->AddRectFilled(itemRectMin, itemRectMax, get_color(Color_Fill), m_border_radius);
		draw_list->AddRect(itemRectMin + vec2(1.0f), itemRectMax, get_color(Color_BorderHighlights), m_border_radius);
		draw_list->AddRect(itemRectMin, itemRectMax, borderCol, m_border_radius);

		// darken the background under the content
		draw_list->AddRectFilled(itemRectMin + vec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + settings.ui_node_padding), itemRectMax, ImColor(0.0f, 0.0f, 0.0f, 0.1f), m_border_radius, 4);

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
	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings.ui_node_padding * 2.0f); // x2 padding to keep space for "this" connector
	ImGui::SetCursorPosY( ImGui::GetCursorPosY() + settings.ui_node_padding );
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
        ImGuiEx::ShadowedText(vec2(1.0f), get_color(Color_BorderHighlights), label.c_str()); // text with a lighter shadow (incrust effect)

        ImGui::SameLine();

        ImGui::BeginGroup();
        // Draw inputs
        for (auto &memberView : m_exposed_input_only_members)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(cursor_pos_content_start.y + 1.0f);
            changed |= draw(memberView);
        }

        // Draw outputs
        for (auto &memberView : m_exposed_out_or_inout_members)
        {
            ImGui::SameLine();
            ImGui::SetCursorPosY(cursor_pos_content_start.y + 8.0f);
            changed |= draw(memberView);
        }

        ImGui::EndGroup();
        ImGui::SameLine();

        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + settings.ui_node_padding * 2.0f);
        ImGui::SetCursorPosY( ImGui::GetCursorPosY() + settings.ui_node_padding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    m_size.x = std::ceil(ImGui::GetItemRectSize().x );
    m_size.y = std::max(NODE_VIEW_DEFAULT_SIZE.y, std::ceil(ImGui::GetItemRectSize().y ));
    m_size.x = std::max( 1.0f, m_size.x); // to avoid 0 sized rectangle 
    m_size.y = std::max( 1.0f, m_size.y);

    // Draw Member in/out connectors
    {
        float radius      = settings.ui_node_memberConnectorRadius;
        ImColor color     = settings.ui_node_nodeConnectorColor;
        ImColor borderCol = settings.ui_node_borderColor;
        ImColor hoverCol  = settings.ui_node_nodeConnectorHoveredColor;

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
            node->flag_to_delete();
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

	if( changed )
    {
        get_owner()->set_dirty();
    }


    m_is_hovered = is_node_hovered || is_connector_hovered;

	return changed;
}

bool NodeView::draw(MemberView* _view )
{
    bool    show;
    bool    changed = false;
    Member* member  = _view->m_member;

    type owner_type = member->get_owner()->get_type();

    /*
     * Handle input visibility
     */


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
        show = member->get_variant()->is_defined();
    }
    else if( !member->has_input_connected() )
    {
        show = member->get_variant()->is_defined();   // we always show a defined unconnected member
    }
    else if (member->get_type().is_ptr() )
    {
        show = member->is_connected_to_variable();
    }
    else if ( member->is_connected_to_variable() )
    {
        show = true;
    }
    else
    {
        show = member->get_variant()->is_defined();
    }

    _view->m_showInput = show;

    vec2 new_relative_pos = ImGui::GetCursorScreenPos() - get_screen_position();

    // input
    float input_size = NodeView::s_member_input_toggle_button_size.x;

    if ( _view->m_showInput )
    {
        bool limit_size = member->get_type() != type::get<bool>();

        if ( limit_size )
        {
            // try to draw an as small as possible input field
            std::string str;
            if (member->is_connected_to_variable())
            {
                str = member->get_connected_variable()->get_identifier();
            }
            else
            {
                str = member->convert_to<std::string>();
            }
            input_size = 5.0f + std::max(ImGui::CalcTextSize(str.c_str()).x, NodeView::s_member_input_size_min);
            ImGui::PushItemWidth(input_size);
        }
        changed = NodeView::draw_input(m_ctx, member, nullptr);

        if ( limit_size )
        {
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Button("", NodeView::s_member_input_toggle_button_size);

        if ( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s (%s)",
                        member->get_name().c_str(),
                        member->get_type().get_fullname().c_str());
            ImGuiEx::EndTooltip();
        }

        if ( ImGui::IsItemClicked(0) )
        {
            _view->m_showInput = !_view->m_showInput;
            _view->m_touched = true;
        }
    }

    new_relative_pos.x += input_size * 0.5f; // center over input
    _view->relative_pos(new_relative_pos);

    return changed;
}

bool NodeView::draw_input(IAppCtx& _ctx, Member *_member, const char *_label)
{
    bool  changed = false;
    Node* node    = _member->get_owner();

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
        snprintf(str, 255, "%s", variable->get_identifier() );

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4) variable->get<NodeView>()->get_color(Color_Fill) );
        ImGui::InputText(label.c_str(), str, 255, inputFlags);
        ImGui::PopStyleColor();

    }
    else if( !_member->get_variant()->is_initialized() )
    {
        ImGui::LabelText(label.c_str(), "uninitialized!");
    }
    else
    {
        /* Draw the member */
        type t = _member->get_type();

        if( t == type::get<i16_t>() )
        {
            auto i16 = (i16_t)*_member;

            if (ImGui::InputInt(label.c_str(), &i16, 0, 0, inputFlags ) && !_member->has_input_connected())
            {
                _member->set(i16);
                changed |= true;
            }
        }
        else if( t == type::get<double>() )
        {
            auto d = (double)*_member;

            if (ImGui::InputDouble(label.c_str(), &d, 0.0F, 0.0F, "%g", inputFlags ) && !_member->has_input_connected())
            {
                _member->set(d);
                changed |= true;
            }
        }
        else if( t == type::get<std::string>() )
        {
            char str[255];
            snprintf(str, 255, "%s", ((std::string)*_member).c_str() );

            if ( ImGui::InputText(label.c_str(), str, 255, inputFlags) && !_member->has_input_connected() )
            {
                _member->set( std::string(str) );
                changed |= true;
            }
        }
        else if( t == type::get<bool >() )
        {
            std::string checkBoxLabel = _member->get_name();

            auto b = (bool)*_member;

            if (ImGui::Checkbox(label.c_str(), &b ) && !_member->has_input_connected() )
            {
                _member->set(b);
                changed |= true;
            }
        }
        else
        {
            auto member_as_string = _member->get_variant()->convert_to<std::string>();
            ImGui::Text( "%s", member_as_string.c_str());
        }

        /* If value is hovered, we draw a tooltip that print the source expression of the value*/
        if (ImGuiEx::BeginTooltip())
        {
            std::string buffer;
            _ctx.language().get_serializer().serialize(buffer, _member);
            ImGui::Text("%s", buffer.c_str() );
            ImGuiEx::EndTooltip();
        }
    }

    return changed;
}

bool NodeView::is_inside(NodeView* _nodeView, ImRect _rect) {
	auto nodeRect = _nodeView->get_rect();
	return _rect.Contains(nodeRect);
}

void NodeView::draw_as_properties_panel(IAppCtx &_ctx, NodeView *_view, bool *_show_advanced)
{
    Node*       node             = _view->get_owner();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto drawMember = [&](Member* _member)
    {
        // label (<name> (<way> <type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s, %s): ",
                _member->get_name().c_str(),
                WayToString(_member->get_allowed_connection()).c_str(),
                _member->get_type().get_fullname().c_str());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            std::shared_ptr<Token> token = _member->get_src_token();
            const auto variant = _member->get_variant();
            ImGui::Text("initialized: %s,\n"
                        "defined:     %s,\n"
                        "Source token:\n"
                        "%s\n",
                        variant->is_initialized() ? "true" : "false",
                        variant->is_defined()     ? "true" : "false",
                        Token::to_JSON(token).c_str()
                        );
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool edited = NodeView::draw_input(_ctx, _member, nullptr);
        if ( edited )
        {
            _member->get_owner()->set_dirty();
        }

    };

    ImGui::Text("Name:       \"%s\"" , node->get_label());
    ImGui::Text("Short Name: \"%s\"" , node->get_short_label());
    ImGui::Text("Class:      %s"     , node->get_type().get_name());

    // Draw exposed input members
    ImGui::Separator();
    ImGui::Text("Input(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_exposed_input_only_members.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& eachView : _view->m_exposed_input_only_members )
        {
            drawMember(eachView->m_member);
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    // Draw exposed output members
    ImGui::Text("Output(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_exposed_out_or_inout_members.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& eachView : _view->m_exposed_out_or_inout_members )
        {
            drawMember(eachView->m_member);
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    if ( ImGui::TreeNode("Debug") )
    {
        // Draw exposed output members
        if( ImGui::TreeNode("Other Members") )
        {
            drawMember(_view->m_exposed_this_member_view->m_member);
            ImGui::TreePop();
        }

        // Components
        if( ImGui::TreeNode("Components") )
        {
            for (auto &pair : node->get_components())
            {
                Component *component = pair.second;
                ImGui::BulletText("%s", component->get_type().get_name());
            }
            ImGui::TreePop();
        }

        if( ImGui::TreeNode("Slots") )
        {
            auto draw_slots = [](const char *label, const Slots<Node *> &slots)
            {
                if( ImGui::TreeNode(label) )
                {
                    if (!slots.empty())
                    {
                        for (auto each : slots.content())
                        {
                            ImGui::BulletText("- %s", each->get_label());
                        }
                    }
                    else
                    {
                        ImGui::BulletText("None");
                    }
                    ImGui::TreePop();
                }
            };

            draw_slots("Inputs:"      , node->inputs());
            draw_slots("Outputs:"     , node->outputs());
            draw_slots("Predecessors:", node->predecessors());
            draw_slots("Successors:"  , node->successors());
            draw_slots("Children:"    , node->children_slots());

            ImGui::TreePop();
        }

        // m_apply_constraints
        ImGui::Separator();
        if( ImGui::TreeNode("Constraints") )
        {
            ImGui::Checkbox("Apply", &_view->m_apply_constraints);
            int i = 0;
            for(auto& constraint : _view->m_constraints)
            {
                constraint.draw_view();
            }
            ImGui::TreePop();
        }

        // Scope specific:
        ImGui::Separator();
        if (auto scope = node->get<Scope>())
        {
            if( ImGui::TreeNode("Variables") )
            {
                auto vars = scope->get_variables();
                for (auto eachVar : vars)
                {
                    ImGui::BulletText("%s: %s", eachVar->get_identifier(), eachVar->get_value()->convert_to<std::string>().c_str());
                }
                ImGui::TreePop();
            }
        }

        if( ImGui::TreeNode("Misc:") )
        {
            // dirty state
            ImGui::Separator();
            bool b = _view->get_owner()->is_dirty();
            ImGui::Checkbox("Is dirty ?", &b);

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
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
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
        return { m_position - m_size * 0.5f, m_position + m_size * 0.5f};
    }

    ImRect result_rect( vec2(std::numeric_limits<float>::max()), vec2(-std::numeric_limits<float>::max()) );

    if ( !_ignoreSelf && m_is_visible )
    {
        ImRect self_rect = get_rect(false);
        ImGuiEx::EnlargeToInclude(result_rect, self_rect);
    }

    auto enlarge_to_fit_all = [&](NodeView* _view)
    {
        if( !_view) return;

        if ( _view->m_is_visible && !(_view->m_pinned && _ignorePinned) && _view->should_follow_output(this) )
        {
            ImRect child_rect = _view->get_rect(true, _ignorePinned, _ignoreMultiConstrained);
            ImGuiEx::EnlargeToInclude(result_rect, child_rect);
        }
    };

    std::for_each(m_children_slots.begin(), m_children_slots.end(), enlarge_to_fit_all);
    std::for_each(m_input_slots.begin()   , m_input_slots.end()   , enlarge_to_fit_all);

    return result_rect;
}

void NodeView::clear_constraints() {
    m_constraints.clear();
}

void NodeView::add_constraint(ViewConstraint &_constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void NodeView::apply_constraints(float _dt)
{
    if( !m_apply_constraints ) return;

    for ( ViewConstraint& eachConstraint : m_constraints)
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
        for ( auto each_input : m_input_slots )
        {
            if (!each_input->m_pinned && each_input->should_follow_output(this))
            {
                each_input->add_force(force, _recurse);
            }
        }
    }
}

void NodeView::apply_forces(float _dt, bool _recurse)
{
    float magnitude = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );

    constexpr float magnitude_max  = 100.0f;
    const float     friction       = math::lerp (0.0f, 0.5f, magnitude / magnitude_max);
    const vec2 avg_forces_sum      = (m_forces_sum + m_last_frame_forces_sum) * 0.5f;

    translate( avg_forces_sum * ( 1.0f - friction) * _dt , _recurse);

    m_last_frame_forces_sum = m_forces_sum;
    m_forces_sum            = vec2();
}

void NodeView::translate_to(vec2 desiredPos, float _factor, bool _recurse)
{
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
        bool _ignoreMultiConstrained)
{
    ImRect result_rect( vec2(std::numeric_limits<float>::max()), vec2(-std::numeric_limits<float>::max()) );

    for (auto eachView : _views)
    {
        if ( eachView->m_is_visible )
        {
            auto rect = eachView->get_rect(_recursive, _ignorePinned, _ignoreMultiConstrained);
            ImGuiEx::EnlargeToInclude(result_rect, rect);
        }
    }

    return result_rect;
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
        if( _visible || (outputs().empty() || each_input->should_follow_output(this)) )
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
        if( _visible || (outputs().empty() || each_child->should_follow_output(this)) )
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
