#include "Graph_View.h"

#include <algorithm>
#include "imgui.h"
#include "tools/core/Types.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/core/Math.h"
#include "tools/core/State_Machine.h"
#include "tools/gui/App.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"

#include "Config.h"
#include "Event.h"
#include "Node_View.h"
#include "Physics_Component.h"
#include "Node_Slot_View.h"
#include "Scope_View.h"

using namespace ndbl;
using namespace tools;

// Popup name
constexpr const char* CONTEXT_POPUP    = "Graph_View.ContextMenuPopup";
// Tool names
constexpr const char* CURSOR_STATE     = "Cursor Tool";
constexpr const char* ROI_STATE        = "Selection Tool";
constexpr const char* DRAG_STATE       = "Drag Node Tool";
constexpr const char* VIEW_PAN_STATE   = "Grab View Tool";
constexpr const char* LINE_STATE       = "Line Tool";

Graph_View::Graph_View()
: Component<Graph>("View")
, _m_state_machine(this)
, _m_shape( Vec2{100.f, 100.f} ) // non null area
{
    Component::signal_init.connect<&Graph_View::_handle_init>(this);
    Component::signal_shutdown.connect<&Graph_View::_handle_shutdown>(this);

    _m_state_machine.add_state(CURSOR_STATE);
    _m_state_machine.bind<&Graph_View::cursor_state_tick>(CURSOR_STATE, When::OnTick);
    _m_state_machine.set_default_state(CURSOR_STATE);

    _m_state_machine.add_state(ROI_STATE);
    _m_state_machine.bind<&Graph_View::roi_state_enter>(ROI_STATE, When::OnEnter);
    _m_state_machine.bind<&Graph_View::roi_state_tick>(ROI_STATE, When::OnTick);

    _m_state_machine.add_state(DRAG_STATE);
    _m_state_machine.bind<&Graph_View::drag_state_enter>(DRAG_STATE, When::OnEnter);
    _m_state_machine.bind<&Graph_View::drag_state_tick>(DRAG_STATE, When::OnTick);

    _m_state_machine.add_state(VIEW_PAN_STATE);
    _m_state_machine.bind<&Graph_View::view_pan_state_tick>(VIEW_PAN_STATE, When::OnTick);

    _m_state_machine.add_state(LINE_STATE);
    _m_state_machine.bind<&Graph_View::line_state_enter>(LINE_STATE, When::OnEnter);
    _m_state_machine.bind<&Graph_View::line_state_tick>(LINE_STATE, When::OnTick);
    _m_state_machine.bind<&Graph_View::line_state_leave>(LINE_STATE, When::OnLeave);
}

Graph_View::~Graph_View()
{
    Component::signal_init.disconnect(); // Simple_Signal
    Component::signal_shutdown.disconnect();
}

void Graph_View::_handle_init()
{
    // add nodes present before connecting signals
    for( auto* node : graph()->nodes() )
    {
        _handle_add_node(node);
    }

    _m_selection.signal_change.connect<&Graph_View::_on_selection_change>(this);
    graph()->signal_change.connect<&Graph_View::_on_graph_change>(this);
    graph()->signal_add_node.connect<&Graph_View::_handle_add_node>(this);
    graph()->signal_remove_node.connect<&Graph_View::_handle_remove_node>(this);
    graph()->signal_change_scope.connect<&Graph_View::_handle_change_scope>(this);
    graph()->signal_reset.connect<&Graph_View::reset>(this);
    graph()->signal_is_complete.connect<&Graph_View::reset>(this);

    _m_state_machine.start();
}

void Graph_View::_handle_shutdown()
{
    _m_state_machine.stop();

    _m_selection.signal_change.disconnect();
    graph()->signal_add_node.disconnect();
    graph()->signal_remove_node.disconnect();
    graph()->signal_reset.disconnect();
    graph()->signal_is_complete.disconnect();
    ASSERT_DEBUG_ONLY( graph()->signal_change.disconnect<&Graph_View::_on_graph_change>(this) );

    // add nodes still present after connecting signals
    for( auto* node : graph()->nodes() )
    {
        _handle_remove_node(node);
    }
}

void Graph_View::_handle_add_node(Node* node)
{
    // view
    auto* nodeview = node->components()->create<Node_View>();
    nodeview->set_size({20.f, 35.f});

    if (Scope_View* scopeview = nodeview->internal_scopeview() )
        scopeview->signal_hover.connect<&Graph_View::_handle_hover>(this); // I'm not sure if this is a good approach...

    if( graph()->root_node() == node )
    {
        // root must be parented to the graph view itself
        spatial_node()->add_child( nodeview->spatial_node() );
    }
    else
    {
        Spatial_Node* scopeview_spatial_node = node->scope()->view()->spatial_node();
        scopeview_spatial_node->add_child( nodeview->spatial_node() );
    }

    // physics
    node->components()->create<Physics_Component>();
}

