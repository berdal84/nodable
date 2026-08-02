#include "Graph_View.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include "imgui.h"

#include "tools/core/Component.h"
#include "tools/core/Types.h"
#include "tools/core/Math.h"
#include "tools/core/State_Machine.h"
#include "tools/gui/Size.h"
#include "tools/gui/View_State.h"
#include "tools/gui/geometry/Box_2D.h"
#include "tools/gui/geometry/Pivots.h"
#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/Spatial_Node.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/gui/Layout.h"
#include "tools/gui/App.h"

#include "ndbl/core/Scope.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"

#include "ndbl/gui/Config.h"
#include "ndbl/gui/Event.h"
#include "ndbl/gui/Node_View.h"
#include "ndbl/gui/Node_Slot_View.h"
#include "ndbl/gui/Scope_View.h"

// private
namespace ndbl
{   
    void    _graphview_draw_wire_from_slot_to_pos(Graph_View*, Node_Slot_View *from, const Vec2 &end_pos);
    void    _graphview_handle_init(Graph_View*);
    void    _graphview_handle_deinit(Graph_View*);
    void    _graphview_handle_add_node(Graph_View*, Node*);
    void    _graphview_handle_remove_node(Graph_View*, Node* node);
    void    _graphview_handle_change_scope(Graph_View*, Graph::Scope_Change);
    void    _graphview_handle_hover(Graph_View*, Scope_View*);
    void    _graphview_on_graph_change(Graph_View*);
    void    _graphview_on_selection_change(Graph_View*, Selection::Event_Type, Selection::Element );
    void    _graphview_draw_context_menu(Graph_View*, Node_Slot_View* dragged_slotview = nullptr );
    void    _graphview_do_layout_recursively(Graph_View*, Node_View*);
    void    _graphview_do_layout_recursively_on_expressions_only(Graph_View*, Node_View*);
    void    _graphview_do_layout_element(Graph_View*, Node_View* );
    void    _graphview_cursor_state_tick(Graph_View*);
    void    _graphview_roi_state_enter(Graph_View*);
    void    _graphview_roi_state_tick(Graph_View*);
    void    _graphview_drag_state_enter(Graph_View*);
    void    _graphview_drag_state_tick(Graph_View*);
    void    _graphview_view_pan_state_tick(Graph_View*);
    void    _graphview_line_state_enter(Graph_View*);
    void    _graphview_line_state_tick(Graph_View*);
    void    _graphview_line_state_leave(Graph_View*);

    // Popup name
    constexpr const char* CONTEXT_POPUP    = "Graph_View.ContextMenuPopup";

    // Tool names
    constexpr const char* CURSOR_STATE     = "Cursor Tool";
    constexpr const char* ROI_STATE        = "Selection Tool";
    constexpr const char* DRAG_STATE       = "Drag Node Tool";
    constexpr const char* VIEW_PAN_STATE   = "Grab View Tool";
    constexpr const char* LINE_STATE       = "Line Tool";
}

using namespace ndbl;
using namespace tools;

ndbl::Graph_View::Graph_View()
: Component<Graph>("Graph_View")
, state_machine(this)
, shape( Vec2{100.f, 100.f} ) // non null area
{
    signal_init.connect<&_graphview_handle_init>(this);
    signal_deinit.connect<&_graphview_handle_deinit>(this);

    state_machine.add_state(CURSOR_STATE);
    state_machine.bind<&_graphview_cursor_state_tick>(CURSOR_STATE, When::OnTick);
    state_machine.set_default_state(CURSOR_STATE);

    state_machine.add_state(ROI_STATE);
    state_machine.bind<&_graphview_roi_state_enter>(ROI_STATE, When::OnEnter);
    state_machine.bind<&_graphview_roi_state_tick>(ROI_STATE, When::OnTick);

    state_machine.add_state(DRAG_STATE);
    state_machine.bind<&_graphview_drag_state_enter>(DRAG_STATE, When::OnEnter);
    state_machine.bind<&_graphview_drag_state_tick>(DRAG_STATE, When::OnTick);

    state_machine.add_state(VIEW_PAN_STATE);
    state_machine.bind<&_graphview_view_pan_state_tick>(VIEW_PAN_STATE, When::OnTick);

    state_machine.add_state(LINE_STATE);
    state_machine.bind<&_graphview_line_state_enter>(LINE_STATE, When::OnEnter);
    state_machine.bind<&_graphview_line_state_tick>(LINE_STATE, When::OnTick);
    state_machine.bind<&_graphview_line_state_leave>(LINE_STATE, When::OnLeave);
}

