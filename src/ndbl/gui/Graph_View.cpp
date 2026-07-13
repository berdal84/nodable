#include "Graph_View.h"

#include <algorithm>
#include "core/Component.h"
#include "core/Scope.h"
#include "gui/View_State.h"
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

ndbl::Graph_View::Graph_View()
: Component<Graph>("View")
, state_machine(this)
, shape( Vec2{100.f, 100.f} ) // non null area
{
    Component::signal_init.connect<&graphview_handle_init>(this);
    Component::signal_shutdown.connect<&graphview_handle_shutdown>(this);

    state_machine.add_state(CURSOR_STATE);
    state_machine.bind<&graphview_cursor_state_tick>(CURSOR_STATE, When::OnTick);
    state_machine.set_default_state(CURSOR_STATE);

    state_machine.add_state(ROI_STATE);
    state_machine.bind<&graphview_roi_state_enter>(ROI_STATE, When::OnEnter);
    state_machine.bind<&graphview_roi_state_tick>(ROI_STATE, When::OnTick);

    state_machine.add_state(DRAG_STATE);
    state_machine.bind<&graphview_drag_state_enter>(DRAG_STATE, When::OnEnter);
    state_machine.bind<&graphview_drag_state_tick>(DRAG_STATE, When::OnTick);

    state_machine.add_state(VIEW_PAN_STATE);
    state_machine.bind<&graphview_view_pan_state_tick>(VIEW_PAN_STATE, When::OnTick);

    state_machine.add_state(LINE_STATE);
    state_machine.bind<&graphview_line_state_enter>(LINE_STATE, When::OnEnter);
    state_machine.bind<&graphview_line_state_tick>(LINE_STATE, When::OnTick);
    state_machine.bind<&graphview_line_state_leave>(LINE_STATE, When::OnLeave);
}

ndbl::Graph_View::~Graph_View()
{
    Component::signal_init.disconnect(); // Simple_Signal
    Component::signal_shutdown.disconnect();
}

void ndbl::graphview_handle_init(Graph_View* graph_view)
{
    // add nodes present before connecting signals
    for( Node* each_node : graph_view->graph()->nodes )
    {
        graphview_handle_add_node(graph_view, each_node);
    }

    graph_view->selection.signal_change.connect<&graphview_on_selection_change>(graph_view);
    graph_view->graph()->signal_change.connect<&graphview_on_graph_change>(graph_view);
    graph_view->graph()->signal_add_node.connect<&graphview_handle_add_node>(graph_view);
    graph_view->graph()->signal_remove_node.connect<&graphview_handle_remove_node>(graph_view);
    graph_view->graph()->signal_change_scope.connect<&graphview_handle_change_scope>(graph_view);
    graph_view->graph()->signal_reset.connect<&graphview_reset>(graph_view);
    graph_view->graph()->signal_is_complete.connect<&graphview_reset>(graph_view);

    graph_view->state_machine.start();
}

void ndbl::graphview_handle_shutdown(Graph_View* graph_view)
{
    graph_view->state_machine.stop();

    graph_view->selection.signal_change.disconnect();
    graph_view->graph()->signal_add_node.disconnect();
    graph_view->graph()->signal_remove_node.disconnect();
    graph_view->graph()->signal_reset.disconnect();
    graph_view->graph()->signal_is_complete.disconnect();
    ASSERT_DEBUG_ONLY( graph_view->graph()->signal_change.disconnect<&graphview_on_graph_change>(graph_view) );

    // add nodes still present after connecting signals
    for( Node* each_node : graph_view->graph()->nodes )
    {
        graphview_handle_remove_node(graph_view, each_node);
    }
}

void ndbl::graphview_handle_add_node(Graph_View* graph_view, Node* node)
{
    // view
    auto* nodeview = new Node_View();
    component_init(nodeview, node);
    nodeview->shape.set_size({20.f, 35.f});

    componentbag_add(& node->component_bag, nodeview);

    if (Scope_View* scopeview = nodeview->internal_scopeview )
        scopeview->signal_hover.connect<&graphview_handle_hover>(graph_view); // I'm not sure if this is a good approach...

    if( node == graph_root(graph_view->graph()) )
    {
        // root must be parented to the graph view itself
        graph_view->spatial_node()->add_child( nodeview->spatial_node() );
    }
    else
    {
        node->scope->view->spatial_node.add_child( nodeview->spatial_node() );
    }

    // physics
    auto* physics_component = new Physics_Component();
    component_init(physics_component, node);
    componentbag_add(&node->component_bag, physics_component);
}