void Graph_View::_handle_remove_node(Node* node)
{
    // clean physics
    auto* physics_component = node->component<Physics_Component>();
    VERIFY(physics_component, "Should have been created from _handle_add_node()");
    node->components()->destroy( physics_component );

    // clean nodeview
    auto* nodeview = node->component<Node_View>();
    VERIFY(nodeview, "Should have been created from _handle_add_node()");

    if ( Scope_View* scopeview = nodeview->internal_scopeview() )
    {
        scopeview->signal_hover.disconnect(); // I'm not sure if this is a good approach...
    }

    if( Spatial_Node* _parent = nodeview->spatial_node()->parent() )
    {
        _parent->remove_child( nodeview->spatial_node() );
    }

    node->components()->destroy( nodeview );
}

void Graph_View::_handle_change_scope(Node* node, Scope* old_scope, Scope* new_scope)
{
    auto* nodeview = node->component<Node_View>();
    VERIFY(nodeview, "a nodeview must be present since we are in a Graph_View");

    // Un-parent from old scope's spatial node
    if( auto _parent = nodeview->spatial_node()->parent() )
        _parent->remove_child( nodeview->spatial_node() );

    // Parent to new scope or default to graph's spatial node
    if( Scope_View* _scopeview = new_scope->view() )
        _scopeview->spatial_node()->add_child( nodeview->spatial_node() );
}

ImGuiID make_wire_id(const Node_Slot *ptr1, const Node_Slot *ptr2)
{
    String_128 id;
    id.append_fmt("wire %zu->%zu", ptr1, ptr2);
    return ImGui::GetID(id.c_str());
}

void Graph_View::draw_wire_from_slot_to_pos(Node_Slot_View *from, const Vec2 &end_pos)
{
    VERIFY(from != nullptr, "from slot can't be nullptr");

    Config* cfg = get_config();

    // Style

    ImGuiEx::WireStyle style;
    style.shadow_color = cfg->ui_codeflow_shadowColor,
    style.roundness    = 0.f;

    if (from->slot->type() == Node_Slot_Flag_TYPE_FLOW) {
        style.color = cfg->ui_codeflow_color,
                style.thickness = cfg->ui_slot_rectangle_size.x * cfg->ui_codeflow_thickness_ratio;
    } else {
        style.color = cfg->ui_node_borderHighlightedColor;
        style.thickness = cfg->ui_wire_bezier_thickness;
    }

    // Draw

    ImGuiID id = make_wire_id(from->slot, nullptr);
    Vec2 start_pos = from->spatial_node()->position(WORLD_SPACE);

    Bezier_Curve_Segment_2D segment{
            start_pos, start_pos,
            end_pos, end_pos
    }; // straight line

    ImGuiEx::DrawWire(id, ImGui::GetWindowDrawList(), segment, style);
}