ndbl::Graph_View::~Graph_View()
{
    // not really needed, but lets clear memory
    signal_init.disconnect();
    signal_deinit.disconnect();
}

void ndbl::_graphview_handle_init(Graph_View* graph_view)
{
    // add nodes present before connecting signals
    for( Node* each_node : graph_view->graph()->nodes )
    {
        _graphview_handle_add_node(graph_view, each_node);
    }

    graph_view->selection.signal_change.connect<&_graphview_on_selection_change>(graph_view);
    graph_view->graph()->signal_change.connect<&_graphview_on_graph_change>(graph_view);
    graph_view->graph()->signal_add_node.connect<&_graphview_handle_add_node>(graph_view);
    graph_view->graph()->signal_remove_node.connect<&_graphview_handle_remove_node>(graph_view);
    graph_view->graph()->signal_change_scope.connect<&_graphview_handle_change_scope>(graph_view);
    graph_view->graph()->signal_reset.connect<&graphview_reset>(graph_view);
    graph_view->graph()->signal_is_complete.connect<&graphview_reset>(graph_view);

    graph_view->state_machine.start();
}

void ndbl::_graphview_handle_deinit(Graph_View* graph_view)
{
    graph_view->state_machine.stop();

    graph_view->selection.signal_change.disconnect();
    graph_view->graph()->signal_add_node.disconnect();
    graph_view->graph()->signal_remove_node.disconnect();
    graph_view->graph()->signal_reset.disconnect();
    graph_view->graph()->signal_is_complete.disconnect();
    ASSERT_DEBUG_ONLY( graph_view->graph()->signal_change.disconnect<&_graphview_on_graph_change>(graph_view) );

    // add nodes still present after connecting signals
    for( Node* each_node : graph_view->graph()->nodes )
    {
        _graphview_handle_remove_node(graph_view, each_node);
    }
}

void ndbl::_graphview_handle_add_node(Graph_View* graph_view, Node* node)
{
    // view
    auto* nodeview = new Node_View();
    component_init(nodeview, node);
    nodeview->shape.set_size({80.f, 35.f});

    componentbag_add(& node->component_bag, nodeview);

    if (Scope_View* scopeview = nodeview->internal_scopeview )
    {
        scopeview->signal_hover.connect<&_graphview_handle_hover>(graph_view); // I'm not sure if this is a good approach...
    }

    // All Node_Views are parented to the Graph_View
    spatialnode_add_child(&graph_view->shape.spatial_node, &nodeview->shape.spatial_node );
}

void ndbl::_graphview_handle_remove_node(Graph_View* graph_view, Node* node)
{
    // clean nodeview
    auto* nodeview = componentbag_get<Node_View>(&node->component_bag);
    VERIFY(nodeview, "Should have been created from _handle_add_node()");

    if ( Scope_View* scopeview = nodeview->internal_scopeview )
    {
        scopeview->signal_hover.disconnect(); // I'm not sure if this is a good approach...
    }

    if( nodeview->shape.spatial_node.parent )
    {
        spatialnode_remove_child(nodeview->shape.spatial_node.parent, &nodeview->shape.spatial_node );
    }

    componentbag_remove( &node->component_bag, nodeview );
    component_deinit(nodeview);
    delete nodeview;
}

void ndbl::_graphview_handle_change_scope(Graph_View* graph_view, Graph::Scope_Change change)
{
    auto* nodeview = componentbag_get<Node_View>(&change.node->component_bag);
    VERIFY(nodeview, "a nodeview must be present since we are in a Graph_View");

    // Un-parent from old scope's spatial node
    if( auto _parent = nodeview->shape.spatial_node.parent )
        spatialnode_remove_child( _parent, &nodeview->shape.spatial_node );

    // Parent to new scope or default to graph's spatial node
    if( Scope_View* _scopeview = change.new_scope->view )
        spatialnode_add_child(&_scopeview->spatial_node, &nodeview->shape.spatial_node );
}

ImGuiID make_wire_id(const Node_Slot *ptr1, const Node_Slot *ptr2)
{
    String_128 id;
    id.append_fmt("wire %zu->%zu", ptr1, ptr2);
    return ImGui::GetID(id.c_str());
}