void ndbl::graphview_handle_remove_node(Graph_View* graph_view, Node* node)
{
    // clean physics
    auto* physics_component = componentbag_get<Physics_Component>(&node->component_bag);
    VERIFY(physics_component, "Should have been created from _handle_add_node()");
    componentbag_remove(&node->component_bag, physics_component );
    component_shutdown(physics_component);
    delete physics_component;

    // clean nodeview
    auto* nodeview = componentbag_get<Node_View>(&node->component_bag);
    VERIFY(nodeview, "Should have been created from _handle_add_node()");

    if ( Scope_View* scopeview = nodeview->internal_scopeview )
    {
        scopeview->signal_hover.disconnect(); // I'm not sure if this is a good approach...
    }

    if( Spatial_Node* _parent = nodeview->spatial_node()->parent() )
    {
        _parent->remove_child( nodeview->spatial_node() );
    }

    componentbag_remove( &node->component_bag, nodeview );
    component_shutdown(nodeview);
    delete nodeview;
}

void ndbl::graphview_handle_change_scope(Graph_View* graph_view, Graph::Scope_Change change)
{
    auto* nodeview = componentbag_get<Node_View>(&change.node->component_bag);
    VERIFY(nodeview, "a nodeview must be present since we are in a Graph_View");

    // Un-parent from old scope's spatial node
    if( auto _parent = nodeview->spatial_node()->parent() )
        _parent->remove_child( nodeview->spatial_node() );

    // Parent to new scope or default to graph's spatial node
    if( Scope_View* _scopeview = change.new_scope->view )
        _scopeview->spatial_node.add_child( nodeview->spatial_node() );
}

ImGuiID make_wire_id(const Node_Slot *ptr1, const Node_Slot *ptr2)
{
    String_128 id;
    id.append_fmt("wire %zu->%zu", ptr1, ptr2);
    return ImGui::GetID(id.c_str());
}

