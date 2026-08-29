#include "Graph_View.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>

#include "bdc/String.hpp"
#include "gui/Command.h"
#include "gui/Command_Manager.h"
#include "gui/Config.h"
#include "imgui.h"

#include "tools/core/Asserts.h"
#include "tools/core/Event_Manager.h"
#include "tools/core/Event.h"
#include "tools/core/Flags.h"
#include "tools/core/Math.h"
#include "tools/core/State_Machine.h"
#include "tools/gui/App.h"
#include "tools/gui/geometry/Box_2D.h"
#include "tools/gui/geometry/Pivots.h"
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/Spatial_Node.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/geometry/Vec4.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/gui/Layout.h"
#include "tools/gui/Size.h"
#include "tools/gui/View_Flags.h"

#include "ndbl/core/Scope.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"

#include "ndbl/gui/View.h"
#include "ndbl/gui/Node_Search_Input.h"
#include "ndbl/gui/Config.h"
#include "ndbl/gui/Event.h"
#include "ndbl/gui/Node_View.h"
#include "ndbl/gui/Node_Slot_View.h"
#include "ndbl/gui/Scope_View.h"

// private
namespace ndbl
{   
    void    _graphview_draw_wire_from_slot_to_pos(Graph_View*, Node_Slot_View *from, const Vec2 &end_pos, bool* hovered = nullptr);
    void    _graphview_handle_add_node(Graph_View*, Node*);
    void    _graphview_handle_remove_node(Graph_View*, Node* node);
    void    _graphview_handle_change_scope(Graph_View*, Graph::Scope_Change);
    void    _graphview_handle_hover(Graph_View*, Scope_View*);
    void    _graphview_frame_content(Graph_View*);
    void    _graphview_reset(Graph_View*);
    void    _graphview_on_graph_change(Graph_View*);
    void    _graphview_on_selection_change(Graph_View*, View_Selection_Event_Type, View);
    void    _graphview_draw_context_menu(Graph_View*, Node_Slot_View* dragged_slotview = nullptr );
    void    _graphview_do_layout_recursively(Graph_View*, Node_View*);
    void    _graphview_do_layout_recursively_on_expressions_only(Graph_View*, Node_View*);
    void    _graphview_select_scope(Graph_View*, Scope_View*);
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
    const char* CONTEXT_POPUP    = "Graph_View.ContextMenuPopup";

    // Tool names
    const char* CURSOR_STATE     = "Cursor Tool";
    const char* ROI_STATE        = "Selection Tool";
    const char* DRAG_STATE       = "Drag Node Tool";
    const char* VIEW_PAN_STATE   = "Grab View Tool";
    const char* LINE_STATE       = "Line Tool";
}

using namespace ndbl;
using namespace tools;

void ndbl::graphview_init(Graph_View* graphview, Graph* graph)
{
    graphview->graph = graph;

    graphview->state_machine.init(graphview);
    graphview->shape = Vec2{100.f,100.f}; // non null area

    graphview->state_machine.add_state(CURSOR_STATE);
    graphview->state_machine.bind<&_graphview_cursor_state_tick>(CURSOR_STATE, When::OnTick);
    graphview->state_machine.set_default_state(CURSOR_STATE);

    graphview->state_machine.add_state(ROI_STATE);
    graphview->state_machine.bind<&_graphview_roi_state_enter>(ROI_STATE, When::OnEnter);
    graphview->state_machine.bind<&_graphview_roi_state_tick>(ROI_STATE, When::OnTick);

    graphview->state_machine.add_state(DRAG_STATE);
    graphview->state_machine.bind<&_graphview_drag_state_enter>(DRAG_STATE, When::OnEnter);
    graphview->state_machine.bind<&_graphview_drag_state_tick>(DRAG_STATE, When::OnTick);

    graphview->state_machine.add_state(VIEW_PAN_STATE);
    graphview->state_machine.bind<&_graphview_view_pan_state_tick>(VIEW_PAN_STATE, When::OnTick);

    graphview->state_machine.add_state(LINE_STATE);
    graphview->state_machine.bind<&_graphview_line_state_enter>(LINE_STATE, When::OnEnter);
    graphview->state_machine.bind<&_graphview_line_state_tick>(LINE_STATE, When::OnTick);
    graphview->state_machine.bind<&_graphview_line_state_leave>(LINE_STATE, When::OnLeave);

    // add nodes present before connecting signals
    for(Node& each_node : graph->nodes )
    {
        _graphview_handle_add_node(graphview, &each_node);
    }

    graphview->selection.signal_change.connect<&_graphview_on_selection_change>(graphview);
    graph->signal_change.connect<&_graphview_on_graph_change>(graphview);
    graph->signal_add_node.connect<&_graphview_handle_add_node>(graphview);
    graph->signal_remove_node.connect<&_graphview_handle_remove_node>(graphview);
    graph->signal_change_scope.connect<&_graphview_handle_change_scope>(graphview);
    graph->signal_reset.connect<&_graphview_reset>(graphview);
    graph->signal_is_complete.connect<&_graphview_reset>(graphview);

    graphview->state_machine.start();
}