void ndbl::_graphview_draw_wire_from_slot_to_pos(Graph_View*, Node_Slot_View *from, const Vec2 &end_pos)
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
    Vec2 start_pos = from->shape.pivot_position(CENTER, WORLD_SPACE);

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
    graph_view->shape.set_position(region.top_left()); // children will be relative to the center
    box2d_draw_debug_info(&graph_view->shape);

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
                Vec2 tail_pos = tail->shape.pivot_position(CENTER, WORLD_SPACE);
                Vec2 head_pos = head->shape.pivot_position(CENTER,  WORLD_SPACE);
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

                p1 = slot_view_out->shape.pivot_position(CENTER, WORLD_SPACE);
                p2 = slot_view_in->shape.pivot_position(CENTER, WORLD_SPACE);

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


    // debug layout
    if( cfg->has_flags(Config_Flag_DRAW_DEBUG_LINES))
    {
        auto list = ImGui::GetForegroundDrawList();
        for(Element& el : layout_elements())
        {
            switch ( el.type)
            {
                case tools::Element::Type_CONTAINER:
                {
                    list->AddRect(element_rect_min(&el), element_rect_max(&el), ImColor(0,255,0), 0.0f, 0, 2.f);
                    break;
                }

                case tools::Element::Type_LEAF:
                {
                    list->AddRect(element_rect_min(&el), element_rect_max(&el), ImColor(255,255,255));
                    break;
                }            
            }            
        }
    }

	return changed;
}

void ndbl::_graphview_do_layout_element(Graph_View* graph_view, Node_View* nodeview )
{
    Config* cfg  = get_config();
    Rect    rect = nodeview_get_rect(nodeview, tools::PARENT_SPACE);

    layout_append_element(rect.width(), rect.height(), nodeview);

    if( nodeview->state.has_flags(View_Flag_PINNED) )
    {
        layout_pin_element_at_position(rect.top_left());
    }
}

void ndbl::_graphview_do_layout_recursively_on_expressions_only(Graph_View* graph_view, Node_View* nodeview )
{
    Config* cfg = get_config();

    Node*   node = nodeview->node();
    Rect    rect = nodeview_get_rect(nodeview, WORLD_SPACE);

    if( node->inputs().empty() )
    {
        return _graphview_do_layout_element(graph_view, nodeview);
    }

    layout_begin_column();
    {
        layout_set_gap( cfg->ui_node_gap(Size_SM).y );
        layout_begin_row();
        {
            layout_set_gap( cfg->ui_node_gap(Size_SM).x );            
            if( node_is_connected_to_codeflow(node) )
            {
                layout_set_padding( rect.width(), 0, 0, 0 );
            }   

            for( Node* input_node : node->inputs() )
            {
                Node_View* input_nodeview = node_component<Node_View>(input_node);

                if (node_is_connected_to_codeflow(input_node))
                {
                    continue;
                }

                _graphview_do_layout_recursively_on_expressions_only(graph_view, input_nodeview);
            }
        }
        layout_end();

        _graphview_do_layout_element(graph_view, nodeview);
    }
    layout_end();
}

void ndbl::_graphview_do_layout_recursively(Graph_View* graph_view, Node_View* nodeview )
{
    Config* cfg  = get_config();
    Node*   node = nodeview->node();

    if( node->internal_scope == nullptr)
    {
        return _graphview_do_layout_recursively_on_expressions_only(graph_view, nodeview);
    }

    layout_begin_column();
    {
        if( nodeview->node() == graph_root( graph_view->graph() ) )
        {
            layout_pin_element_at_position(nodeview->shape.position());
        }

        _graphview_do_layout_recursively_on_expressions_only(graph_view, nodeview);

        for( Node* backbone_node : scope_get_backbone(node->internal_scope) )
        {
            Node_View* backbone_nodeview = node_component<Node_View>(backbone_node);
            _graphview_do_layout_recursively_on_expressions_only(graph_view, backbone_nodeview);
        }
    }
    layout_end();

    // // Create a row with each branch
    // if( node_has_switch_behavior(node))
    // {
    //     if( node->switch_data.branch_count > 1)
    //     {
    //         Rect parent_rect = nodeview_get_rect(node_view);
    //         layout_set_cursor( parent_rect.bottom_left() + cfg->ui_node_gap() * Vec2(1.f, 1.f) );
    //         layout_begin( Container_Config::Row );
    //         layout_set_container_gap( cfg->ui_node_gap(tools::Size_SM).x );

    //         for( Scope* partition : node->internal_scope->partition )
    //         {
    //             Node*       partition_node     = partition->node();
    //             Node_View*  partition_nodeview = node_component<Node_View>(partition->node());

    //             Rect partition_rect = partition_nodeview->shape.rect(WORLD_SPACE);

    //             layout_push(partition_rect.width(), partition_rect.height(), partition_nodeview);
    //         }
    //         layout_end();
    //     }
    // }
};