bool Graph_View::draw(float dt)
{
    bool changed = false;

    // Ensure view state fit with content region
    // (n.b. we could also implement a struct RootView_State wrapping View_State)
    Rect region = ImGuiEx::GetContentRegion(WORLD_SPACE);
    _m_shape.set_size( region.size() );
    _m_shape.set_position(region.center()); // children will be relative to the center
    _m_shape.draw_debug_info();

    _m_hovered = {};

    Config*         cfg       = get_config();
    ImDrawList*     draw_list = ImGui::GetWindowDrawList();

    // Draw Scopes
    std::vector<Scope*> scopes_to_draw = graph()->scopes();
    // TODO: we should sort them only when a new parent/child connection is created/deleted
    auto low_to_high_depth = [](Scope* s1, Scope* s2) { return s1->depth() < s2->depth(); };
    std::sort(scopes_to_draw.begin(), scopes_to_draw.end(), low_to_high_depth);

    for( Scope* scope : scopes_to_draw )
    {
        if (Scope_View* view = scope->view())
        {
            view->draw(dt);
        }
    }

    // Draw Grid
    const Rect window_content_region = {
        ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMin(),
        ImGui::GetWindowPos() + ImGui::GetWindowContentRegionMax()
    };
    ImGuiEx::Grid(
            window_content_region,
            cfg->ui_grid_size,
            cfg->ui_grid_subdiv_count,
            ImGui::GetColorU32(cfg->ui_graph_grid_color_major),
            ImGui::GetColorU32(cfg->ui_graph_grid_color_minor));

    // Draw Wires (code flow ONLY)
    const ImGuiEx::WireStyle code_flow_style{
            cfg->ui_codeflow_color,
            cfg->ui_codeflow_color, // hover
            cfg->ui_codeflow_shadowColor,
            cfg->ui_codeflow_thickness(),
            0.0f
    };
    for (Node* each_node: graph()->nodes() )
    {
        Node_View *each_view = Node_View::substitute_with_parent_if_not_visible(each_node->component<Node_View>() );

        if (!each_view) {
            continue;
        }

        std::vector<Node_Slot *> slots = each_node->filter_slots(Node_Slot_Flag_FLOW_OUT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index)
        {
            Node_Slot *slot = slots[slot_index];

            if (slot->empty())
            {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent())
            {
                Node*     each_successor_node  = adjacent_slot->node;
                Node_View* possibly_hidden_view = each_successor_node->component<Node_View>();
                Node_View* each_successor_view  = Node_View::substitute_with_parent_if_not_visible(possibly_hidden_view);

                if ( each_successor_view == nullptr )
                    continue;
                if ( !each_view->state()->visible() )
                    continue;
                if ( !each_successor_view->state()->visible() )
                    continue;

                Node_Slot_View* tail = slot->view;
                Node_Slot_View* head = adjacent_slot->view;

                ImGuiID id = make_wire_id(slot, adjacent_slot);
                Vec2 tail_pos = tail->spatial_node()->position(WORLD_SPACE);
                Vec2 head_pos = head->spatial_node()->position(WORLD_SPACE);
                Bezier_Curve_Segment_2D segment{
                        tail_pos,
                        tail_pos,
                        head_pos,
                        head_pos,
                };
                ImGuiEx::DrawWire(id, draw_list, segment, code_flow_style);
                if (ImGui::GetHoveredID() == id )
                {
                    _m_hovered = Node_Slot_Link_View{tail, head};
                }
            }
        }
    }

    // Draw Wires (regular)
    const ImGuiEx::WireStyle default_wire_style{
            cfg->ui_wire_color,
            cfg->ui_wire_color, // hover
            cfg->ui_wire_shadowColor,
            cfg->ui_wire_bezier_thickness,
            cfg->ui_wire_bezier_roundness.x // roundness min
    };
    float time = ImGui::GetTime();
    for (Node* node_out: graph()->nodes() )
    {
        for (const Node_Slot* slot_out: node_out->filter_slots(Node_Slot_Flag_OUTPUT))
        {
            for(const Node_Slot* slot_in : slot_out->adjacent())
            {
                if (slot_in == nullptr)
                    continue;

                auto *node_view_out = slot_out->node->component<Node_View>();
                auto *node_view_in  = slot_in->node->component<Node_View>();

                if ( !node_view_out->state()->visible() )
                    continue;
                if ( !node_view_in->state()->visible() )
                    continue;

                Vec2 p1, cp1, cp2, p2; // BezierCurveSegment's points

                Node_Slot_View* slot_view_out = slot_out->view;
                Node_Slot_View* slot_view_in  = slot_in->view;

                p1 = slot_view_out->spatial_node()->position(WORLD_SPACE);
                p2 = slot_view_in->spatial_node()->position(WORLD_SPACE);

                const Vec2  signed_dist = Vec2::distance(p1, p2);
                const float lensqr_dist = signed_dist.lensqr();

                // Animate style
                ImGuiEx::WireStyle style = default_wire_style;
                if ( _m_selection.contains( node_view_out ) || _m_selection.contains( node_view_in ) )
                {
                    style.color.w *= wave(0.5f, 1.f, time, 10.f);
                }
                else if (lensqr_dist > cfg->ui_wire_bezier_fade_lensqr_range.x)
                {
                    // transparent depending on wire length
                    float factor = (lensqr_dist - cfg->ui_wire_bezier_fade_lensqr_range.x) /
                                   (cfg->ui_wire_bezier_fade_lensqr_range.y - cfg->ui_wire_bezier_fade_lensqr_range.x);
                    style.color        = Vec4::lerp(style.color,        Vec4(0, 0, 0, 0), factor);
                    style.shadow_color = Vec4::lerp(style.shadow_color, Vec4(0, 0, 0, 0), factor);
                }

                // Draw transparent some "variable--->ref" wires in certain cases
                if (node_out->type() == Node_Type_VARIABLE ) // from a variable
                {
                    if (slot_out == node_out->variable_data().ref_out() ) // from a reference slot (can't be a declaration link)
                        if (!node_view_out->state()->selected() && !node_view_in->state()->selected() )
                            style.color.w *= 0.25f;
                }

                // draw the wire if necessary
                if (style.color.w != 0.f)
                {
                    // Determine control points
                    float roundness = tools::clamped_lerp(0.f, 10.f, lensqr_dist / 100.f);
                    cp1 = p1;
                    cp2 = p2 + slot_view_in->direction * roundness;
                    if ( slot_view_out->direction.y > 0.f ) // round out when direction is bottom
                        cp1 += slot_view_out->direction * roundness;

                    Bezier_Curve_Segment_2D segment{p1, cp1, cp2, p2};

                    ImGuiID id = make_wire_id(slot_view_out->slot, slot_in);
                    ImGuiEx::DrawWire(id, draw_list, segment, style);
                    if (ImGui::GetHoveredID() == id)
                    {
                        _m_hovered = Node_Slot_Link_View{slot_view_out, slot_view_in};
                    }
                }
            }
        }
    }

    // Draw Node_Views
    for (Node* node : graph()->nodes()  )
    {
        Node_View* nodeview = node->component<Node_View>();

        if ( !nodeview)
            continue;
        if ( !nodeview->state()->visible() )
            continue;

        changed |= nodeview->draw();

        if ( nodeview->state()->hovered() ) // no check if something else is hovered, last node always win against an edge
        {
            if ( nodeview->m_hovered_slotview != nullptr)
            {
                _m_hovered = nodeview->m_hovered_slotview;
            }
            else
                _m_hovered = {nodeview};
        }
    }

    _m_state_machine.tick();

    // Debug Infos
    if (cfg->tools_cfg->runtime_debug)
    {
        if (ImGui::Begin("Graph_ViewToolState_Machine"))
        {
            ImGui::Text("current_tool:         %s" , _m_state_machine.get_current_state_name());
            ImGui::Text("_m_focused.type:       %zu", _m_focused.index() );
            ImGui::Text("_m_hovered.type:       %zu", _m_hovered.index() );
            Vec2 mouse_pos = ImGui::GetMousePos();
            ImGui::Text("_m_mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
        }
        ImGui::End();
    }

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    if ( changed )
        signal_change.emit();

	return changed;
}

void Graph_View::_create_constraints__align_down(Node* follower, const  std::vector<Node*>& leader )
{
    if( leader.empty() )
        return;

    std::vector<Node_View*> leader_view;
    for ( Node* _leader : leader )
        leader_view.push_back(_leader->component<Node_View>() );

    Node_View* follower_view = follower->component<Node_View>();

    auto& constraint = _m_contraints.emplace_back();

    constraint.name           = "Position below previous";
    constraint.rule           = &ViewConstraintRule_1_to_N_as_row;
    constraint.leader         = leader_view;
    constraint.follower       = {follower_view};
    constraint.follower_flags = Node_ViewFlag_WITH_RECURSION;
    const Vec2 halignment     = constraint.leader.size() == 1 ? LEFT : CENTER;
    constraint.leader_pivot   = halignment + BOTTOM;
    constraint.follower_pivot = halignment + TOP;

    // vertical gap
    constraint.gap_size      = Size_MD;
    constraint.gap_direction = BOTTOM;

};

void Graph_View::_create_constraints__align_top_recursively(const std::vector<Node*>& unfiltered_follower, ndbl::Node* leader )
{
    if ( unfiltered_follower.empty() )
        return;

    ASSERT(leader);
    Node_View* leader_view = leader->component<Node_View>();
    // nodeview's inputs must be aligned on center-top
    // It's a one to many constrain.
    //
    std::vector<Node_View*> follower;
    for (auto* _follower : unfiltered_follower )
        if (node_is_output_node_in_expression(_follower, leader))
            follower.push_back(_follower->component<Node_View>() );

    if ( follower.empty() )
        return;

    auto& constraint = _m_contraints.emplace_back();
    constraint.name           = "Align many inputs above";
    constraint.rule           = &ViewConstraintRule_N_to_1_as_a_row;
    constraint.leader         = { leader_view };
    constraint.leader_pivot   = TOP;
    constraint.follower       = follower;
    constraint.follower_pivot = BOTTOM;
    constraint.gap_size       = Size_SM;
    constraint.gap_direction  = TOP;

    if (follower.size() > 1 )
    {
        constraint.follower_flags = Node_ViewFlag_WITH_RECURSION;
    }

    if ( leader->has_flow_adjacent() )
    {
        constraint.follower_pivot = BOTTOM_LEFT;
        constraint.leader_pivot   = TOP_RIGHT;
        constraint.row_direction  = RIGHT;
    }

    for( Node_View* _leader : follower )
        _create_constraints__align_top_recursively(_leader->node()->inputs(), _leader->node());
};

void Graph_View::_create_constraints(Scope* scope )
{
    // distribute child scopes
    if ( node_is_conditional(scope->node()) )
    {
        auto& constraint = _m_contraints.emplace_back();
        constraint.name          = "Align Scope_View partitions";
        constraint.rule          = &ViewConstraintRule_distribute_sub_scope_views;
        constraint.leader        = {scope->entity()->component<Node_View>()};
        constraint.leader_pivot  = BOTTOM;
        for(Branch i = 0; i < scope->node()->switch_behavior_data().branch_count(); ++i )
        {
            auto branch = scope->node()->switch_behavior_data().branch_out(i);
            Node_View* nodeview = branch->node->component<Node_View>();
            constraint.follower.push_back( nodeview );
        }
        constraint.gap_size      = Size_XL;
        constraint.gap_direction = BOTTOM;
    }

    std::vector<Node*> backbone = scope->backbone();
    for ( Node* child_node : backbone )
    {
        // align child below flow_inputs
        if ( child_node != backbone.front() || scope->is_orphan() )
            _create_constraints__align_down(child_node, child_node->flow_inputs());

        // align child's inputs above
        _create_constraints__align_top_recursively(child_node->inputs(), child_node );
    }

    for ( Node* _child_node : scope->children() )
        if ( Scope* _child_scope = _child_node->internal_scope() )
            _create_constraints(_child_scope);
};

void Graph_View::update(float dt)
{
    // Determines how many times update should be called
    ASSERT( dt >= 0.f);
    u16_t sample_count = (u16_t)(dt * get_config()->ui_node_physics_frequency);
    if ( sample_count == 0 ) // When frame rate is too slow
        sample_count = 1;
    const float sample_dt = dt / float(sample_count);

    // Do the update(s)
    for(size_t i = 0; i < sample_count; ++i)
        _update_once(sample_dt);
}

void Graph_View::_update_once(float dt)
{
    ASSERT( graph() );

    // Physics Components
    // TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph_View", "Updating constraints ...\n");

    // Reset constraints when necessary
    if ( _m_physics_dirty )
    {
        _m_contraints.clear();
        _create_constraints(graph()->root_scope());

        _m_physics_dirty = false;
    }

    // Apply contraints (constraints => forces)
    // Note: This could run in parallel.
    for (ViewConstraint& constraint : _m_contraints)
        if( constraint.rule ) // is 0-initialized
            constraint.rule(&constraint, dt);

    // Apply forces (forces => positons)
    for ( Node* node : graph()->nodes() )
        if ( auto* _physics = node->component<Physics_Component>() )
            _physics->apply_forces(dt);

    // TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph_View", "Constraints updated.\n");

    // Node_Views
    for (Node* node : graph()->nodes() )
        if ( auto* view = node->component<Node_View>() )
            view->update(dt);

    // Scope_Views
    if( Node* root = graph()->root_node() )
        if ( auto* view = root->component<Scope_View>())
            view->update( dt, Scope_View_Flag_RECURSE );
}

void Graph_View::_update_until_unfold()
{
    const Config* cfg = get_config();

    // Compute the number of update necessary to simulate unfolding for dt seconds
    const u32_t dt      = cfg->graph_view_unfold_duration;
    const u32_t samples = 1000 * dt / cfg->tools_cfg->dt_cap;

    // Run the updates
    ASSERT(samples != 0 );
    auto sample_dt = float(dt) / samples;
    ASSERT(sample_dt > 0.f );

    for(u32_t i = 0; i < samples; ++i)
        _update_once( sample_dt );
}

void Graph_View::add_action_to_node_menu(Action_CreateNode* _action )
{
    _m_create_node_menu.add_action(_action);
}

void Graph_View::frame_content(Frame_Mode mode )
{
    auto frame_root_node_view = [&]() {
        // Get root node view
        Scope* root_scope = graph()->root_scope();
        if ( !root_scope ) return;
        auto root_node_view = root_scope->node()->component<Node_View>();
        ASSERT(root_node_view);

        // compute the delta to apply
        const Vec2 margin(40.f);
        const Vec2 target = margin + _m_shape.pivot( tools::TOP_LEFT, WORLD_SPACE);
        const Vec2 origin = root_node_view->shape()->pivot( tools::TOP_LEFT, WORLD_SPACE);
        const Vec2 delta = target - origin;

        // apply the delta
        root_node_view->translate( delta );
    };

    if ( mode ==  Frame_Mode::Root_Node_View )
        return frame_root_node_view();

    // Get selected node views rectangle
    auto selected_nodeviews = _m_selection.collect<Node_View*>();
    if ( selected_nodeviews.empty() )
        return frame_root_node_view(); // by default, we frame the root node

    const Rect rect = Node_View::bounding_rect( selected_nodeviews, tools::WORLD_SPACE);

    // compute the delta to apply
    const Vec2 target = _m_shape.pivot( tools::CENTER, tools::WORLD_SPACE);
    const Vec2 source = Box_2D(rect).pivot(tools::CENTER, WORLD_SPACE);
    const Vec2 delta =  target - source;

    // apply the delta to all node views
    for (Node* node : graph()->nodes() )
        if ( Node_View* view = node->component<Node_View>() )
            view->spatial_node()->translate( delta );
}

void Graph_View::_on_graph_change()
{
    _m_physics_dirty = true;
}

void Graph_View::_on_selection_change(Selection::Event_Type type, Selection::Element elem)
{
    bool selected = type == Selection::Event_Type::Append;

    switch ( elem.index() )
    {
        case Selectable::index_of<Scope_View*>():
        {
            elem.get<Scope_View*>()->state()->set_selected(selected );
            break;
        }
        case Selectable::index_of<Node_View*>():
        {
            elem.get<Node_View*>()->state()->set_selected(selected );
            break;
        }
        case Selectable::index_of<Node_Slot_Link_View>():
        {
            break;
        }
        default:
        {
            ASSERT(false); // unhandled case
        }
    }
}

void Graph_View::reset()
{
    if ( graph()->is_empty() )
        return;

    _update_until_unfold(); // Otherwise it would not render a nice graph when nodes are rendered for the first time

    // make sure views are outside viewable rectangle (to avoid flickering)
    Vec2 far_outside = Vec2(-1000.f, -1000.0f);

    for( Node* node : graph()->nodes() )
        if ( auto* view = node->component<Node_View>() )
            view->spatial_node()->translate( far_outside );

    // physics
    _m_physics_dirty = true;

    //   Note: Instead of waiting an arbitrary period of time, we should rather be able to unfold the graph instantly
    const size_t dispatch_delay_ms = 100;
    get_event_manager()->dispatch_delayed<Event_FrameSelection>( dispatch_delay_ms, { Frame_Mode::Root_Node_View } );
}

bool Graph_View::has_an_active_tool() const
{
    return !_m_state_machine.has_default_state();
}

void Graph_View::reset_all_properties()
{
    for( Node* node : graph()->nodes() )
        if ( Node_View* v = node->component<Node_View>() )
            v->reset_all_properties();
}

//-----------------------------------------------------------------------------

void Graph_View::_draw_create_node_context_menu(Node_View_Contextual_Menu& menu, Node_Slot_View* dragged_slotview)
{
    if (Action_CreateNode* triggered_action = menu.draw_search_input( dragged_slotview, 10))
    {
        // Generate an event from this action, add some info to the state and dispatch it.
        auto event                     = triggered_action->make_event();
        event->data.graph              = graph();
        event->data.active_slotview    = dragged_slotview;
        event->data.desired_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        get_event_manager()->dispatch(event);
        ImGui::CloseCurrentPopup();
    }
}

//-----------------------------------------------------------------------------

void Graph_View::drag_state_enter()
{
    for( const Selectable& elem : _m_selection )
    {
        if ( auto* nodeview = elem.get_if<Node_View*>() )
            nodeview->state()->set_pinned();
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
            scopeview->state()->set_pinned();
    }
}

void Graph_View::drag_state_tick()
{
    const Vec2 delta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    for ( const Selectable& elem : _m_selection )
    {
        auto* nodeview = elem.get_if<Node_View*>();

        if ( nodeview )
        {
            nodeview->translate(delta);
            nodeview->state()->set_pinned();
        }
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
        {
            nodeview = scopeview->node()->component<Node_View>();
            nodeview->translate(delta);
            nodeview->state()->set_pinned();
        }
    }

    if ( ImGui::IsMouseReleased(0) )
        _m_state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void Graph_View::view_pan_state_tick()
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graphview content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for( Node* node : graph()->nodes() )
        if ( auto v = node->component<Node_View>() )
            v->spatial_node()->translate(delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        _m_state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void Graph_View::cursor_state_tick()
{
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            _m_create_node_menu.flag_to_be_reset();

        switch ( _m_focused.index() )
        {
            case Selectable::index_null:
            {
                _draw_create_node_context_menu(_m_create_node_menu);
                break;
            }

            case Selectable::index_of<Scope_View*>():
            {
                auto* scopeview = _m_focused.get<Scope_View*>();
                auto* nodeview = scopeview->node()->component<Node_View>();
                if ( ImGui::MenuItem( nodeview->expanded() ? "Collapse Scope" : "Expand Scope" ) )
                {
                    nodeview->expand_toggle_rec();
                }

                if ( ImGui::MenuItem("Delete Scope") )
                {
                    auto event = new Event_DeleteSelection({scopeview->node()});
                    get_event_manager()->dispatch(event);
                }

                if ( ImGui::MenuItem("Select Scope") )
                {
                    // Get descendent scopes
                    std::set<Scope*> children;
                    Scope::get_descendent(children, scopeview->scope(), Scope_Flag_INCLUDE_SELF );

                    // Extract node views from each descendent
                    std::vector<Node_View*> views;
                    for(Scope* child : children)
                    {
                        // Include scope owner's view too
                        if ( auto* view = child->entity()->component<Node_View>())
                            views.push_back( view );

                        // and every other child's
                        for( Node* child_node : child->backbone() )
                            if ( auto* view = child_node->component<Node_View>())
                                views.push_back(view);
                    }
                    // Replace selection
                    _m_selection.clear();
                    _m_selection.append( views.begin(), views.end() ) ;
                }

                ImGui::Separator();
                _draw_create_node_context_menu(_m_create_node_menu);

                break;
            }

            case Selectable::index_of<Node_Slot_Link_View>():
            {
                auto edge = _m_focused.get<Node_Slot_Link_View>();
                if ( ImGui::MenuItem(ICON_FA_TRASH " Delete Edge") )
                {
                    auto* event = new Event_DeleteEdge();
                    event->data.first  = edge.head->slot;
                    event->data.second = edge.tail->slot;
                    get_event_manager()->dispatch( event );
                }

                break;
            }

            case Selectable::index_of<Node_Slot_View*>():
            {
                if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect Edges") )
                {
                    auto* event = new Event_Node_SlotDisconnectAll();
                    event->data.first = _m_focused.get<Node_Slot_View*>()->slot;
                    get_event_manager()->dispatch( event );
                }

                break;
            }

            case Selectable::index_of<Node_View*>():
            {
                auto nodeview = _m_focused.get<Node_View*>();

                if ( ImGui::MenuItem(ICON_FA_TRASH " Delete Node") )
                {
                    auto* event = new Event_DeleteSelection ();
                    event->data.node = nodeview->node();
                    get_event_manager()->dispatch( event );
                }

                if ( ImGui::MenuItem(ICON_FA_MAP_PIN " Pin/Unpin Node") )
                {
                    const bool pinned = nodeview->state()->pinned();
                    nodeview->state()->set_pinned( !pinned );
                }

                if ( ImGui::MenuItem(ICON_FA_WINDOW_RESTORE " Arrange Node") )
                {
                    nodeview->arrange_recursively();
                }

                break;
            }
        }

        ImGui::EndPopup();

        // When we're focused on something with popup open, we don't want to do things based on _m_hovered.type (see below)
        if ( !_m_focused.empty() )
            return;
    }

    switch ( _m_hovered.index() )
    {
        case Selectable::index_of<Node_Slot_View*>():
        {
            if ( ImGui::IsMouseClicked(1) )
            {
                _m_focused = _m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if (ImGui::IsMouseDragging(0, 0.f))
            {
                _m_focused = _m_hovered;
                _m_state_machine.change_state(LINE_STATE);
            }
            break;
        }

        case Selectable::index_of<Node_Slot_Link_View>():
        {
            if (ImGui::IsMouseDragging(0, 0.1f))
            {
                _m_focused = _m_hovered;
            }
            else if (ImGui::IsMouseClicked(1))
            {
                _m_focused = _m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            break;
        }

        case Selectable::index_of<Node_View*>():
        case Selectable::index_of<Scope_View*>():
        {
            const bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);

            if ( ImGui::IsMouseReleased(0) )
            {
                if ( ctrl_pressed )
                {
                    if ( !_m_selection.contains( _m_hovered ) )
                    {
                        _m_selection.append( _m_hovered );
                        _m_focused = _m_hovered;
                    }
                    else
                    {
                        _m_selection.remove( _m_hovered );
                    }
                }
                else
                {
                    _m_selection.clear();
                    _m_selection.append( _m_hovered );
                    _m_focused = _m_hovered;
                }
            }
            else if (ImGui::IsMouseClicked(1))
            {
                _m_focused = _m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if ( ImGui::IsMouseDragging(0) )
            {
                if ( !_m_selection.contains( _m_hovered) )
                {
                    if ( !ctrl_pressed )
                        _m_selection.clear();
                    _m_selection.append( _m_hovered );
                }
                _m_state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case Selectable::index_null:
        {
            if ( ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
            {
                if (ImGui::IsMouseClicked(0))
                    _m_selection.clear(); // Deselect All (Click on the background)
                else if (ImGui::IsMouseReleased(0))
                    _m_focused = {};
                else if (ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup(CONTEXT_POPUP);
                else if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::IsKeyDown(ImGuiKey_Space))
                        _m_state_machine.change_state(VIEW_PAN_STATE);
                    else
                        _m_state_machine.change_state(ROI_STATE);
                }
            }

            break;
        }

        default:
            VERIFY(false, "Unhandled case, must be implemented!");
    }
}

//-----------------------------------------------------------------------------

void Graph_View::line_state_enter()
{
    ASSERT( _m_focused.holds_alternative<Node_Slot_View*>() );
}

void Graph_View::line_state_tick()
{
    Vec2 mouse_pos_snapped = Vec2{ImGui::GetMousePos()};
    if ( auto slotview = _m_hovered.get_if<Node_Slot_View*>() )
    {
        mouse_pos_snapped = slotview->spatial_node()->position(WORLD_SPACE);
    }

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            _m_create_node_menu.flag_to_be_reset();

        if ( _m_hovered.empty() )
            _draw_create_node_context_menu(_m_create_node_menu, _m_focused.get<Node_Slot_View*>() );

        if ( ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
            _m_state_machine.exit_state();

        ImGui::EndPopup();
    }
    else if ( ImGui::IsMouseReleased(0) )
    {
        if ( _m_hovered.holds_alternative<Node_Slot_View*>() )
        {
            if ( _m_focused != _m_hovered )
            {
                auto event = new Event_Node_SlotDropped();
                event->data.first  = _m_focused.get<Node_Slot_View*>()->slot;
                event->data.second = _m_hovered.get<Node_Slot_View*>()->slot;
                get_event_manager()->dispatch(event);
                _m_state_machine.exit_state();
            }
        }
        else
        {
            ImGui::OpenPopup(CONTEXT_POPUP);
        }
    }

    // Draw a temporary wire from focused/dragged slotview to the mouse cursor
    Graph_View::draw_wire_from_slot_to_pos(_m_focused.get<Node_Slot_View*>(), mouse_pos_snapped );
}

void Graph_View::line_state_leave()
{
    _m_focused = {};
}

//-----------------------------------------------------------------------------

void Graph_View::roi_state_enter()
{
    _m_state_roi_start_pos = ImGui::GetMousePos();
    _m_state_roi_end_pos   = ImGui::GetMousePos();;
}

void Graph_View::roi_state_tick()
{
    _m_state_roi_end_pos = ImGui::GetMousePos();

    // Get normalized ROI rectangle
    Rect roi = Rect::normalize({_m_state_roi_start_pos, _m_state_roi_end_pos});

    // Expand to avoid null area
    const int roi_border_width = 2;
    roi.expand(Vec2{roi_border_width*0.5f});

    // Draw the ROI rectangle
    float time = ImGui::GetTime();
    float alpha = wave(0.5f, 0.75f, time, 10.f);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll , roi_border_width );

    if (ImGui::IsMouseReleased(0))
    {
        // Get the views included in the ROI
        std::set<Node_View*> nodeviews_inside_roi;
        for ( Node* node : graph()->nodes() )
            if ( auto view = node->component<Node_View>() )
                if ( Rect::contains(roi, view->get_rect()) )
                    nodeviews_inside_roi.insert( view );

        // Select them
        const bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        if ( !ctrl_pressed )
            _m_selection.clear();
        _m_selection.append(nodeviews_inside_roi.begin(), nodeviews_inside_roi.end() );

        _m_state_machine.exit_state();
    }
}

void Graph_View::_handle_hover(Scope_View* scope_view)
{
    if ( !_m_hovered.holds_alternative<Scope_View*>() )
        _m_hovered = scope_view;
    else if ( _m_hovered.empty() )
        _m_hovered = scope_view;
    else if ( scope_view->depth() >= _m_hovered.get<Scope_View*>()->depth() )
        _m_hovered = scope_view;
}

std::vector<Node_View*> get_clean_views(std::vector<Node_View*>& possibly_hidden_views)
{
    std::vector<Node_View*> result;
    for(Node_View* view : possibly_hidden_views)
        if (view->state()->visible())
            if (!view->state()->pinned())
                result.push_back(view);
    return std::move(result);
}

void ndbl::ViewConstraintRule_1_to_N_as_row(ViewConstraint* constraint, float dt)
{
    // This type of constrain is designed to make a single Node_View to follow many others

    VERIFY(!constraint->leader.empty(), "No leader found!");
    VERIFY(constraint->follower.size() == 1, "This is a one to many relationship, a single follower only is allowed");

    std::vector<Node_View*> clean_follower = get_clean_views(constraint->follower);
    if( clean_follower.empty() )
        return;

    Config* cfg = get_config();
    const Node_View* _follower      = clean_follower[0];
    const Box_2D leaders_box{Node_View::bounding_rect(constraint->leader, WORLD_SPACE, constraint->leader_flags) };
    const Box_2D follower_box{ _follower->get_rect_ex(WORLD_SPACE, constraint->follower_flags) };

    // Compute how much the follower box needs to be moved to snap the leader's box at a given pivots.
    Vec2 delta = Box_2D::diff(leaders_box, constraint->leader_pivot , follower_box, constraint->follower_pivot );
    delta += constraint->gap_direction * cfg->ui_node_gap(constraint->gap_size);

    // Apply a force to translate to the (single) follower
    Vec2 current_pos = _follower->spatial_node()->position(WORLD_SPACE);
    Vec2 desired_pos = current_pos + delta;
    auto* physics_component = _follower->node()->component<Physics_Component>();
    VERIFY(physics_component, "Component required");
    physics_component->translate_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
}

void ndbl::ViewConstraintRule_N_to_1_as_a_row(ViewConstraint* constraint, float _dt)
{
    ASSERT(constraint->leader.size() == 1);
    ASSERT(constraint->follower.size() > 0);

    Config* cfg = get_config();
    std::vector<Node_View*> clean_follower = get_clean_views(constraint->follower);
    if( clean_follower.empty() )
        return;

    // Form a row with each view box
    std::vector<Box_2D> box(constraint->follower.size());
    std::vector<Vec2>       delta(constraint->follower.size());
    const Vec2              gap = cfg->ui_node_gap(constraint->gap_size);

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        box[i] = Box_2D{ clean_follower[i]->get_rect_ex(WORLD_SPACE, constraint->follower_flags) };

        // Determine the delta required to snap the current follower with either the leaders or the previous follower.
        if ( i == 0 )
        {
            // First box is aligned with the leader
            const Box_2D leader_box{ constraint->leader[0]->get_rect_ex(WORLD_SPACE, constraint->leader_flags) };
            delta[i] = Box_2D::diff(leader_box, constraint->leader_pivot, box[i], constraint->follower_pivot);
            delta[i] += gap * constraint->gap_direction;
        }
        else
        {
            // i+1 box is aligned with the i
            delta[i] = Box_2D::diff(box[i - 1] , constraint->row_direction, box[i], -constraint->row_direction);
            delta[i] += gap * constraint->row_direction;
            delta[i] -= delta[i-1]; //
        }
    }

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        auto* physics_component = clean_follower[i]->node()->component<Physics_Component>();
        if( !physics_component )
            continue;
        Vec2 current_pos = clean_follower[i]->spatial_node()->position(WORLD_SPACE);
        Vec2 desired_pos = current_pos + delta[i];
        physics_component->translate_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
    }
}

void ndbl::ViewConstraintRule_distribute_sub_scope_views(ViewConstraint* constraint, float dt)
{
    // filter views to constrain
    //
    // TODO: there is an issue here, due to the specific case of Scope being partitions (sharing the same node with
    //       their parent scope), it is complicated to disable the constraints when the partition contains a single
    //       nested scope (e.g. in a while/if/for/etc.).
    //       The concept of partition should be removed. They must be either dynamically added/removed when user
    //       connects a node to a branch, or they must be attached to a separate node.
    //
    std::vector<Scope_View*> sub_scope_view;
    for( Node_View* _follower : constraint->follower )
    {
        Scope_View* _follower_scopeview = _follower->internal_scopeview();
        ASSERT(_follower_scopeview);
        if ( !_follower_scopeview->pinned() )
            if ( _follower_scopeview->must_be_draw() )
                sub_scope_view.push_back( _follower_scopeview );
    }

    // get all content rects
    std::vector<Rect> new_content_rect;
    for(auto _view : sub_scope_view)
        new_content_rect.push_back( _view->content_rect() );

    // make a row
    const float gap = get_config()->ui_scope_gap( constraint->gap_size );
    Rect::make_row(new_content_rect, gap );

    // v align
    const Vec2 align_pos = constraint->leader[0]->shape()->pivot(constraint->leader_pivot, WORLD_SPACE )
                         + Vec2{0.f, gap} * constraint->gap_direction;
    Rect::align_top(new_content_rect, align_pos.y );

    // h align
    Rect::center(new_content_rect, align_pos.x );

    // translate each sub_scope
    for(size_t i = 0; i < sub_scope_view.size(); ++i)
    {
        const Vec2 cur_pos = sub_scope_view[i]->content_rect().center();
        const Vec2 new_pos = new_content_rect[i].center();
        const Vec2 delta = new_pos - cur_pos;

        // Apply force to translate head
        auto* physics = sub_scope_view[i]->scope()->head()->component<Physics_Component>();
        VERIFY(physics, "A Physics_Component is required on this entity to apply a force to");
        physics->translate(delta, get_config()->ui_node_speed, true );
    }
}
