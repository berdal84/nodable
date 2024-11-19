#include "GraphView.h"

#include <algorithm>
#include "tools/core/types.h"
#include "tools/core/log.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/core/math.h"
#include "tools/gui/Color.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Utils.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/Interpreter.h"

#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "SlotView.h"
#include "ndbl/core/ComponentFactory.h"
#include "tools/core/StateMachine.h"
#include "ScopeView.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(GraphView);
)

// Popup name
constexpr const char* CONTEXT_POPUP    = "GraphView.ContextMenuPopup";
// Tool names
constexpr const char* CURSOR_STATE     = "Cursor Tool";
constexpr const char* ROI_STATE        = "Selection Tool";
constexpr const char* DRAG_STATE       = "Drag Node Tool";
constexpr const char* VIEW_PAN_STATE   = "Grab View Tool";
constexpr const char* LINE_STATE       = "Line Tool";

GraphView::GraphView(Graph* graph)
: m_graph(graph)
, m_state_machine(this)
{
    ASSERT(graph != nullptr);
    m_state_machine.add_state(CURSOR_STATE);
    m_state_machine.bind<&GraphView::cursor_state_tick>(CURSOR_STATE, When::OnTick);
    m_state_machine.set_default_state(CURSOR_STATE);

    m_state_machine.add_state(ROI_STATE);
    m_state_machine.bind<&GraphView::roi_state_enter>(ROI_STATE, When::OnEnter);
    m_state_machine.bind<&GraphView::roi_state_tick>(ROI_STATE, When::OnTick);

    m_state_machine.add_state(DRAG_STATE);
    m_state_machine.bind<&GraphView::drag_state_enter>(DRAG_STATE, When::OnEnter);
    m_state_machine.bind<&GraphView::drag_state_tick>(DRAG_STATE, When::OnTick);

    m_state_machine.add_state(VIEW_PAN_STATE);
    m_state_machine.bind<&GraphView::view_pan_state_tick>(VIEW_PAN_STATE, When::OnTick);

    m_state_machine.add_state(LINE_STATE);
    m_state_machine.bind<&GraphView::line_state_enter>(LINE_STATE, When::OnEnter);
    m_state_machine.bind<&GraphView::line_state_tick>(LINE_STATE, When::OnTick);
    m_state_machine.bind<&GraphView::line_state_leave>(LINE_STATE, When::OnLeave);

    m_state_machine.start();

    CONNECT(graph->on_add    , &GraphView::decorate_node );
    CONNECT(graph->on_change , &GraphView::_on_graph_change);
    CONNECT(graph->on_reset  , &GraphView::reset);
}

GraphView::~GraphView()
{
    DISCONNECT(m_graph->on_add);
    DISCONNECT(m_graph->on_change);
    DISCONNECT(m_graph->on_reset);
}

void GraphView::decorate_node(Node* node)
{
    ComponentFactory* component_factory = get_component_factory();

    // add NodeView & Physics component
    auto nodeview = component_factory->create<NodeView>();
    auto physics  = component_factory->create<Physics>();

    node->add_component( nodeview );
    node->add_component( physics );

    physics->init( nodeview );

    // add a ScopeView for the inner scope and any child that is owned by this node too
    if ( node->has_internal_scope() )
    {
        Scope*     internal_scope = node->internal_scope();
        ScopeView* scope_view     = component_factory->create<ScopeView>();
        CONNECT(scope_view->on_hover, &GraphView::_set_hovered );
        node->add_component( scope_view );
        scope_view->init( internal_scope );

        for ( Scope* sub_scope : internal_scope->partition() )
        {
            ScopeView* sub_scope_view = component_factory->create<ScopeView>();
            node->add_component(sub_scope_view );
            CONNECT(sub_scope_view->on_hover, &GraphView::_set_hovered );
            sub_scope_view->init(sub_scope );
        }
    }
}

ImGuiID make_wire_id(const Slot *ptr1, const Slot *ptr2)
{
    string128 id;
    id.append_fmt("wire %zu->%zu", ptr1, ptr2);
    return ImGui::GetID(id.c_str());
}