void ndbl::graphview_update(Graph_View* graph_view, float dt)
{
    ASSERT( graph_view->graph() );

    if( graph_view->flags & Graph_View_Flag_NEEDS_TO_BE_RESET)
    {
        graphview_reset(graph_view);
        graph_view->flags &= ~Graph_View_Flag_NEEDS_TO_BE_RESET;
    }

    // Define a Layout
    layout_begin_frame();
    layout_begin();
    {
        Node* root_node = graph_root(graph_view->graph());
        auto* root_nodeview = node_component<Node_View>(root_node);
        layout_pin_element_at_position(root_nodeview->shape.position(tools::WORLD_SPACE) + get_config()->ui_textview_padding );
        _graphview_do_layout_recursively(graph_view, root_nodeview);
    }
    layout_end();
    layout_end_frame();

    // Update the layout
    layout_compute_sizes_and_positions();

    // Update our views according to the layout (positions/sizes)
    for(const Element& elem : layout_elements() )
    {
        if (elem.userdata == nullptr)
        {
            // where no userdata is set we have nothing to do with this element
            continue;
        }

        auto nodeview = static_cast<Node_View*>(elem.userdata);
        
        if (nodeview->state.has_flags(View_Flag_PINNED))
        {
            // pinned views are positionned by the user and should not be moved
            continue;
        }
        
        spatialnode_set_position(&nodeview->spatial_node(), graph_view->shape.position() + elem.position, tools::WORLD_SPACE);
    }
    
    // Node_Views
    for (Node* node : graph_view->graph()->nodes )
        if ( auto* view = componentbag_get<Node_View>(&node->component_bag) )
            nodeview_update(view, dt);

    // Scope_Views
    if( Scope* root = graph_root_scope(graph_view->graph()) )
        if ( root->view != nullptr )
            scopeview_update( root->view, dt, Scope_View_Flag_RECURSE );

    
    if( graph_view->flags & Graph_View_Flag_NEEDS_TO_FRAME_CONTENT)
    {
        graphview_frame_content(graph_view, Frame_Mode::Root_Node_View );
        graph_view->flags &= ~Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
    }    
}

void ndbl::graphview_frame_content(Graph_View* graph_view, Frame_Mode mode )
{
    // Frame_Mode::Root_Node_View
    if ( mode ==  Frame_Mode::Root_Node_View || graph_view->selection.collect<Node_View*>().empty() )
    {
        // Move the main Scope_View to the top-left corner of the Graph_View
        Scope* root_scope = graph_root_scope(graph_view->graph());
        auto root_scopeview = root_scope->view;
        ASSERT(root_scopeview);
        const Vec2 target   = spatialnode_position( &root_scopeview->spatial_node, WORLD_SPACE ) + get_config()->ui_textview_padding;
        const Vec2 origin   = root_scope->view->content_rect.top_left();
        const Vec2 delta    = target - origin;
        spatialnode_translate( &root_scopeview->spatial_node, delta );
        return;
    }
    // Frame_Mode::Selected_Node_Views
    else
    {   
        // Get selected node views rectangle
        std::vector<Node_View*> selected_nodeviews = graph_view->selection.collect<Node_View*>();
        const Rect rect = nodeview_bounding_rect( selected_nodeviews, WORLD_SPACE);

        // compute the delta to apply
        const Vec2 target = graph_view->shape.pivot_position( tools::CENTER, WORLD_SPACE);
        const Vec2 source = rect.min;
        const Vec2 delta =  target - source;

        // apply the delta to all node views
        for (Node* node : graph_view->graph()->nodes )
            if ( Node_View* nodeview = componentbag_get<Node_View>(&node->component_bag) )
                spatialnode_translate( &nodeview->shape.spatial_node, delta );
    }
}