void ndbl::graphview_deinit(Graph_View* graphview)
{
    graphview->state_machine.stop();

    graphview->selection.signal_change.disconnect();
    graphview->graph->signal_add_node.disconnect();
    graphview->graph->signal_remove_node.disconnect();
    graphview->graph->signal_reset.disconnect();
    graphview->graph->signal_is_complete.disconnect();
    ASSERT_DEBUG_ONLY( graphview->graph->signal_change.disconnect<&_graphview_on_graph_change>(graphview) );

    // add nodes still present after connecting signals
    for( Node& each_node : graphview->graph->nodes )
    {
        _graphview_handle_remove_node(graphview, &each_node);
    }
}

void ndbl::_graphview_handle_add_node(Graph_View* graphview, Node* node)
{
    // view
    node->view = bdc::memory_new<Node_View>();
    nodeview_init(node->view, node);
    node->view->shape.set_size({80.f, 35.f});

    if (Scope_View* scopeview = node->view->internal_scopeview )
    {
        scopeview->signal_hover.connect<&_graphview_handle_hover>(graphview); // I'm not sure if this is a good approach...
    }

    // All Node_Views are parented to the Graph_View
    spatialnode_add_child(&graphview->shape.spatial_node, &node->view->shape.spatial_node );
}

void ndbl::_graphview_handle_remove_node(Graph_View* graphview, Node* node)
{
    // clean node.view
    VERIFY(node->view, "Should have been created from _handle_add_node()");

    if ( Scope_View* scopeview = node->view->internal_scopeview )
    {
        scopeview->signal_hover.disconnect(); // I'm not sure if this is a good approach...
    }

    if( node->view->shape.spatial_node.parent )
    {
        spatialnode_remove_child(node->view->shape.spatial_node.parent, &node->view->shape.spatial_node );
    }

    if( node->view)
    {
        nodeview_deinit(node->view);
        bdc::memory_free(node->view);
    }
}

void ndbl::_graphview_handle_change_scope(Graph_View* graphview, Graph::Scope_Change change)
{
    // Note: we were previously remove/add_child from/to parent, but now we want all Node_View
    //       to be child of the Graph_View instead.

    // auto* node.view = componentbag_get<Node_View>(&change.node->component_bag);
    // VERIFY(node.view, "a node.view must be present since we are in a Graph_View");

    // // Un-parent from old scope's spatial node
    // if( auto _parent = node.view->shape.spatial_node.parent )
    //     spatialnode_remove_child( _parent, &node.view->shape.spatial_node );

    // // Parent to new scope or default to graph's spatial node
    // if( Scope_View* _scopeview = change.new_scope->view )
    //     spatialnode_add_child(&_scopeview->spatial_node, &node.view->shape.spatial_node );
}

ImGuiID make_wire_id(const Node_Slot *ptr1, const Node_Slot *ptr2)
{
    bdc::String temp = string_printf( bdc::temp_allocator(), "wire %zu->%zu", ptr1, ptr2 );
    return ImGui::GetID(temp.data);
}

void ndbl::_graphview_draw_wire_from_slot_to_pos(Graph_View*, Node_Slot_View *from, const Vec2 &end_pos, bool* hovered)
{
    VERIFY(from != nullptr, "from slot can't be nullptr");

    Config* cfg = config();

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

    Vec2 start_pos = from->shape.pivot_position(CENTER, WORLD_SPACE);

    Bezier_Curve_Segment_2D segment{
            start_pos, start_pos,
            end_pos, end_pos
    }; // straight line

    ImGuiEx::DrawWire(ImGui::GetWindowDrawList(), segment, style, hovered);
}