void GraphView::draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos)
{
    VERIFY(from != nullptr, "from slot can't be nullptr");

    Config* cfg = get_config();

    // Style

    ImGuiEx::WireStyle style;
    style.shadow_color = cfg->ui_codeflow_shadowColor,
    style.roundness    = 0.f;

    if (from->slot->type() == SlotFlag_TYPE_FLOW) {
        style.color = cfg->ui_codeflow_color,
                style.thickness = cfg->ui_slot_rectangle_size.x * cfg->ui_codeflow_thickness_ratio;
    } else {
        style.color = cfg->ui_node_borderHighlightedColor;
        style.thickness = cfg->ui_wire_bezier_thickness;
    }

    // Draw

    ImGuiID id = make_wire_id(from->slot, nullptr);
    Vec2 start_pos = from->spatial_node().position(WORLD_SPACE);

    BezierCurveSegment2D segment{
            start_pos, start_pos,
            end_pos, end_pos
    }; // straight line

    ImGuiEx::DrawWire(id, ImGui::GetWindowDrawList(), segment, style);
}

bool GraphView::draw(float dt)
{
    bool changed = false;

    if ( !m_view_state.visible )
        return false;

    // Ensure view state fit with content region
    // (n.b. we could also implement a struct RootViewState wrapping ViewState)
    Rect region = ImGuiEx::GetContentRegion(WORLD_SPACE );
    m_view_state.shape().set_size( region.size() );
    m_view_state.shape().set_position(region.center()); // children will be relative to the center
    m_view_state.shape().draw_debug_info();

    m_hovered = {};

    Config*         cfg                    = get_config();
    Interpreter*    interpreter            = get_interpreter();

    ImDrawList*     draw_list              = ImGui::GetWindowDrawList();
    const bool      enable_edition         = interpreter->is_program_stopped();

    // Draw Scopes
    std::vector<Scope*> scopes_to_draw = graph()->scopes();
    auto low_to_high_depth = [](Scope* s1, Scope* s2) { return s1->depth() < s2->depth(); };
    std::sort(scopes_to_draw.begin(), scopes_to_draw.end(), low_to_high_depth);

    const ScopeView* focused_scope_view = m_focused.type == ViewItemType_SCOPE ? m_focused.scopeview : nullptr;
    for( Scope* scope : scopes_to_draw )
    {
        if (ScopeView* view = scope->view())
        {
            bool highlight = view == focused_scope_view;
            view->draw(dt, highlight);
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
    for (Node* each_node: m_graph->nodes() )
    {
        NodeView *each_view = NodeView::substitute_with_parent_if_not_visible(each_node->get_component<NodeView>());

        if (!each_view) {
            continue;
        }

        std::vector<Slot *> slots = each_node->filter_slots(SlotFlag_FLOW_OUT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index)
        {
            Slot *slot = slots[slot_index];

            if (slot->empty())
            {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent())
            {
                Node*     each_successor_node  = adjacent_slot->node;
                NodeView* possibly_hidden_view = each_successor_node->get_component<NodeView>();
                NodeView* each_successor_view  = NodeView::substitute_with_parent_if_not_visible(possibly_hidden_view);

                if ( each_successor_view == nullptr )
                    continue;
                if ( each_view->visible() == false )
                    continue;
                if ( each_successor_view->visible() == false )
                    continue;

                SlotView* tail = slot->view;
                SlotView* head = adjacent_slot->view;

                ImGuiID id = make_wire_id(slot, adjacent_slot);
                Vec2 tail_pos = tail->spatial_node().position(WORLD_SPACE);
                Vec2 head_pos = head->spatial_node().position(WORLD_SPACE);
                BezierCurveSegment2D segment{
                        tail_pos,
                        tail_pos,
                        head_pos,
                        head_pos,
                };
                ImGuiEx::DrawWire(id, draw_list, segment, code_flow_style);
                if (ImGui::GetHoveredID() == id )
                    m_hovered = {tail, head};
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
    for (Node* node_out: m_graph->nodes() )
    {
        for (const Slot* slot_out: node_out->filter_slots(SlotFlag_OUTPUT))
        {
            for(const Slot* slot_in : slot_out->adjacent())
            {
                if (slot_in == nullptr)
                    continue;

                auto *node_view_out = slot_out->node->get_component<NodeView>();
                auto *node_view_in  = slot_in->node->get_component<NodeView>();

                if ( !node_view_out->visible() )
                    continue;
                if ( !node_view_in->visible() )
                    continue;

                Vec2 p1, cp1, cp2, p2; // BezierCurveSegment's points

                SlotView* slot_view_out = slot_out->view;
                SlotView* slot_view_in  = slot_in->view;

                p1 = slot_view_out->spatial_node().position(WORLD_SPACE);
                p2 = slot_view_in->spatial_node().position(WORLD_SPACE);

                const Vec2  signed_dist = Vec2::distance(p1, p2);
                const float lensqr_dist = signed_dist.lensqr();

                // Animate style
                ImGuiEx::WireStyle style = default_wire_style;
                if (is_selected(node_view_out) ||
                    is_selected(node_view_in))
                {
                    style.color.w *= wave(0.5f, 1.f, (float) App::get_time(), 10.f);
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
                if (node_out->type() == NodeType_VARIABLE ) // from a variable
                {
                    auto variable = static_cast<VariableNode*>( node_out );
                    if (slot_out == variable->ref_out() ) // from a reference slot (can't be a declaration link)
                        if (!node_view_out->selected() && !node_view_in->selected() )
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

                    BezierCurveSegment2D segment{p1, cp1, cp2, p2};

                    ImGuiID id = make_wire_id(slot_view_out->slot, slot_in);
                    ImGuiEx::DrawWire(id, draw_list, segment, style);
                    if (ImGui::GetHoveredID() == id)
                        m_hovered = {slot_view_out, slot_view_in};
                }
            }
        }
    }

    // Draw NodeViews
    for (Node* node : graph()->nodes()  )
    {
        NodeView* nodeview = node->get_component<NodeView>();

        if ( !nodeview)
            continue;
        if ( nodeview->visible() == false )
            continue;

        changed |= nodeview->draw();

        if ( nodeview->hovered() ) // no check if something else is hovered, last node always win against an edge
        {
            if ( nodeview->m_hovered_slotview != nullptr)
            {
                m_hovered = {nodeview->m_hovered_slotview};
            }
            else
                m_hovered = {nodeview};
        }

        // VM Cursor (scroll to the next node when VM is debugging)
        if (interpreter->is_debugging())
            if (interpreter->is_next_node( nodeview->node() ))
                ImGui::SetScrollHereY();
    }

    // Virtual Machine cursor
    if (interpreter->is_program_running())
    {
        const Node* node = interpreter->get_next_node();
        if (const NodeView* view = node->get_component<NodeView>())
        {
            Vec2 left = view->get_rect().left();
            Vec2 interpreter_cursor_pos = Vec2::round(left);
            draw_list->AddCircleFilled(interpreter_cursor_pos, 5.0f, ImColor(255, 0, 0));

            Vec2 linePos = interpreter_cursor_pos + Vec2(-10.0f, 0.5f);
            linePos += Vec2( glm::sin(float(App::get_time()) * 12.0f) * 4.0f, 0.f); // wave
            float size = 20.0f;
            float width = 2.0f;
            ImColor color = ImColor(255, 255, 255);

            // arrow ->
            draw_list->AddLine(linePos - Vec2(1.f, 0.0f), linePos - Vec2(size, 0.0f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, -size * 0.5f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, size * 0.5f), color, width);
        }
    }

    m_state_machine.tick();

    // Debug Infos
    if (cfg->tools_cfg->runtime_debug)
    {
        if (ImGui::Begin("GraphViewToolStateMachine"))
        {
            ImGui::Text("current_tool:         %s", m_state_machine.get_current_state_name());
            ImGui::Text("m_focused.type:       %i", m_focused.type );
            ImGui::Text("m_hovered.type:       %i", m_hovered.type );
            Vec2 mouse_pos = ImGui::GetMousePos();
            ImGui::Text("m_mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
        }
        ImGui::End();
    }

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    if ( changed )
        on_change.emit();

	return changed;
}

void GraphView::_update(float dt, u16_t count)
{
    // Guards
    ASSERT(dt > 0.f );
    ASSERT(count != 0 );

    for(u16_t i = 0; i < count; i++)
        _update( dt );
}

void GraphView::create_constraints__align_down(Node* follower, const  std::vector<Node*>& leader )
{
    if( leader.empty() )
        return;

    std::vector<NodeView*> leader_view;
    for ( Node* _leader : leader )
        leader_view.push_back( _leader->get_component<NodeView>() );

    NodeView* follower_view = follower->get_component<NodeView>();
    ViewConstraint constraint;
    constraint.name           = "Position below previous";
    constraint.rule           = &ViewConstraint::rule_1_to_N_as_row;
    constraint.leader         = leader_view;
    constraint.follower       = {follower_view};
    constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
    const Vec2 halignment     = constraint.leader.size() == 1 ? LEFT : CENTER;
    constraint.leader_pivot   = halignment + BOTTOM;
    constraint.follower_pivot = halignment + TOP;

    // vertical gap
    constraint.gap_size      = Size_MD;
    constraint.gap_direction = BOTTOM;

    follower->get_component<Physics>()->add_constraint(constraint);
};

void GraphView::create_constraints__align_top_recursively(const std::vector<Node*>& unfiltered_follower, ndbl::Node* leader )
{
    if ( unfiltered_follower.empty() )
        return;

    ASSERT(leader);
    NodeView* leader_view = leader->get_component<NodeView>();
    // nodeview's inputs must be aligned on center-top
    // It's a one to many constrain.
    //
    std::vector<NodeView*> follower;
    for (auto* _follower : unfiltered_follower )
        if (Utils::is_output_node_in_expression(_follower, leader))
            follower.push_back(_follower->get_component<NodeView>() );

    if ( follower.empty() )
        return;

    ViewConstraint constraint;
    constraint.name           = "Align many inputs above";
    constraint.rule           = &ViewConstraint::rule_N_to_1_as_a_row;
    constraint.leader         = { leader_view };
    constraint.leader_pivot   = TOP;
    constraint.follower       = follower;
    constraint.follower_pivot = BOTTOM;
    constraint.gap_size       = Size_SM;
    constraint.gap_direction  = TOP;

    if (follower.size() > 1 )
    {
        constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
    }

    if ( leader->has_flow_adjacent() )
    {
        constraint.follower_pivot = BOTTOM_LEFT;
        constraint.leader_pivot   = TOP_RIGHT;
        constraint.row_direction  = RIGHT;
    }

    leader->get_component<Physics>()->add_constraint(constraint);

    for( NodeView* _leader : follower )
        create_constraints__align_top_recursively(_leader->node()->inputs(), _leader->node());
};

void GraphView::create_constraints(Scope* scope )
{
    // distribute child scopes
    if ( scope->is_partitioned() )
    {
        ViewConstraint constraint;
        constraint.name          = "Align ScopeView partitions";
        constraint.rule          = &ViewConstraint::rule_distribute_sub_scope_views;
        constraint.leader        = {scope->node()->get_component<NodeView>()};
        constraint.leader_pivot  = BOTTOM;
        constraint.gap_size      = Size_XL;
        constraint.gap_direction = BOTTOM;
        scope->node()->get_component<Physics>()->add_constraint(constraint);
    }

    for ( Scope* sub_scope : scope->partition() )
    {
        create_constraints(sub_scope);
    }

    for ( Node* child_node : scope->child() )
    {
        // align child below flow_inputs
        if (child_node != scope->first_child() || scope->is_orphan() )
            create_constraints__align_down(child_node, child_node->flow_inputs());

        // align child's inputs above
        if ( Utils::is_instruction(child_node) )
            create_constraints__align_top_recursively(child_node->inputs(), child_node );

        // child's internal scope
        if ( child_node->has_internal_scope() )
            create_constraints( child_node->internal_scope() );
    }
};

void GraphView::_update(float dt)
{
    // Physics Components
    LOG_VERBOSE("GraphView", "Updating constraints ...\n");

    if ( m_physics_dirty )
    {
        // clear all constraints, and THEN create them again

        for (Node* node : graph()->nodes())
            node->get_component<Physics>()->clear_constraints();

        for (Scope* _scope : graph()->root_scopes() )
            create_constraints(_scope);

        for (Node* _node : graph()->nodes() )
            if ( _node->is_orphan() )
                create_constraints__align_down(_node, _node->flow_inputs());

        m_physics_dirty = false;
    }

    // Apply all constraints, and THEN apply all forces
    for ( Node* node : graph()->nodes() ) node->get_component<Physics>()->apply_constraints(dt);
    for ( Node* node : graph()->nodes() ) node->get_component<Physics>()->apply_forces(dt);

    LOG_VERBOSE("GraphView", "Constraints updated.\n");

    // NodeViews
    for (Node* node : graph()->nodes() )
        node->get_component<NodeView>()->update(dt);

    // ScopeViews
    for( Scope* scope : graph()->root_scopes() )
        scope->view()->update( dt, ScopeViewFlags_RECURSE );
}

void GraphView::frame_views(const std::vector<NodeView*>& _views, const Vec2& pivot)
{
    if (_views.empty())
    {
        LOG_VERBOSE("GraphView", "Unable to frame views vector. Reason: is empty.\n");
        return;
    }

    BoxShape2D views_bbox = NodeView::get_rect(_views, WORLD_SPACE );
    const Vec2 desired_pivot_pos = m_view_state.shape().pivot( pivot * 0.95f, WORLD_SPACE); // 5%  margin
    const Vec2 pivot_pos         = views_bbox.pivot(pivot, WORLD_SPACE);
    const Vec2 delta             = desired_pivot_pos - pivot_pos;

    // apply the translation
    // TODO: Instead of applying a translation to all views, apply it to all scope views
    for (Node* node : graph()->nodes() )
        if ( NodeView* view = node->get_component<NodeView>() )
            view->spatial_node().translate( delta );
}

void GraphView::unfold()
{
    const Config* cfg = get_config();

    // Compute the number of update necessary to simulate unfolding for dt seconds
    const u32_t dt      = cfg->graph_view_unfold_duration;
    const u32_t samples = 1000 * dt / cfg->tools_cfg->dt_cap;

    // Run the updates
    _update( float(dt) / samples, samples);
}

void GraphView::add_action_to_node_menu(Action_CreateNode* _action )
{
    m_create_node_menu.add_action(_action);
}

void GraphView::frame_nodes(FrameMode mode )
{
    switch (mode)
    {
        case FRAME_ALL:
        {
            if( m_graph->is_empty() )
                return;

            std::vector<NodeView*> views;
            for(Node* node : graph()->nodes())
                views.push_back( node->get_component<NodeView>() );
            frame_views( views, CENTER );
            break;
        }

        case FRAME_SELECTION_ONLY:
        {
            if ( !m_selected_nodeview.empty())
                frame_views(m_selected_nodeview, CENTER);
            break;
        }
        default:
            VERIFY(false, "unhandled case!");
    }
}

void GraphView::set_selected(const Selection& views, SelectionMode mode )
{
    Selection curr_selection = m_selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        m_selected_nodeview.clear();
        for(auto& each : curr_selection )
            each->set_selected(false);
    }

    for(auto& each : views)
    {
        m_selected_nodeview.emplace_back(each);
        each->set_selected();
    }

    EventPayload_NodeViewSelectionChange event{ m_selected_nodeview, curr_selection };
    get_event_manager()->dispatch<Event_SelectionChange>(event);
}

const GraphView::Selection& GraphView::get_selected() const
{
    return m_selected_nodeview;
}

bool GraphView::is_selected(NodeView* view) const
{
    return std::find( m_selected_nodeview.begin(), m_selected_nodeview.end(), view) != m_selected_nodeview.end();
}

bool GraphView::selection_empty() const
{
    return m_selected_nodeview.empty();
}

void GraphView::_on_graph_change()
{
    m_physics_dirty = true;
}

void GraphView::reset()
{
    if ( m_graph->is_empty() )
        return;

    // unfold the graph (does not work great when nodes are rendered for the first time)
    unfold();

    // make sure views are outside viewable rectangle (to avoid flickering)
    Vec2 far_outside = Vec2(-1000.f, -1000.0f);

    for( Node* node : graph()->nodes() )
        if ( NodeView* v = node->get_component<NodeView>() )
            v->spatial_node().translate( far_outside );

    // physics
    m_physics_dirty = true;

    // frame all (100ms delayed to ensure layout is correct)
    get_event_manager()->dispatch_delayed<Event_FrameSelection>( 100, { FRAME_ALL } );
}

bool GraphView::has_an_active_tool() const
{
    return !m_state_machine.has_default_state();
}

void GraphView::reset_all_properties()
{
    for( Node* node : graph()->nodes() )
        if ( NodeView* v = node->get_component<NodeView>() )
            v->reset_all_properties();
}

Graph *GraphView::graph() const
{
    return m_graph;
}

//-----------------------------------------------------------------------------

void GraphView::draw_create_node_context_menu(CreateNodeCtxMenu& menu, SlotView* dragged_slotview)
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

void GraphView::drag_state_enter()
{
    switch ( m_focused.type )
    {
        case ViewItemType_SCOPE:
        {
            m_focused.scopeview->set_pinned();
            break;
        }
        default:
        {
            for ( NodeView* node_view : get_selected() )
                node_view->set_pinned();
            break;
        }
    }
}

void GraphView::drag_state_tick()
{
    const Vec2 delta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    switch ( m_focused.type )
    {
        case ViewItemType_SCOPE:
        {
            m_focused.scopeview->translate( delta );
            break;
        }

        default:
        {
            for (NodeView* view: get_selected() )
                view->spatial_node().translate( delta );
            break;
        }
    }

    if ( ImGui::IsMouseReleased(0) )
        m_state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void GraphView::view_pan_state_tick()
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graphview content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for( Node* node : graph()->nodes() )
        if ( auto v = node->get_component<NodeView>() )
            v->spatial_node().translate(delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        m_state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void GraphView::cursor_state_tick()
{
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            m_create_node_menu.flag_to_be_reset();

        switch ( m_focused.type )
        {
            case ViewItemType_NULL:
            {
                draw_create_node_context_menu(m_create_node_menu);
                break;
            }

            case ViewItemType_SCOPE:
            {
                Node*     node     = m_focused.scopeview->scope()->node();
                NodeView* nodeview = node->get_component<NodeView>();
                if ( ImGui::MenuItem( nodeview->expanded() ? "Collapse Scope" : "Expand Scope" ) )
                {
                    nodeview->expand_toggle_rec();
                }

                if ( ImGui::MenuItem("Delete Scope") )
                {
                    auto event = new Event_DeleteNode({ m_focused.scopeview->node() });
                    get_event_manager()->dispatch(event);
                }

                if ( ImGui::MenuItem("Select Scope") )
                {
                    // Get descendent scopes
                    std::set<Scope*> children;
                    Scope::get_descendent( children, m_focused.scopeview->scope(), ScopeFlags_INCLUDE_SELF );

                    // Extract node views from each descendent
                    std::set<NodeView*> views;
                    for(Scope* child : children)
                    {
                        // Include scope owner's view too
                        if ( NodeView* view = child->node()->get_component<NodeView>())
                            views.insert( view );

                        // and every other child's
                        for(Node* child_node : child->child())
                            if ( NodeView* view = child_node->get_component<NodeView>())
                                views.insert(view);
                    }
                    // Replace selection
                    set_selected({views.begin(), views.end()});
                }

                ImGui::Separator();
                draw_create_node_context_menu(m_create_node_menu);

                break;
            }
            case ViewItemType_EDGE:
            {
                if ( ImGui::MenuItem(ICON_FA_TRASH " Delete Edge") )
                {
                    auto* event = new Event_DeleteEdge();
                    event->data.first  = m_focused.edge.slot[0]->slot;
                    event->data.second = m_focused.edge.slot[1]->slot;
                    get_event_manager()->dispatch( event );
                }

                break;
            }

            case ViewItemType_SLOT:
            {
                if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect Edges") )
                {
                    auto* event = new Event_SlotDisconnectAll();
                    event->data.first = m_focused.slotview->slot;
                    get_event_manager()->dispatch( event );
                }

                break;
            }
            case ViewItemType_NODE:
            {
                if ( ImGui::MenuItem(ICON_FA_TRASH " Delete Node") )
                {
                    auto* event = new Event_DeleteNode ();
                    event->data.node = m_focused.nodeview->node();
                    get_event_manager()->dispatch( event );
                }

                if ( ImGui::MenuItem(ICON_FA_MAP_PIN " Pin/Unpin Node") )
                {
                    m_focused.nodeview->set_pinned( !m_focused.nodeview->pinned() );
                }

                if ( ImGui::MenuItem(ICON_FA_WINDOW_RESTORE " Arrange Node") )
                {
                    m_focused.nodeview->arrange_recursively();
                }

                break;
            }
        }

        ImGui::EndPopup();

        // When we're focused on something with popup open, we don't want to do things based on m_hovered.type (see below)
        if ( !m_focused.empty() )
            return;
    }

    switch (m_hovered.type)
    {
        case ViewItemType_SLOT:
        {
            if ( ImGui::IsMouseClicked(1) )
            {
                m_focused = m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if (ImGui::IsMouseDragging(0, 0.f))
            {
                m_focused = m_hovered;
                m_state_machine.change_state(LINE_STATE);
            }
            break;
        }

        case ViewItemType_NODE:
        {
            if ( ImGui::IsMouseClicked(1) )
            {
                m_focused = m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if (ImGui::IsMouseReleased(0) )
            {
                // TODO: handle remove!
                // Add/Remove/Replace selection
                SelectionMode flags = SelectionMode_REPLACE;
                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
                    flags = SelectionMode_ADD;
                set_selected({m_hovered.nodeview}, flags);
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseDoubleClicked(0))
            {
                m_hovered.nodeview->expand_toggle();
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseDragging(0))
            {
                m_focused = m_hovered;
                if (!m_hovered.nodeview->selected())
                    set_selected({m_hovered.nodeview});
                m_state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case ViewItemType_EDGE:
        {
            if (ImGui::IsMouseDragging(0, 0.1f))
            {
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseClicked(1))
            {
                m_focused = m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            break;
        }

        case ViewItemType_SCOPE:
        {
            if (ImGui::IsMouseClicked(0))
            {
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseClicked(1))
            {
                m_focused = m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if ( ImGui::IsMouseDragging(0) )
            {
                m_focused = m_hovered;
                m_state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case ViewItemType_NULL:
        {
            if ( ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
            {
                if (ImGui::IsMouseClicked(0))
                    set_selected({}, SelectionMode_REPLACE); // Deselect All (Click on the background)
                else if (ImGui::IsMouseReleased(0))
                    m_focused = {};
                else if (ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup(CONTEXT_POPUP);
                else if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::IsKeyDown(ImGuiKey_Space))
                        m_state_machine.change_state(VIEW_PAN_STATE);
                    else
                        m_state_machine.change_state(ROI_STATE);
                }
            }

            break;
        }

        default:
            VERIFY(false, "Unhandled case, must be implemented!");
    }
}

//-----------------------------------------------------------------------------

void GraphView::line_state_enter()
{
    ASSERT(m_focused.type == ViewItemType_SLOT);
    ASSERT(m_focused.slotview != nullptr);
}

void GraphView::line_state_tick()
{
    Vec2 mouse_pos_snapped = m_hovered.type == ViewItemType_SLOT ? m_hovered.slotview->spatial_node().position(WORLD_SPACE)
                                                                 : Vec2{ImGui::GetMousePos()};

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            m_create_node_menu.flag_to_be_reset();

        if ( m_hovered.empty() )
            draw_create_node_context_menu(m_create_node_menu, m_focused.slotview);

        if ( ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
            m_state_machine.exit_state();

        ImGui::EndPopup();
    }
    else if ( ImGui::IsMouseReleased(0) )
    {
        switch (m_hovered.type)
        {
            case ViewItemType_SLOT:
            {
                if ( m_focused.slotview == m_hovered.slotview )
                    break;
                auto event = new Event_SlotDropped();
                event->data.first  = m_focused.slotview->slot;
                event->data.second = m_hovered.slotview->slot;
                get_event_manager()->dispatch(event);
                m_state_machine.exit_state();
                break;
            }

            case ViewItemType_SCOPE:
            case ViewItemType_EDGE:
            case ViewItemType_NODE:
            case ViewItemType_NULL: // ...on background
            {
                ImGui::OpenPopup(CONTEXT_POPUP);
                break;
            }
        }
    }

    // Draw a temporary wire from focused/dragged slotview to the mouse cursor
    draw_wire_from_slot_to_pos(m_focused.slotview, mouse_pos_snapped );
}

void GraphView::line_state_leave()
{
    m_focused = {};
}

//-----------------------------------------------------------------------------

void GraphView::roi_state_enter()
{
    m_roi_state_start_pos = ImGui::GetMousePos();
    m_roi_state_end_pos   = ImGui::GetMousePos();;
}

void GraphView::roi_state_tick()
{
    m_roi_state_end_pos = ImGui::GetMousePos();

    // Get normalized ROI rectangle
    Rect roi = Rect::normalize({m_roi_state_start_pos, m_roi_state_end_pos});

    // Expand to avoid null area
    const int roi_border_width = 2;
    roi.expand(Vec2{roi_border_width*0.5f});

    // Draw the ROI rectangle
    float alpha = wave(0.5f, 0.75f, (float) App::get_time(), 10.f);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll , roi_border_width );

    if (ImGui::IsMouseReleased(0))
    {
        // Select the views included in the ROI
        std::vector<NodeView*> nodeview_in_roi;
        for ( Node* node : graph()->nodes() )
            if ( auto v = node->get_component<NodeView>() )
                if (Rect::contains(roi, v->get_rect()))
                    nodeview_in_roi.emplace_back(v);

        bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                               ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        SelectionMode flags = control_pressed ? SelectionMode_ADD : SelectionMode_REPLACE;
        set_selected(nodeview_in_roi, flags);

        m_state_machine.exit_state();
    }
}

void GraphView::add_child(NodeView* view)
{
    m_view_state.spatial_node().add_child( &view->spatial_node() );
}

void GraphView::update(float dt)
{
    // Determines how many times update should be called
    ASSERT( dt >= 0.f);
    u16_t sample_count = (u16_t)(dt * get_config()->ui_node_physics_frequency);
    if ( sample_count == 0 ) // When frame rate is too slow
        sample_count = 1;
    const float sample_dt = dt / float(sample_count);

    // Do the update(s)
    for(size_t i = 0; i < sample_count; ++i)
        _update(sample_dt);
}

void GraphView::_set_hovered(ScopeView* scope_view)
{
    if ( m_hovered.type != ViewItemType_SCOPE )
        m_hovered = scope_view;
    else if ( !m_hovered.scopeview )
        m_hovered = scope_view;
    else if ( scope_view->depth() >= m_hovered.scopeview->depth() )
        m_hovered = scope_view;
}