void ndbl::_graphview_on_graph_change(Graph_View* graph_view)
{
    // graph_view->is_physics_dirty = true;
}

void ndbl::_graphview_on_selection_change(Graph_View* graph_view, Selection::Event_Type type, Selection::Element elem)
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
    
    for( Scope* scope : graph_collect_scopes(graph_view->graph()) )
    {
        if ( Scope_View* scopeview = scope->view )
        {
            scopeview->state.flags = View_Flag_DEFAULTS;
        }
    }

    for( Node* node : graph_view->graph()->nodes )
    {
        if ( Node_View* each_node_view = componentbag_get<Node_View>(&node->component_bag) )
        {
            each_node_view->state.flags = View_Flag_DEFAULTS;
            nodeview_reset_all_properties(each_node_view);
        }
    }
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

void ndbl::_graphview_draw_context_menu(Graph_View* graph_view, Node_Slot_View* dragged_slotview)
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

void ndbl::_graphview_drag_state_enter(Graph_View* graph_view)
{
    for( const Selectable& elem : graph_view->selection )
    {
        if ( auto* nodeview = elem.get_if<Node_View*>() )
            nodeview->state.set_flags(View_Flag_PINNED);
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
            scopeview->state.set_flags(View_Flag_PINNED);
    }
}

void ndbl::_graphview_drag_state_tick(Graph_View* graph_view)
{
    const Vec2 delta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    for ( const Selectable& elem : graph_view->selection )
    {
        auto* nodeview = elem.get_if<Node_View*>();

        if ( nodeview )
        {
            spatialnode_translate(&nodeview->shape.spatial_node, delta);
            nodeview->state.set_flags(View_Flag_PINNED);
        }
        else if ( auto* scopeview = elem.get_if<Scope_View*>() )
        {
            nodeview = node_component<Node_View>(scopeview->scope->entity);
            spatialnode_translate(&nodeview->shape.spatial_node, delta);
            nodeview->state.set_flags(View_Flag_PINNED);
        }
    }

    if ( ImGui::IsMouseReleased(0) )
        graph_view->state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void ndbl::_graphview_view_pan_state_tick(Graph_View* graph_view)
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graph_view content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for( Scope* scope : graph_collect_root_scopes(graph_view->graph()) )
        if ( auto nodeview = componentbag_get<Node_View>(&scope->node()->component_bag) )
            spatialnode_translate(&nodeview->shape.spatial_node, delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        graph_view->state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_cursor_state_tick(Graph_View* graph_view)
{
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            graph_view->contextual_menu.flag_to_be_reset();

        switch ( graph_view->focused.index() )
        {
            case Selectable::index_null:
            {
                _graphview_draw_context_menu(graph_view);
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
                _graphview_draw_context_menu(graph_view);

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
                    graphview_reset(graph_view);
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

void ndbl::_graphview_line_state_enter(Graph_View* graph_view)
{
    ASSERT( graph_view->focused.holds_alternative<Node_Slot_View*>() );
}

void ndbl::_graphview_line_state_tick(Graph_View* graph_view)
{
    Vec2 mouse_pos_snapped = Vec2{ImGui::GetMousePos()};
    if ( auto slotview = graph_view->hovered.get_if<Node_Slot_View*>() )
    {
        mouse_pos_snapped = spatialnode_position(&slotview->shape.spatial_node, WORLD_SPACE);
    }

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            graph_view->contextual_menu.flag_to_be_reset();

        if ( graph_view->hovered.empty() )
            _graphview_draw_context_menu(graph_view, graph_view->focused.get<Node_Slot_View*>() );

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
    ndbl::_graphview_draw_wire_from_slot_to_pos(graph_view, graph_view->focused.get<Node_Slot_View*>(), mouse_pos_snapped );
}

void ndbl::_graphview_line_state_leave(Graph_View* graph_view)
{
    graph_view->focused = {};
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_roi_state_enter(Graph_View* graph_view)
{
    graph_view->state_roi_start_pos = ImGui::GetMousePos();
    graph_view->state_roi_end_pos   = ImGui::GetMousePos();;
}

void ndbl::_graphview_roi_state_tick(Graph_View* graph_view)
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

void ndbl::_graphview_handle_hover(Graph_View* graph_view, Scope_View* scope_view)
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