bool ndbl::graphview_draw(Graph_View* graphview, float dt)
{
    bool changed = false;

    // Ensure view state fit with content region
    // (n.b. we could also implement a struct RootView_State wrapping View_State)
    Rect region = ImGuiEx::GetContentRegion(WORLD_SPACE);
    graphview->shape.set_size( region.size() );
    graphview->shape.set_position(region.top_left()); // children will be relative to the center
    box2d_draw_debug_info(&graphview->shape);

    Node_Slot_Link_View lastframe_hovered_linkview;
    if(graphview->hovered.type == View_Type_LINK)
    {
        lastframe_hovered_linkview = graphview->hovered.linkview;
    }

    graphview->hovered = {};

    Config*         cfg       = config();
    ImDrawList*     draw_list = ImGui::GetWindowDrawList();

    // Draw Scopes
    std::vector<Scope*> scopes_to_draw = graph_collect_scopes(graphview->graph);
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
    ImGuiEx::WireStyle style {
        cfg->ui_codeflow_color,
        cfg->ui_codeflow_color, // hover
        cfg->ui_codeflow_shadowColor,
        cfg->ui_codeflow_thickness(),
        0.0f
    };
    for (Node& each_node: graphview->graph->nodes )
    {
        if ( each_node.view == nullptr || HAS_FLAGS(each_node.view->flags, View_Flag_HIDDEN) )
            continue;

        std::vector<Node_Slot *> slots = node_filter_slots(&each_node, Node_Slot::Flag_FLOW_OUT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index)
        {
            Node_Slot *slot = slots[slot_index];

            if ( slot->adjacent.size == 0 )
            {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent)
            {
                Node* successor_node = adjacent_slot->node;

                if ( successor_node->view == nullptr || HAS_FLAGS(successor_node->view->flags, View_Flag_HIDDEN) )
                {
                    continue;
                }

                Node_Slot_View* tail = slot->view;
                Node_Slot_View* head = adjacent_slot->view;

                Vec2 tail_pos = tail->shape.pivot_position(CENTER, WORLD_SPACE);
                Vec2 head_pos = head->shape.pivot_position(CENTER,  WORLD_SPACE);
                Bezier_Curve_Segment_2D segment{
                        tail_pos,
                        tail_pos, 
                        head_pos,
                        head_pos,
                };
                Node_Slot_Link_View linkview{tail, head};
                if( lastframe_hovered_linkview == linkview)
                {
                    float time = ImGui::GetTime();
                    float expansion = wave(1.f, 2.0f, time, 10.f);
                    style.thickness += expansion;
                }

                bool hovered = false;
                ImGuiEx::DrawWire( draw_list, segment, style, &hovered);
                if (hovered)
                    graphview->hovered = linkview;
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
    for (Node& node_out: graphview->graph->nodes )
    {
        for (const Node_Slot* slot_out: node_filter_slots(&node_out, Node_Slot::Flag_OUTPUT))
        {
            for(const Node_Slot* slot_in : slot_out->adjacent)
            {
                if (slot_in == nullptr)
                    continue;

                Node_View* node_view_out = slot_out->node->view;
                Node_View* node_view_in  = slot_in->node->view;

                if ( HAS_FLAGS( node_view_out->flags | node_view_in->flags, View_Flag_HIDDEN) )
                {
                    continue;
                }

                Vec2 p1, cp1, cp2, p2; // BezierCurveSegment's points

                Node_Slot_Link_View linkview{slot_out->view, slot_in->view};

                p1 = linkview.tail->shape.pivot_position(CENTER, WORLD_SPACE);
                p2 = linkview.head->shape.pivot_position(CENTER, WORLD_SPACE);

                const Vec2  signed_dist = Vec2::distance(p1, p2);
                const float lensqr_dist = signed_dist.lensqr();

                // Animate style
                ImGuiEx::WireStyle style = default_wire_style;
                if ( (lastframe_hovered_linkview == linkview)
                || view_selection_contains(&graphview->selection, node_view_out )
                || view_selection_contains(&graphview->selection, node_view_in ) )
                {
                    style.color.w *= wave(0.5f, 1.f, time, 10.f);

                    float time = ImGui::GetTime();
                    float expansion = wave(1.f, 2.0f, time, 10.f);
                    style.thickness += expansion;
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
                if (node_out.type == Node_Type_VARIABLE ) // from a variable
                {
                    if (slot_out == node_out.variable_data.ref_out ) // from a reference slot (can't be a declaration link)
                        if (!HAS_FLAGS(node_view_out->flags, View_Flag_SELECTED) && !HAS_FLAGS(node_view_in->flags, View_Flag_SELECTED) )
                            style.color.w *= 0.25f;
                }

                // draw the wire if necessary
                if (style.color.w != 0.f)
                {
                    // Determine control points
                    float roundness = tools::clamped_lerp(0.f, 10.f, lensqr_dist / 100.f);
                    cp1 = p1;
                    cp2 = p2 + linkview.head->direction * roundness;
                    if ( linkview.tail->direction.y > 0.f ) // round out when direction is bottom
                        cp1 += linkview.tail->direction * roundness;

                    Bezier_Curve_Segment_2D segment{p1, cp1, cp2, p2};

                    bool hovered = false;
                    ImGuiEx::DrawWire(draw_list, segment, style, &hovered);

                    if (hovered)
                    {
                        graphview->hovered = linkview;
                    }
                }
            }
        }
    }

    // Draw Node_Views
    for (Node& node : graphview->graph->nodes  )
    {
        if ( node.view == nullptr || HAS_FLAGS(node.view->flags, View_Flag_HIDDEN) )
        {
            continue;
        }

        changed |= nodeview_draw(node.view);

        if ( HAS_FLAGS( node.view->flags, View_Flag_HOVERED) ) // no check if something else is hovered, last node always win against an edge
        {
            if ( node.view->hovered_slotview != nullptr)
            {
                graphview->hovered = node.view->hovered_slotview;
            }
            else
            {
                graphview->hovered = {node.view};
            }
        }
    }

    graphview->state_machine.tick();

    // Debug Infos
    if ( cfg->tools_cfg->debug_flags )
    {
        if (ImGui::Begin("Graph_ViewToolState_Machine"))
        {
            ImGui::Text("current_tool:       %s", graphview->state_machine.get_current_state_name() );
            ImGui::Text("focused.type:       %i", graphview->focused.type );
            ImGui::Text("hovered.type:       %i", graphview->hovered.type );
            Vec2 mouse_pos = ImGui::GetMousePos();
            ImGui::Text("mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
        }
        ImGui::End();
    }

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    if ( changed )
        graphview->signal_change.emit();


    // debug layout
    if( HAS_FLAGS(cfg->tools_cfg->debug_flags, Debug_Flags_DRAW_LAYOUT_DEBUG_LINES) )
    {
        auto list = ImGui::GetForegroundDrawList();
        Vec2 origin = graphview->shape.position();
        for(Element& el : layout_elements())
        {
            Rect    rect       = element_rect(&el);
            Rect    inner_rect = element_inner_rect(&el);
            ImColor color, padding_color;

            switch ( el.type )
            {
                case tools::Element::Type_CONTAINER:
                {
                    if( el.depth % 2 == 0)
                        color = ImColor(100,255,100);
                    else
                        color = ImColor(255,100,100);
                    break;
                }

                case tools::Element::Type_LEAF:
                {
                    color   = ImColor(255,255,255);
                    break;
                }
                
                default:
                {
                    TOOLS_UNREACHABLE("Unexpected Element_Type: %i\n", el.type);
                }
            } 
            
            // Translate to align with origin
            // We do not use the layout system for the whole UI yet, that's why it's origin is not (0,0)
            rect.translate(origin);
            inner_rect.translate(origin);

            // block fill
            ImColor fill_color = color;
            fill_color.Value.w = 0.05f;
            list->AddRectFilled(rect.min, rect.max, fill_color, 0.0f);

            // inner block fill
            padding_color = ImColor(0.f, 0.f, 1.f, 0.1f);
            list->AddRectFilled(inner_rect.min, inner_rect.max, padding_color, 0.0f, 0);

            // block border
            const float half_thickness = 1.f;
            list->AddRect(rect.min+half_thickness, rect.max-half_thickness, color, 0.0f, 0, half_thickness*2);
        }
    }

	return changed;
}

void ndbl::_graphview_do_layout_element(Graph_View* graphview, Node_View* nodeview )
{
    Config* cfg  = config();
    Rect    rect = nodeview_get_rect(nodeview, tools::PARENT_SPACE);

    layout_append_element(rect.width(), rect.height(), nodeview);

    if( HAS_FLAGS(nodeview->flags, View_Flag_PINNED) )
    {
        layout_set_floating_at_position(rect.top_left());
    }
}

void ndbl::_graphview_do_layout_recursively_on_expressions_only(Graph_View* graphview, Node_View* nodeview )
{
    Config* cfg = config();

    Node*   node = nodeview->node;
    Rect    rect = nodeview_get_rect(nodeview, WORLD_SPACE);

    // Gather all the Node_View(s) that needs to be displayed as a row
    // Since there are some conditions, we have to do this before starting any
    // layout.
    std::vector<Node_View*> input_nodeviews;
    for( Node* input_node : node->inputs() )
    {
        // When the input_node is connected to the code flow, it means it is part of
        // a Scope's backbone, which is handled by _graphview_do_layout_recursively already.
        if (node_is_connected_to_codeflow(input_node))
        {
            continue;
        }

        // When the input_node is from a different scope, we don't constrain it here
        // it is handled by the parent node from the same scope somewhere else in the code.
        if ( input_node->type == Node_Type_VARIABLE && input_node->scope != node->scope )
        {
            continue;
        }

        input_nodeviews.push_back(input_node->view);
    }

    if( input_nodeviews.empty() )
    {
        return _graphview_do_layout_element(graphview, nodeview);
    }

    layout_begin_column();
    {
        layout_set_gap( cfg->ui_node_gap(tools::Size_SM).y );

        layout_begin_row();
        {
            layout_set_gap( cfg->ui_node_gap(Size_SM).x );            
            if( node_is_connected_to_codeflow(node) )
            {
                layout_set_padding( cfg->ui_node_gap(tools::Size_SM).x, 0, 0, 0 );
            }   

            
            for( Node* input_node : node->inputs() )
            {
                // When the input_node is connected to the code flow, it means it is part of
                // a Scope's backbone, which is handled by _graphview_do_layout_recursively already.
                if (node_is_connected_to_codeflow(input_node))
                {
                    continue;
                }

                // When the input_node is from a different scope, we don't constrain it here
                // it is handled by the parent node from the same scope somewhere else in the code.
                if ( input_node->type == Node_Type_VARIABLE && input_node->scope != node->scope )
                {
                    continue;
                }

                _graphview_do_layout_recursively_on_expressions_only(graphview, input_node->view);
            }
        }
        layout_end();

        _graphview_do_layout_element(graphview, nodeview);
    }
    layout_end();
}

void ndbl::_graphview_do_layout_recursively(Graph_View* graphview, Node_View* nodeview )
{
    Config* cfg  = config();
    Node*   node = nodeview->node;

    if( node->internal_scope == nullptr)
    {
        return _graphview_do_layout_recursively_on_expressions_only(graphview, nodeview);
    }

    layout_begin_column();
    {
        // Add a padding to the container, we want each scope to have a little space around to see well the visual feedback (rounded rectangle)
        // of the scope.
        layout_set_padding(cfg->ui_scope_padding);
        layout_set_gap( cfg->ui_node_gap(tools::Size_SM).x );

        if( nodeview->node == graph_root( graphview->graph ) )
        {
            layout_set_floating_at_position(nodeview->shape.position() - Vec2{cfg->ui_scope_padding.left, cfg->ui_scope_padding.top} );
        }

        _graphview_do_layout_recursively_on_expressions_only(graphview, nodeview);

        // propagate on switch branches
        if( node_has_switch_behavior(node))
        {
            if( node->switch_data.branch_count > 1)
            {
                Rect parent_rect = nodeview_get_rect(nodeview);
                layout_begin_row();
                layout_set_padding( cfg->ui_node_gap(tools::Size_SM).x, 0,0,0); // TODO: the container must be centered horizontally, we cannot do that currently with layout
                layout_set_gap( cfg->ui_node_gap(tools::Size_SM).x * 2.f );

                for( Node_Slot* branch_slot : node->switch_data.branch_slots )
                {
                    if ( Node* adjacent_node = branch_slot->first_adjacent_node() )
                    {
                        _graphview_do_layout_recursively(graphview, adjacent_node->view);
                    }
                }
                layout_end();
            }
            else
            {
                TOOLS_UNREACHABLE("Not implemented yet");
            }
        }
        else
        {
            // propagate on internal scope backbone
            for( Node* backbone_node : scope_get_backbone(node->internal_scope) )
            {
                _graphview_do_layout_recursively(graphview, backbone_node->view);
            }
        }
    }
    layout_end();
};

void ndbl::graphview_update(Graph_View* graphview, float dt)
{
    ASSERT( graphview->graph );

    if( graphview->flags & Graph_View_Flag_NEEDS_TO_BE_RESET)
    {
        _graphview_reset(graphview);
        graphview->flags &= ~Graph_View_Flag_NEEDS_TO_BE_RESET;
    }

    // Define a Layout
    layout_begin_frame();
    layout_begin();
    {
        layout_set_floating();        
        if( Node* root_node = graph_root(graphview->graph) )
            if( root_node->view )
                _graphview_do_layout_recursively(graphview, root_node->view);
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
        
        if ( HAS_FLAGS(nodeview->flags, View_Flag_PINNED) )
        {
            // pinned views are positionned by the user and should not be moved
            continue;
        }
        
        spatialnode_set_position(&nodeview->spatial_node(), graphview->shape.position() + elem.position, tools::WORLD_SPACE);
    }
    
    // Update Node_Views
    for (Node& node : graphview->graph->nodes )
        if ( node.view )
            nodeview_update(node.view, dt);

    // Update Scope_Views
    if( Scope* root = graph_root_scope(graphview->graph) )
        if ( root->view != nullptr )
            scopeview_update( root->view, dt, Scope_View_Flag_RECURSE );

    // Frame Content or Selection    
    if( graphview->flags & Graph_View_Flag_NEEDS_TO_FRAME_CONTENT)
    {
        std::vector<View> selected_views{};
        view_selection_collect(&selected_views, &graphview->selection, View_Type_NODE);

        bool has_selection = !selected_views.empty();

        // Select all in case selection is empty
        if ( !has_selection )
            for (Node& node : graphview->graph->nodes )
                if ( node.view )
                    selected_views.push_back(node.view);

        // Get selected node views rectangle
        const Rect selected_rect = view_bounding_rect( selected_views, WORLD_SPACE);

        // Compute the delta to apply to each node
        // We have two different targets depending on if something is selected or not.
        Vec2 delta;

        if( has_selection )
        {
            delta =  graphview->shape.pivot_position( CENTER, WORLD_SPACE) - selected_rect.center();
        }
        else
        {
            Vec2 offset = config()->ui_textview_padding + Vec2{ config()->ui_scope_padding.left, config()->ui_scope_padding.top };
            delta =  graphview->shape.pivot_position( TOP_LEFT, WORLD_SPACE) - selected_rect.top_left() + offset;
        }

        // Apply the delta to all node views
        for (Node& node : graphview->graph->nodes )
            if ( node.view )
                spatialnode_translate( &node.view->shape.spatial_node, delta );

        graphview->flags &= ~Graph_View_Flag_NEEDS_TO_FRAME_CONTENT;
    }    
}

void ndbl::_graphview_on_graph_change(Graph_View* graphview)
{
    // graphview->is_physics_dirty = true;
}

void ndbl::_graphview_on_selection_change(Graph_View* graphview, View_Selection_Event_Type type, View view)
{
    bool selected = type == View_Selection_Event_Type_APPEND;

    switch ( view.type )
    {
        case View_Type_SCOPE:
        {
            SET_FLAGS_VALUE(view.scopeview->flags, View_Flag_SELECTED, selected);
            break;
        }
        case View_Type_NODE:
        {
            SET_FLAGS_VALUE(view.nodeview->flags, View_Flag_SELECTED, selected);
            break;
        }
        case View_Type_LINK:
        {
            break;
        }
        default:
        {
            ASSERT(false); // unhandled case
        }
    }
}

void ndbl::_graphview_reset(Graph_View* graphview)
{
    if ( graph_is_empty(graphview->graph ) )
        return;
    
    for( Scope* scope : graph_collect_scopes(graphview->graph) )
    {
        if ( Scope_View* scopeview = scope->view )
        {
            scopeview->flags = View_Flag_NULL;
        }
    }

    for( Node& node : graphview->graph->nodes )
    {
        if ( node.view )
        {
            node.view->flags = View_Flag_NULL;
            nodeview_reset_all_properties(node.view);
        }
    }
}

bool ndbl::graphview_has_an_active_tool(const Graph_View* graphview)
{
    return !graphview->state_machine.has_default_state();
}

void ndbl::graphview_reset_all_properties(Graph_View* graphview)
{
    for( Node& node : graphview->graph->nodes )
        if ( node.view )
            nodeview_reset_all_properties(node.view);
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_draw_context_menu(Graph_View* graphview, Node_Slot_View* dragged_slotview)
{
    if (Action* triggered_action = nodeview_contextmenu_draw_search_input( &graphview->node_search_input, dragged_slotview, 10))
    {
        ASSERT(triggered_action->event.type == Event_Type_USER);

        // Generate an event from this action, add some info to the state and dispatch it.
        Event event = triggered_action->event;
        auto event_data = static_cast<Event_Data__Create_Node*>(event.user.data1);
        event_data->active_slotview    = dragged_slotview;
        event_data->desired_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        event_manager_push_event( event );
        ImGui::CloseCurrentPopup();
    }
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_drag_state_enter(Graph_View* graphview)
{
    for( const View& selected_view : graphview->selection )
    {
        switch (selected_view.type)
        {
            case View_Type_NODE:    { SET_FLAGS(selected_view.nodeview->flags, View_Flag_PINNED); break; }
            case View_Type_SCOPE:   { SET_FLAGS(selected_view.scopeview->flags, View_Flag_PINNED);break; }
        }
    }
}

void ndbl::_graphview_drag_state_tick(Graph_View* graphview)
{
    const Vec2 delta = ImGui::GetMouseDragDelta();
    ImGui::ResetMouseDragDelta();

    for ( const View& selected_view : graphview->selection )
    {
         switch (selected_view.type)
        {
            case View_Type_NODE:
            {
                spatialnode_translate(&selected_view.nodeview->shape.spatial_node, delta);
                break;
            }
            case View_Type_SCOPE:
            {
                spatialnode_translate(&selected_view.scopeview->scope->node->view->shape.spatial_node, delta);
                break;
            }
        }
    }

    if ( ImGui::IsMouseReleased(0) )
        graphview->state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void ndbl::_graphview_view_pan_state_tick(Graph_View* graphview)
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graphview content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for( Scope* scope : graph_collect_root_scopes(graphview->graph) )
        if ( Node_View* nodeview = scope->node->view )
            spatialnode_translate(&nodeview->shape.spatial_node, delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        graphview->state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_select_scope(Graph_View* graphview, Scope_View* scopeview)
{
    ASSERT(scopeview);

    // 1) Gather Node_Views that are a child (direct or not) of the scopeview

    // Get descendent scopes
    std::set<Scope*> scopes;
    scope_get_descendent(scopes, scopeview->scope, Scope_Flag_INCLUDE_SELF );

    // Extract node views from each descendent
    View_Selection new_selection;
    for(Scope* each_scope : scopes)
    {
        // Include scope owner's view too
        if ( each_scope->node->view )
            view_selection_add(&new_selection, each_scope->node->view );

        // and every other child's
        for( Node* child_node : each_scope->children )
            if ( child_node->view )
                view_selection_add(&new_selection, child_node->view );
    }

    // 2) Replace selection
    Command cmd = command_selection_change(&new_selection);
    command_manager_push_command(cmd);
}

void ndbl::_graphview_cursor_state_tick(Graph_View* graphview)
{
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            graphview->node_search_input.must_be_reset_flag = true;

        switch ( graphview->focused.type )
        {
            case View_Type_NULL:
            {
                _graphview_draw_context_menu(graphview);
                break;
            }

            case View_Type_SCOPE:
            {
                Scope_View* scopeview = graphview->focused.scopeview;
                Node_View*  nodeview  = scopeview->scope->node->view;

                if ( ImGui::MenuItem( "Collapse / Expand") )
                {
                    nodeview_toggle_expandcollapse(nodeview);
                }
                
                if ( ImGui::MenuItem("Select Content") )
                {
                    _graphview_select_scope(graphview, scopeview);
                }

                if ( ImGui::MenuItem("Delete") )
                {
                    // TODO: we should probably handle the selection when we perform a right click on something (slot, node.view, linkview, etc..)
                    view_selection_clear(&graphview->selection);
                    view_selection_add(&graphview->selection, scopeview);

                    event_manager_push_event( event_from_user_data({Event_Type_DELETE}) );
                }


                ImGui::Separator();
                _graphview_draw_context_menu(graphview);

                break;
            }

            case View_Type_LINK:
            {
                auto edge = graphview->focused.linkview;
                if ( ImGui::MenuItem("Delete") )
                {
                    Event event = event_from_user_data({ 
                        Event_Type_DELETE_LINK, 
                        edge.tail->slot, 
                        edge.head->slot
                    });
                    event_manager_push_event( event );
                }

                break;
            }

            case View_Type_SLOT:
            {
                if ( ImGui::MenuItem("Disconnect") )
                {
                    Event event = event_from_user_data({ 
                        Event_Type_DELETE_ALL_LINKS, 
                        graphview->focused.slotview->slot, 
                        nullptr
                    });

                    event_manager_push_event( event );
                }

                break;
            }

            case View_Type_NODE:
            {
                Node_View* nodeview = graphview->focused.nodeview;

                if ( ImGui::MenuItem( "Collapse / Expand") )
                {
                    nodeview_toggle_expandcollapse(nodeview);
                }

                if ( ImGui::MenuItem("Pin / Unpin") )
                {
                    TOGGLE_FLAGS( nodeview->flags, View_Flag_PINNED );
                }

                if ( ImGui::MenuItem("Reset Layout") )
                {
                    nodeview_arrange_recursively(nodeview);
                    _graphview_reset(graphview);
                }

                if ( ImGui::MenuItem("Delete") )
                {
                    view_selection_clear(&graphview->selection);
                    view_selection_add(&graphview->selection, nodeview);

                    event_manager_push_event(event_from_user_data({Event_Type_DELETE}));
                }

                break;
            }
        }

        ImGui::EndPopup();

        // When we're focused on something with popup open, we don't want to do things based on _m_hovered.type (see below)
        if ( graphview->focused.type != View_Type_NULL )
            return;
    }

    switch ( graphview->hovered.type )
    {
        case View_Type_SLOT:
        {
            if ( ImGui::IsMouseClicked(1) )
            {
                graphview->focused = graphview->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if (ImGui::IsMouseClicked(0))
            {
                graphview->focused = graphview->hovered;
                graphview->state_machine.change_state(LINE_STATE);
            }
            break;
        }

        case View_Type_LINK:
        {
            if (ImGui::IsMouseDragging(0, 0.1f))
            {
                graphview->focused = graphview->hovered;
            }
            else if (ImGui::IsMouseClicked(1))
            {
                graphview->focused = graphview->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            break;
        }

        case View_Type_SCOPE:
        {
            if ( ImGui::IsMouseDoubleClicked(0) )
            {
                _graphview_select_scope(graphview, graphview->hovered.scopeview);
            }
            else if (ImGui::IsMouseClicked(1))
            {
                graphview->focused = graphview->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            break;
        }

        case View_Type_NODE:
        {
            const bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);

            if ( ImGui::IsMouseReleased(0) )
            {
                if ( ctrl_pressed )
                {
                    if ( !view_selection_contains(&graphview->selection, graphview->hovered ) )
                    {
                        view_selection_add( &graphview->selection, graphview->hovered );
                        graphview->focused = graphview->hovered;
                    }
                    else
                    {
                        view_selection_remove( &graphview->selection, graphview->hovered );
                    }
                }
                else
                {
                    view_selection_clear( &graphview->selection );
                    view_selection_add( &graphview->selection, graphview->hovered );
                    graphview->focused = graphview->hovered;
                }
            }
            else if (ImGui::IsMouseClicked(1))
            {
                graphview->focused = graphview->hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            else if ( ImGui::IsMouseClicked(0) )
            {
                if ( !view_selection_contains( &graphview->selection, graphview->hovered) )
                {
                    if ( !ctrl_pressed )
                        view_selection_clear( &graphview->selection );
                    view_selection_add(&graphview->selection, graphview->hovered );
                }
                graphview->state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case View_Type_NULL:
        {
            if ( ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
            {
                if (ImGui::IsMouseClicked(0))
                    view_selection_clear(&graphview->selection); // Deselect All (Click on the background)
                else if (ImGui::IsMouseReleased(0))
                    graphview->focused = {};
                else if (ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup(CONTEXT_POPUP);
                else if (ImGui::IsMouseDragging(0))
                {
                    if (ImGui::IsKeyDown(ImGuiKey_Space))
                        graphview->state_machine.change_state(VIEW_PAN_STATE);
                    else
                        graphview->state_machine.change_state(ROI_STATE);
                }
            }

            break;
        }

        default:
            VERIFY(false, "Unhandled case, must be implemented!");
    }
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_line_state_enter(Graph_View* graphview)
{
    ASSERT( graphview->focused.type == View_Type_SLOT );
}

void ndbl::_graphview_line_state_tick(Graph_View* graphview)
{
    Vec2 mouse_pos_snapped = Vec2{ImGui::GetMousePos()};
    if ( graphview->hovered.type == View_Type_SLOT )
    {
        mouse_pos_snapped = spatialnode_position(&graphview->hovered.slotview->shape.spatial_node, WORLD_SPACE);
    }

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            graphview->node_search_input.must_be_reset_flag = true;

        if ( graphview->hovered.type == View_Type_NULL )
            _graphview_draw_context_menu(graphview, graphview->focused.slotview );

        if ( ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
            graphview->state_machine.exit_state();

        ImGui::EndPopup();
    }
    else if ( ImGui::IsMouseReleased(0) )
    {
        if ( graphview->hovered.type == View_Type_SLOT )
        {
            if ( graphview->focused != graphview->hovered )
            {
                
                Event event = event_from_user_data({
                    Event_Type_SLOT_DROPPED_ONTO_ANOTHER,
                    graphview->focused.slotview->slot,
                    graphview->hovered.slotview->slot
                });
                event_manager_push_event(event);
                graphview->state_machine.exit_state();
            }
        }
        else
        {
            ImGui::OpenPopup(CONTEXT_POPUP);
        }
    }

    // Draw a temporary wire from focused/dragged slotview to the mouse cursor
    ndbl::_graphview_draw_wire_from_slot_to_pos(graphview, graphview->focused.slotview, mouse_pos_snapped );
}

void ndbl::_graphview_line_state_leave(Graph_View* graphview)
{
    graphview->focused = {};
}

//-----------------------------------------------------------------------------

void ndbl::_graphview_roi_state_enter(Graph_View* graphview)
{
    graphview->state_roi_start_pos = ImGui::GetMousePos();
    graphview->state_roi_end_pos   = ImGui::GetMousePos();
}

void ndbl::_graphview_roi_state_tick(Graph_View* graphview)
{
    graphview->state_roi_end_pos = ImGui::GetMousePos();

    // Get normalized ROI rectangle
    Rect roi = Rect::normalize({graphview->state_roi_start_pos, graphview->state_roi_end_pos});

    // Expand to avoid null area
    const int roi_border_width = 1;
    roi.expand(roi_border_width);

    // Draw the ROI rectangle
    float time = ImGui::GetTime();
    float alpha = wave(0.5f, 0.75f, time, 10.f);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll , roi_border_width );

    if (ImGui::IsMouseReleased(0))
    {
        // Get the views included in the ROI
        // Note: this might be expensive with a lots of nodes, we will need some sort of space partitionning in the future.
        std::vector<View> nodeviews_inside_roi;
        for ( Node& node : graphview->graph->nodes )
            if ( node.view )
                if ( Rect::contains(roi, nodeview_get_rect(node.view, tools::WORLD_SPACE)) )
                    nodeviews_inside_roi.push_back( node.view );

        // Select them
        const bool ctrl_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        if ( !ctrl_pressed )
            view_selection_clear(&graphview->selection);
        view_selection_add(&graphview->selection, nodeviews_inside_roi );

        graphview->state_machine.exit_state();
    }
}

void ndbl::_graphview_handle_hover(Graph_View* graphview, Scope_View* scope_view)
{
    if ( graphview->hovered.type != View_Type_SCOPE || graphview->hovered.type == View_Type_NULL )
        graphview->hovered = scope_view;
    else if ( scopeview_get_depth( scope_view ) >= scopeview_get_depth( graphview->hovered.scopeview ) )
        graphview->hovered = scope_view;
}

std::vector<Node_View*> get_clean_views(std::vector<Node_View*>& possibly_hidden_views)
{
    std::vector<Node_View*> result;
    for(Node_View* view : possibly_hidden_views)
        if ( !HAS_FLAGS(view->flags, View_Flag_HIDDEN | tools::View_Flag_PINNED))
            result.push_back(view);
    return std::move(result);
}