void ndbl::graphview_draw_wire_from_slot_to_pos(Graph_View*, Node_Slot_View *from, const Vec2 &end_pos)
{
    VERIFY(from != nullptr, "from slot can't be nullptr");

    Config* cfg = get_config();

    // Style

    ImGuiEx::WireStyle style;
    style.shadow_color = cfg->ui_codeflow_shadowColor,
    style.roundness    = 0.f;

    if (from->slot->type() == Node_Slot::Flag_TYPE_FLOW) {
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

bool ndbl::graphview_draw(Graph_View* graph_view, float dt)
{
    bool changed = false;

    // Ensure view state fit with content region
    // (n.b. we could also implement a struct RootView_State wrapping View_State)
    Rect region = ImGuiEx::GetContentRegion(WORLD_SPACE);
    graph_view->shape.set_size( region.size() );
    graph_view->shape.set_position(region.center()); // children will be relative to the center
    graph_view->shape.draw_debug_info();

    graph_view->hovered = {};

    Config*         cfg       = get_config();
    ImDrawList*     draw_list = ImGui::GetWindowDrawList();

    // Draw Scopes
    std::vector<Scope*> scopes_to_draw = graph_collect_scopes(graph_view->graph());
    // TODO: we should sort them only when a new parent/child connection is created/deleted
    auto low_to_high_depth = [](Scope* s1, Scope* s2) { return scope_get_depth(s1) < scope_get_depth(s2); };
    std::sort(scopes_to_draw.begin(), scopes_to_draw.end(), low_to_high_depth);

    for( Scope* scope : scopes_to_draw )
        if (scope->view != nullptr)
            scopeview_draw( scope->view, dt);

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
    for (Node* each_node: graph_view->graph()->nodes )
    {
        Node_View *each_view = nodeview_substitute_with_parent_if_not_visible( componentbag_get<Node_View>(&each_node->component_bag) );

        if (!each_view) {
            continue;
        }

        std::vector<Node_Slot *> slots = node_filter_slots(each_node, Node_Slot::Flag_FLOW_OUT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index)
        {
            Node_Slot *slot = slots[slot_index];

            if (slot->adjacent.empty() )
            {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent)
            {
                Node*       each_successor_node     = adjacent_slot->node;
                Node_View*  possibly_hidden_view    = componentbag_get<Node_View>(&each_successor_node->component_bag);
                Node_View*  each_successor_view     = nodeview_substitute_with_parent_if_not_visible(possibly_hidden_view);

                if ( each_successor_view == nullptr )
                    continue;
                if ( !each_view->state.has_flags(View_Flag_VISIBLE) )
                    continue;
                if ( !each_successor_view->state.has_flags(View_Flag_VISIBLE) )
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
                    graph_view->hovered = Node_Slot_Link_View{tail, head};
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
    for (Node* node_out: graph_view->graph()->nodes )
    {
        for (const Node_Slot* slot_out: node_filter_slots(node_out, Node_Slot::Flag_OUTPUT))
        {
            for(const Node_Slot* slot_in : slot_out->adjacent)
            {
                if (slot_in == nullptr)
                    continue;

                auto *node_view_out = componentbag_get<Node_View>(&slot_out->node->component_bag);
                auto *node_view_in  = componentbag_get<Node_View>(&slot_in->node->component_bag);

                if ( !node_view_out->state.has_flags(View_Flag_VISIBLE) )
                    continue;
                if ( !node_view_in->state.has_flags(View_Flag_VISIBLE) )
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
                if ( graph_view->selection.contains( node_view_out ) || graph_view->selection.contains( node_view_in ) )
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
                if (node_out->type == Node_Type_VARIABLE ) // from a variable
                {
                    if (slot_out == node_out->variable_data.ref_out ) // from a reference slot (can't be a declaration link)
                        if (!node_view_out->state.has_flags(View_Flag_SELECTED) && !node_view_in->state.has_flags(View_Flag_SELECTED) )
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
                        graph_view->hovered = Node_Slot_Link_View{slot_view_out, slot_view_in};
                    }
                }
            }
        }
    }

    // Draw Node_Views
    for (Node* node : graph_view->graph()->nodes  )
    {
        Node_View* nodeview = componentbag_get<Node_View>(&node->component_bag);

        if ( !nodeview)
            continue;
        if ( !nodeview->state.has_flags(View_Flag_VISIBLE) )
            continue;

        changed |= nodeview_draw(nodeview);

        if ( nodeview->state.has_flags(View_Flag_HOVERED) ) // no check if something else is hovered, last node always win against an edge
        {
            if ( nodeview->hovered_slotview != nullptr)
            {
                graph_view->hovered = nodeview->hovered_slotview;
            }
            else
            {
                graph_view->hovered = {nodeview};
            }
        }
    }

    graph_view->state_machine.tick();

    // Debug Infos
    if (cfg->tools_cfg->runtime_debug)
    {
        if (ImGui::Begin("Graph_ViewToolState_Machine"))
        {
            ImGui::Text("current_tool:         %s"  , graph_view->state_machine.get_current_state_name());
            ImGui::Text("_m_focused.type:       %zu", graph_view->focused.index() );
            ImGui::Text("_m_hovered.type:       %zu", graph_view->hovered.index() );
            Vec2 mouse_pos = ImGui::GetMousePos();
            ImGui::Text("_m_mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
        }
        ImGui::End();
    }

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    if ( changed )
        graph_view->signal_change.emit();

	return changed;
}

void ndbl::graphview_create_constraints__align_down(Graph_View* graph_view, Node* follower, const  std::vector<Node*>& leader )
{
    if( leader.empty() )
        return;

    std::vector<Node_View*> leader_view;

    for ( Node* _leader : leader )
    {
        leader_view.push_back( componentbag_get<Node_View>(&_leader->component_bag) );
    }

    Node_View* follower_view = componentbag_get<Node_View>(&follower->component_bag);

    auto& constraint = graph_view->contraints.emplace_back();

    constraint.name           = "Position below previous";
    constraint.rule           = &nodeviewcontraint_rule_1_to_N_as_row;
    constraint.leader         = leader_view;
    constraint.follower       = {follower_view};
    constraint.follower_flags = Node_View_Flag_WITH_RECURSION;
    const Vec2 halignment     = constraint.leader.size() == 1 ? LEFT : CENTER;
    constraint.leader_pivot   = halignment + BOTTOM;
    constraint.follower_pivot = halignment + TOP;

    // vertical gap
    constraint.gap_size      = Size_MD;
    constraint.gap_direction = BOTTOM;

};

void ndbl::graphview_create_constraints__align_top_recursively(Graph_View* graph_view, const std::vector<Node*>& unfiltered_follower, ndbl::Node* leader )
{
    if ( unfiltered_follower.empty() )
        return;

    ASSERT(leader);
    Node_View* leader_view = componentbag_get<Node_View>(&leader->component_bag);
    // nodeview's inputs must be aligned on center-top
    // It's a one to many constrain.
    //
    std::vector<Node_View*> follower;
    for (auto* _follower : unfiltered_follower )
        if (node_is_output_node_in_expression(_follower, leader))
            follower.push_back( componentbag_get<Node_View>(&_follower->component_bag) );

    if ( follower.empty() )
        return;

    auto& constraint = graph_view->contraints.emplace_back();
    constraint.name           = "Align many inputs above";
    constraint.rule           = &nodeviewcontraint_rule_N_to_1_as_a_row;
    constraint.leader         = { leader_view };
    constraint.leader_pivot   = TOP;
    constraint.follower       = follower;
    constraint.follower_pivot = BOTTOM;
    constraint.gap_size       = Size_SM;
    constraint.gap_direction  = TOP;

    if (follower.size() > 1 )
    {
        constraint.follower_flags = Node_View_Flag_WITH_RECURSION;
    }

    if ( node_has_flow_adjacent(leader) )
    {
        constraint.follower_pivot = BOTTOM_LEFT;
        constraint.leader_pivot   = TOP_RIGHT;
        constraint.row_direction  = RIGHT;
    }

    for( Node_View* _leader : follower ) // TODO: _leader vs follower ??!
    {
        graphview_create_constraints__align_top_recursively(graph_view, _leader->node()->inputs(), _leader->node());
    }
};

void ndbl::graphview_create_constraints(Graph_View* graph_view, Scope* scope )
{
    // distribute child scopes
    if ( node_is_conditional(scope->node()) )
    {
        auto& constraint = graph_view->contraints.emplace_back();
        constraint.name          = "Align Scope_View partitions";
        constraint.rule          = &nodeviewcontraint_rule_distribute_sub_scope_views;
        constraint.leader        = {componentbag_get<Node_View>(&scope->node()->component_bag)};
        constraint.leader_pivot  = BOTTOM;
        for(Branch i = 0; i < scope->node()->switch_data.branch_count(); ++i )
        {
            auto branch = scope->node()->switch_data.branch_out(i);
            Node_View* nodeview = componentbag_get<Node_View>(&branch->node->component_bag);
            constraint.follower.push_back( nodeview );
        }
        constraint.gap_size      = Size_XL;
        constraint.gap_direction = BOTTOM;
    }

    std::vector<Node*> backbone = scope_get_backbone(scope);
    for ( Node* child_node : backbone )
    {
        // align child below flow_inputs
        if ( child_node != backbone.front() || scope_is_orphan(scope) )
            graphview_create_constraints__align_down(graph_view, child_node, child_node->flow_inputs());

        // align child's inputs above
        graphview_create_constraints__align_top_recursively(graph_view, child_node->inputs(), child_node );
    }

    for ( Node* _child_node : scope->children )
        if ( Scope* _child_scope = _child_node->internal_scope )
            graphview_create_constraints(graph_view, _child_scope);
};

void ndbl::graphview_update(Graph_View* graph_view, float dt)
{
    // Determines how many times update should be called
    ASSERT( dt >= 0.f);
    u16_t sample_count = (u16_t)(dt * get_config()->ui_node_physics_frequency);
    if ( sample_count == 0 ) // When frame rate is too slow
        sample_count = 1;
    const float sample_dt = dt / float(sample_count);

    // Do the update(s)
    for(size_t i = 0; i < sample_count; ++i)
        graphview_update_once(graph_view, sample_dt);
}

void ndbl::graphview_update_once(Graph_View* graph_view, float dt)
{
    ASSERT( graph_view->graph() );

    // Physics Components
    // TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph_View", "Updating constraints ...\n");

    // Reset constraints when necessary
    if ( graph_view->is_physics_dirty )
    {
        graph_view->contraints.clear();
        graphview_create_constraints(graph_view, graph_root_scope(graph_view->graph()));

        graph_view->is_physics_dirty = false;
    }

    // Apply contraints (constraints => forces)
    // Note: This could run in parallel.
    for ( Node_View_Constraint& each_constraint : graph_view->contraints)
        if( each_constraint.rule ) // is nullptr initialized
            each_constraint.rule(&each_constraint, dt);

    // Apply forces (forces => positons)
    for ( Node* node : graph_view->graph()->nodes )
        if ( auto* _physics = componentbag_get<Physics_Component>(&node->component_bag) )
            _physics->apply_forces(dt);

    // TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph_View", "Constraints updated.\n");

    // Node_Views
    for (Node* node : graph_view->graph()->nodes )
        if ( auto* view = componentbag_get<Node_View>(&node->component_bag) )
            nodeview_update(view, dt);

    // Scope_Views
    if( Scope* root = graph_root_scope(graph_view->graph()) )
        if ( root->view != nullptr )
            scopeview_update( root->view, dt, Scope_View_Flag_RECURSE );
}

void ndbl::graphview_update_until_unfold(Graph_View* graph_view)
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
        graphview_update_once(graph_view, sample_dt );
}

void ndbl::graphview_frame_content(Graph_View* graph_view, Frame_Mode mode )
{
    // Frame_Mode::Root_Node_View
    if ( mode ==  Frame_Mode::Root_Node_View || graph_view->selection.collect<Node_View*>().empty() )
    {
        // Get root node view
        Scope* root_scope = graph_root_scope(graph_view->graph());
        if ( !root_scope ) return;
        auto root_node_view = componentbag_get<Node_View>(&root_scope->node()->component_bag);
        ASSERT(root_node_view);

        // compute the delta to apply
        const Vec2 margin(40.f);
        const Vec2 target   = margin + graph_view->shape.pivot( tools::TOP_LEFT, WORLD_SPACE);
        const Vec2 origin   = root_node_view->shape.pivot( tools::TOP_LEFT, WORLD_SPACE);
        const Vec2 delta    = target - origin;

        // apply the delta
        root_node_view->spatial_node()->translate( delta );
        
        return;
    }

    // Frame_Mode::Selected_Node_Views

    // Get selected node views rectangle
    std::vector<Node_View*> selected_nodeviews = graph_view->selection.collect<Node_View*>();
    const Rect rect = nodeview_bounding_rect( selected_nodeviews, tools::WORLD_SPACE);

    // compute the delta to apply
    const Vec2 target = graph_view->shape.pivot( tools::CENTER, tools::WORLD_SPACE);
    const Vec2 source = Box_2D(rect).pivot(tools::CENTER, WORLD_SPACE);
    const Vec2 delta =  target - source;

    // apply the delta to all node views
    for (Node* node : graph_view->graph()->nodes )
        if ( Node_View* nodeview = componentbag_get<Node_View>(&node->component_bag) )
            nodeview->spatial_node()->translate( delta );
}

void ndbl::graphview_on_graph_change(Graph_View* graph_view)
{
    graph_view->is_physics_dirty = true;
}

void ndbl::graphview_on_selection_change(Graph_View* graph_view, Selection::Event_Type type, Selection::Element elem)
{
    bool selected = type == Selection::Event_Type::Append;

    switch ( elem.index() )
    {
        case Selectable::index_of<Scope_View*>():
        {
            elem.get<Scope_View*>()->state.set_flags(View_Flag_SELECTED, selected);
            break;
        }
        case Selectable::index_of<Node_View*>():
        {
            elem.get<Node_View*>()->state.set_flags(View_Flag_SELECTED, selected);
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

void ndbl::graphview_reset(Graph_View* graph_view)
{
    if ( graph_is_empty(graph_view->graph() ) )
        return;

    graphview_update_until_unfold(graph_view); // Otherwise it would not render a nice graph when nodes are rendered for the first time

    // make sure views are outside viewable rectangle (to avoid flickering)
    Vec2 far_outside = Vec2(-1000.f, -1000.0f);

    for( Node* node : graph_view->graph()->nodes )
        if ( auto* view = componentbag_get<Node_View>(&node->component_bag) )
            view->spatial_node()->translate( far_outside );

    // physics
    graph_view->is_physics_dirty = true;

    //   Note: Instead of waiting an arbitrary period of time, we should rather be able to unfold the graph instantly
    const size_t dispatch_delay_ms = 100;
    get_event_manager()->dispatch_delayed<Event_FrameSelection>( dispatch_delay_ms, { Frame_Mode::Root_Node_View } );
}

bool ndbl::graphview_has_an_active_tool(const Graph_View* graph_view)
{
    return !graph_view->state_machine.has_default_state();
}

void ndbl::graphview_reset_all_properties(Graph_View* graph_view)
{
    for( Node* node : graph_view->graph()->nodes )
        if ( Node_View* each_node_view = componentbag_get<Node_View>(&node->component_bag) )
            nodeview_reset_all_properties(each_node_view);
}

//-----------------------------------------------------------------------------

void ndbl::graphview_draw_context_menu(Graph_View* graph_view, Node_Slot_View* dragged_slotview)
{
    if (Action_CreateNode* triggered_action = graph_view->contextual_menu.draw_search_input( dragged_slotview, 10))
    {
        // Generate an event from this action, add some info to the state and dispatch it.
        auto event                     = triggered_action->make_event();
        event->data.graph              = graph_view->graph();
        event->data.active_slotview    = dragged_slotview;
        event->data.desired_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        get_event_manager()->dispatch(event);
        ImGui::CloseCurrentPopup();
    }
}

//-----------------------------------------------------------------------------

void ndbl::graphview_drag_state_enter(Graph_View* graph_view)
{
    for( const Selectable& elem : graph_view->selection )
    {
        if ( auto* nodeview = elem.get_if<Node_View*>() )
            nodeview->state.set_flags(View_Flag_PINNED);
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
            scopeview->state.set_flags(View_Flag_PINNED);
    }
}

void ndbl::graphview_drag_state_tick(Graph_View* graph_view)
{
    const Vec2 delta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    for ( const Selectable& elem : graph_view->selection )
    {
        auto* nodeview = elem.get_if<Node_View*>();

        if ( nodeview )
        {
            nodeview->spatial_node()->translate(delta);
            nodeview->state.set_flags(View_Flag_PINNED);
        }
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
        {
            nodeview = componentbag_get<Node_View>(&scopeview->scope->entity->component_bag);
            nodeview->spatial_node()->translate(delta);
            nodeview->state.set_flags(View_Flag_PINNED);
        }
    }

    if ( ImGui::IsMouseReleased(0) )
        graph_view->state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void ndbl::graphview_view_pan_state_tick(Graph_View* graph_view)
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graph_view content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for( Node* node : graph_view->graph()->nodes )
        if ( auto nodeview = componentbag_get<Node_View>(&node->component_bag) )
            nodeview->spatial_node()->translate(delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        graph_view->state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void ndbl::graphview_cursor_state_tick(Graph_View* graph_view)
{
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            graph_view->contextual_menu.flag_to_be_reset();

        switch ( graph_view->focused.index() )
        {
            case Selectable::index_null:
            {
                graphview_draw_context_menu(graph_view);
                break;
            }

            case Selectable::index_of<Scope_View*>():
            {
                auto* scopeview = graph_view->focused.get<Scope_View*>();
                auto* nodeview = componentbag_get<Node_View>(&scopeview->scope->node()->component_bag);
                if ( ImGui::MenuItem( nodeview->is_expanded ? "Collapse Scope" : "Expand Scope" ) )
                {
                    nodeview_expand_toggle_rec(nodeview);
                }

                if ( ImGui::MenuItem("Delete Scope") )
                {
                    auto event = new Event_DeleteSelection({scopeview->scope->node()});
                    get_event_manager()->dispatch(event);
                }

                if ( ImGui::MenuItem("Select Scope") )
                {
                    // Get descendent scopes
                    std::set<Scope*> children;
                    scope_get_descendent(children, scopeview->scope, Scope_Flag_INCLUDE_SELF );

                    // Extract node views from each descendent
                    std::vector<Node_View*> views;
                    for(Scope* child : children)
                    {
                        // Include scope owner's view too
                        if ( auto* nodeview = componentbag_get<Node_View>(&child->node()->component_bag))
                            views.push_back( nodeview );

                        // and every other child's
                        for( Node* child_node : scope_get_backbone(child ))
                            if ( auto* nodeview = componentbag_get<Node_View>(&child_node->component_bag))
                                views.push_back(nodeview);
                    }
                    // Replace selection
                    graph_view->selection.clear();
                    graph_view->selection.append( views.begin(), views.end() ) ;
                }

                ImGui::Separator();
                graphview_draw_context_menu(graph_view);

                break;
            }

            case Selectable::index_of<Node_Slot_Link_View>():
            {
                auto edge = graph_view->focused.get<Node_Slot_Link_View>();
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
                    event->data.first = graph_view->focused.get<Node_Slot_View*>()->slot;
                    get_event_manager()->dispatch( event );
                }

                break;
            }

            case Selectable::index_of<Node_View*>():
            {
                auto nodeview = graph_view->focused.get<Node_View*>();

                if ( ImGui::MenuItem(ICON_FA_TRASH " Delete Node") )
                {
                    auto* event = new Event_DeleteSelection ();
                    event->data.node = nodeview->node();
                    get_event_manager()->dispatch( event );
                }

                if ( ImGui::MenuItem(ICON_FA_MAP_PIN " Pin/Unpin Node") )
                {
                    const bool pinned = nodeview->state.has_flags(View_Flag_PINNED);
                    nodeview->state.set_flags(View_Flag_PINNED, !pinned );
                }

                if ( ImGui::MenuItem(ICON_FA_WINDOW_RESTORE " Arrange Node") )
                {
                    nodeview_arrange_recursively(nodeview);
                }

                break;
            }
        }

        ImGui::EndPopup();

        // When we're focused on something with popup open, we don't want to do things based on _m_hovered.type (see below)
        if ( !graph_view->focused.empty() )
            return;
    }

    switch ( graph_view->hovered.index() )
    {
        case Selectable::index_of<Node_Slot_View*>():
        {
            if ( ImGui::IsMouseClicked(1) )
            {
                graph_view->focused = graph_view->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if (ImGui::IsMouseDragging(0, 0.f))
            {
                graph_view->focused = graph_view->hovered;
                graph_view->state_machine.change_state(LINE_STATE);
            }
            break;
        }

        case Selectable::index_of<Node_Slot_Link_View>():
        {
            if (ImGui::IsMouseDragging(0, 0.1f))
            {
                graph_view->focused = graph_view->hovered;
            }
            else if (ImGui::IsMouseClicked(1))
            {
                graph_view->focused = graph_view->hovered;
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
                    if ( !graph_view->selection.contains( graph_view->hovered ) )
                    {
                        graph_view->selection.append( graph_view->hovered );
                        graph_view->focused = graph_view->hovered;
                    }
                    else
                    {
                        graph_view->selection.remove( graph_view->hovered );
                    }
                }
                else
                {
                    graph_view->selection.clear();
                    graph_view->selection.append( graph_view->hovered );
                    graph_view->focused = graph_view->hovered;
                }
            }
            else if (ImGui::IsMouseClicked(1))
            {
                graph_view->focused = graph_view->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if ( ImGui::IsMouseDragging(0) )
            {
                if ( !graph_view->selection.contains( graph_view->hovered) )
                {
                    if ( !ctrl_pressed )
                        graph_view->selection.clear();
                    graph_view->selection.append( graph_view->hovered );
                }
                graph_view->state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case Selectable::index_null:
        {
            if ( ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
            {
                if (ImGui::IsMouseClicked(0))
                    graph_view->selection.clear(); // Deselect All (Click on the background)
                else if (ImGui::IsMouseReleased(0))
                    graph_view->focused = {};
                else if (ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup(CONTEXT_POPUP);
                else if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::IsKeyDown(ImGuiKey_Space))
                        graph_view->state_machine.change_state(VIEW_PAN_STATE);
                    else
                        graph_view->state_machine.change_state(ROI_STATE);
                }
            }

            break;
        }

        default:
            VERIFY(false, "Unhandled case, must be implemented!");
    }
}

//-----------------------------------------------------------------------------

void ndbl::graphview_line_state_enter(Graph_View* graph_view)
{
    ASSERT( graph_view->focused.holds_alternative<Node_Slot_View*>() );
}

void ndbl::graphview_line_state_tick(Graph_View* graph_view)
{
    Vec2 mouse_pos_snapped = Vec2{ImGui::GetMousePos()};
    if ( auto slotview = graph_view->hovered.get_if<Node_Slot_View*>() )
    {
        mouse_pos_snapped = slotview->spatial_node()->position(WORLD_SPACE);
    }

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            graph_view->contextual_menu.flag_to_be_reset();

        if ( graph_view->hovered.empty() )
            graphview_draw_context_menu(graph_view, graph_view->focused.get<Node_Slot_View*>() );

        if ( ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
            graph_view->state_machine.exit_state();

        ImGui::EndPopup();
    }
    else if ( ImGui::IsMouseReleased(0) )
    {
        if ( graph_view->hovered.holds_alternative<Node_Slot_View*>() )
        {
            if ( graph_view->focused != graph_view->hovered )
            {
                auto event = new Event_Node_SlotDropped();
                event->data.first  = graph_view->focused.get<Node_Slot_View*>()->slot;
                event->data.second = graph_view->hovered.get<Node_Slot_View*>()->slot;
                get_event_manager()->dispatch(event);
                graph_view->state_machine.exit_state();
            }
        }
        else
        {
            ImGui::OpenPopup(CONTEXT_POPUP);
        }
    }

    // Draw a temporary wire from focused/dragged slotview to the mouse cursor
    ndbl::graphview_draw_wire_from_slot_to_pos(graph_view, graph_view->focused.get<Node_Slot_View*>(), mouse_pos_snapped );
}

void ndbl::graphview_line_state_leave(Graph_View* graph_view)
{
    graph_view->focused = {};
}

//-----------------------------------------------------------------------------

void ndbl::graphview_roi_state_enter(Graph_View* graph_view)
{
    graph_view->state_roi_start_pos = ImGui::GetMousePos();
    graph_view->state_roi_end_pos   = ImGui::GetMousePos();;
}

void ndbl::graphview_roi_state_tick(Graph_View* graph_view)
{
    graph_view->state_roi_end_pos = ImGui::GetMousePos();

    // Get normalized ROI rectangle
    Rect roi = Rect::normalize({graph_view->state_roi_start_pos, graph_view->state_roi_end_pos});

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
        for ( Node* node : graph_view->graph()->nodes )
            if ( auto nodeview = componentbag_get<Node_View>(&node->component_bag) )
                if ( Rect::contains(roi, nodeview_get_rect(nodeview)) )
                    nodeviews_inside_roi.insert( nodeview );

        // Select them
        const bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        if ( !ctrl_pressed )
            graph_view->selection.clear();
        graph_view->selection.append(nodeviews_inside_roi.begin(), nodeviews_inside_roi.end() );

        graph_view->state_machine.exit_state();
    }
}

void ndbl::graphview_handle_hover(Graph_View* graph_view, Scope_View* scope_view)
{
    if ( !graph_view->hovered.holds_alternative<Scope_View*>() )
        graph_view->hovered = scope_view;
    else if ( graph_view->hovered.empty() )
        graph_view->hovered = scope_view;
    else if ( scopeview_get_depth( scope_view ) >= scopeview_get_depth( graph_view->hovered.get<Scope_View*>() ) )
        graph_view->hovered = scope_view;
}

std::vector<Node_View*> get_clean_views(std::vector<Node_View*>& possibly_hidden_views)
{
    std::vector<Node_View*> result;
    for(Node_View* view : possibly_hidden_views)
        if (view->state.has_flags(View_Flag_VISIBLE))
            if (!view->state.has_flags(View_Flag_PINNED))
                result.push_back(view);
    return std::move(result);
}

void ndbl::nodeviewcontraint_rule_1_to_N_as_row(Node_View_Constraint* constraint, float dt)
{
    // This type of constrain is designed to make a single Node_View to follow many others

    VERIFY(!constraint->leader.empty(), "No leader found!");
    VERIFY(constraint->follower.size() == 1, "This is a one to many relationship, a single follower only is allowed");

    std::vector<Node_View*> clean_follower = get_clean_views(constraint->follower);
    if( clean_follower.empty() )
        return;

    Config* cfg = get_config();
    const Node_View* _follower      = clean_follower[0];
    const Box_2D leaders_box{nodeview_bounding_rect(constraint->leader, WORLD_SPACE, constraint->leader_flags) };
    const Box_2D follower_box{ nodeview_get_rect_ex(_follower, WORLD_SPACE, constraint->follower_flags) };

    // Compute how much the follower box needs to be moved to snap the leader's box at a given pivots.
    Vec2 delta = Box_2D::diff(leaders_box, constraint->leader_pivot , follower_box, constraint->follower_pivot );
    delta += constraint->gap_direction * cfg->ui_node_gap(constraint->gap_size);

    // Apply a force to translate to the (single) follower
    Vec2 current_pos = _follower->spatial_node()->position(WORLD_SPACE);
    Vec2 desired_pos = current_pos + delta;
    auto* physics_component = componentbag_get<Physics_Component>(&_follower->node()->component_bag);
    VERIFY(physics_component, "Component required");
    physics_component->translate_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
}

void ndbl::nodeviewcontraint_rule_N_to_1_as_a_row(Node_View_Constraint* constraint, float _dt)
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
        box[i] = Box_2D{ nodeview_get_rect_ex(clean_follower[i], WORLD_SPACE, constraint->follower_flags) };

        // Determine the delta required to snap the current follower with either the leaders or the previous follower.
        if ( i == 0 )
        {
            // First box is aligned with the leader
            const Box_2D leader_box{ nodeview_get_rect_ex(constraint->leader[0], WORLD_SPACE, constraint->leader_flags) };
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
        auto* physics_component = componentbag_get<Physics_Component>(&clean_follower[i]->node()->component_bag);
        if( !physics_component )
            continue;
        Vec2 current_pos = clean_follower[i]->spatial_node()->position(WORLD_SPACE);
        Vec2 desired_pos = current_pos + delta[i];
        physics_component->translate_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
    }
}

void ndbl::nodeviewcontraint_rule_distribute_sub_scope_views(Node_View_Constraint* constraint, float dt)
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
        Scope_View* _follower_scopeview = _follower->internal_scopeview;
        ASSERT(_follower_scopeview);
        if ( !_follower_scopeview->state.has_flags(View_Flag_PINNED) )
            if ( scopeview_must_be_draw(_follower_scopeview) )
                sub_scope_view.push_back( _follower_scopeview );
    }

    // get all content rects
    std::vector<Rect> new_content_rect;
    for(auto _view : sub_scope_view)
        new_content_rect.push_back( _view->content_rect );

    // make a row
    const float gap = get_config()->ui_scope_gap( constraint->gap_size );
    Rect::make_row(new_content_rect, gap );

    // v align
    const Vec2 align_pos = constraint->leader[0]->shape.pivot(constraint->leader_pivot, WORLD_SPACE )
                         + Vec2{0.f, gap} * constraint->gap_direction;
    Rect::align_top(new_content_rect, align_pos.y );

    // h align
    Rect::center(new_content_rect, align_pos.x );

    // translate each sub_scope
    for(size_t i = 0; i < sub_scope_view.size(); ++i)
    {
        const Vec2 cur_pos = sub_scope_view[i]->content_rect.center();
        const Vec2 new_pos = new_content_rect[i].center();
        const Vec2 delta = new_pos - cur_pos;

        // Apply force to translate head
        Node* head_node = sub_scope_view[i]->scope->head;
        auto* physics = componentbag_get<Physics_Component>(&head_node->component_bag);
        VERIFY(physics, "A Physics_Component is required on this entity to apply a force to");
        physics->translate(delta, get_config()->ui_node_speed, true );
    }
}
