#include "GraphView.h"

#include <algorithm>
#include "tools/core/types.h"
#include "tools/core/log.h"
#include "tools/core/system.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/core/math.h"
#include "tools/core/Color.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/VirtualMachine.h"

#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "SlotView.h"

using namespace ndbl;
using namespace tools;

const char* k_CONTEXT_MENU_POPUP = "GraphView.ContextMenuPopup";

REFLECT_STATIC_INIT
{
    StaticInitializer<GraphView>("GraphView").extends<View>();
}

GraphView::GraphView(Graph* graph)
    : View()
    , m_graph(graph)
{
}

bool GraphView::onDraw()
{
    Config*         cfg              = get_config();
    VirtualMachine* virtual_machine  = get_virtual_machine();
    bool            changed          = false;
    bool            pixel_perfect    = true;
    ImDrawList*     draw_list        = ImGui::GetWindowDrawList();
    const bool      enable_edition   = virtual_machine->is_program_stopped();
    auto            node_registry    = get_pool_manager()->get_pool()->get( m_graph->get_node_registry() );
    FrameState      curr_frame;

    auto make_wire_id = [](const Slot* ptr1, const Slot* ptr2) -> ImGuiID
    {
        string128 id;
        id.append_fmt("wire %zu->%zu", ptr1, ptr2);
        return ImGui::GetID(id.c_str());
    };


    // Draw grid in the background
    draw_grid( draw_list );

    /*
       Draw Code Flow.
       Code flow is the set of green lines that links  a set of nodes.
     */

    const ImGuiEx::WireStyle code_flow_style{
        cfg->ui_codeflow_color,
        cfg->ui_codeflow_color, // hover
        cfg->ui_codeflow_shadowColor,
        cfg->ui_codeflow_thickness(),
        0.0f
    };

    for( Node* each_node : node_registry )
    {
        NodeView *each_view = NodeView::substitute_with_parent_if_not_visible( each_node->get_component<NodeView>().get() );

        if ( !each_view )
        {
            continue;
        }

        std::vector<Slot*> slots = each_node->filter_slots(SlotFlag_NEXT);
        for ( size_t slot_index = 0; slot_index < slots.size(); ++slot_index  )
        {
            Slot* slot = slots[slot_index];

            if( slot->empty() )
            {
                continue;
            }

            for( const auto& adjacent_slot : slot->adjacent() )
            {
                Node* each_successor_node = adjacent_slot->get_node();
                NodeView* each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->get_component<NodeView>().get() );

                if ( each_successor_view && each_view->is_visible && each_successor_view->is_visible )
                {
                    Rect start = each_view->get_slot_rect( *slot, slot_index );
                    Rect end = each_successor_view->get_slot_rect( *adjacent_slot, 0 );// there is only 1 previous slot

                    ImGuiID id = make_wire_id( slot, adjacent_slot.get());
                    ImGuiEx::DrawVerticalWire(
                            id,
                            ImGui::GetWindowDrawList(),
                            start.center(),
                            end.center(),
                            code_flow_style );
                    if ( ImGui::GetHoveredID() == id)
                    {
                        curr_frame.is_any_wire_hovered = true;
                        curr_frame.hovered_wire_start = slot;
                        curr_frame.hovered_wire_end = adjacent_slot.get();
                    }
                }
            }
        }
    }

    // slot Drag'n Drop
    if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
    {
        // Get the current dragged slot, or the slot that was dragged when context menu opened
        SlotView* _dragged_slot = m_dragged_slotview ? m_dragged_slotview : m_create_node_menu.dragged_slot;

        // Draw temporary edge
        if ( _dragged_slot )
        {
            ImGuiEx::WireStyle hovered_wire_style;
            hovered_wire_style.color = cfg->ui_codeflow_color,
            hovered_wire_style.shadow_color = cfg->ui_codeflow_shadowColor,
            hovered_wire_style.thickness = cfg->ui_slot_size.x * cfg->ui_codeflow_thickness_ratio,
            hovered_wire_style.roundness = 0.f;

            // When dragging, edge follows mouse cursor. Otherwise, it sticks the contextual menu.
            Vec2 edge_end = m_create_node_menu.dragged_slot
                                  ? m_create_node_menu.opened_at_screen_pos
                                  : (Vec2) ImGui::GetMousePos();

            if ( _dragged_slot->slot().type() == SlotFlag_TYPE_CODEFLOW )
            {
                // Thick line
                string128 id;
                id.append_fmt( "wire %zu->mouse", m_dragged_slotview );
                ImGuiEx::DrawVerticalWire(
                        ImGui::GetID( id.c_str() ),
                        ImGui::GetWindowDrawList(),
                        _dragged_slot->get_rect().center(),
                        m_hovered_slotview ? m_hovered_slotview->get_rect().center() : edge_end,
                        hovered_wire_style );
            }
            else
            {
                // Simple line
                ImGui::GetWindowDrawList()->AddLine(
                        _dragged_slot->position(),
                        m_hovered_slotview ? m_hovered_slotview->position() : edge_end,
                        ImGui::ColorConvertFloat4ToU32( cfg->ui_node_borderHighlightedColor ),
                        cfg->ui_wire_bezier_thickness );
            }
        }

        // Determine whether the current dragged SlotView should be dropped or not, and if a new node is required
        if ( m_dragged_slotview && ImGui::IsMouseReleased( 0 ) )
        {
            if ( enable_edition )
            {
                if ( !m_hovered_slotview )
                {
                    curr_frame.slotview_dropped_on_background = true;
                }
                else
                {
                    auto& event_manager = EventManager::get_instance();
                    event_manager.dispatch<Event_SlotDropped>( { m_dragged_slotview->slot(), m_hovered_slotview->slot() } );
                }
            }
            m_dragged_slotview = nullptr;
        }
    }

	{
        /*
            Wires
        */
        const ImGuiEx::WireStyle default_wire_style{
            cfg->ui_wire_color,
            cfg->ui_wire_color, // hover
            cfg->ui_wire_shadowColor,
            cfg->ui_wire_bezier_thickness,
            cfg->ui_wire_bezier_roundness.x // roundness min
        };

        for (auto each_node: node_registry )
        {
            for (const Slot* slot: each_node->filter_slots( SlotFlag_OUTPUT ))
            {
                ImGuiEx::WireStyle style = default_wire_style;

                Slot* adjacent_slot = slot->first_adjacent().get();
                if( adjacent_slot == nullptr )
                {
                    continue;
                }
                NodeView* node_view          = slot->node->get_component<NodeView>().get();
                NodeView* adjacent_node_view = adjacent_slot->node->get_component<NodeView>().get();

                if ( !node_view->is_visible || !adjacent_node_view->is_visible)
                {
                    continue;
                }

                Vec2 slot_pos           = node_view->get_slot_pos( *slot );
                Vec2 slot_norm          = node_view->get_slot_normal( *slot );
                Vec2 adjacent_slot_pos  = adjacent_node_view->get_slot_pos( *adjacent_slot );
                Vec2 adjacent_slot_norm = adjacent_node_view->get_slot_normal( *adjacent_slot );
                float linear_dist       = Vec2::distance(slot_pos, adjacent_slot_pos);
                float linear_dist_half  = linear_dist * 0.5f;

                BezierCurveSegment curve{
                    slot_pos,
                    slot_pos + slot_norm * linear_dist_half,
                    adjacent_slot_pos + adjacent_slot_norm * linear_dist_half,
                    adjacent_slot_pos
                };

                // do not draw long lines between a variable value
                if ( is_selected( node_view->poolid() ) ||
                     is_selected( adjacent_node_view->poolid() ) )
                {
                    // blink wire colors
                    float blink = 1.f + std::sin(float( BaseApp::elapsed_time()) * 10.f) * 0.25f;
                    style.color.w *= blink;
                }
                else
                {
                    // transparent depending on wire length
                    if ( linear_dist > cfg->ui_wire_bezier_fade_length_minmax.x )
                    {
                        float factor = ( linear_dist - cfg->ui_wire_bezier_fade_length_minmax.x ) /
                                       ( cfg->ui_wire_bezier_fade_length_minmax.y - cfg->ui_wire_bezier_fade_length_minmax.x );
                        style.color        = Vec4::lerp(style.color, Vec4(0, 0, 0, 0), factor);
                        style.shadow_color = Vec4::lerp(style.shadow_color, Vec4(0, 0, 0, 0), factor);
                    }
                }

                // draw the wire if necessary
                if (style.color.w != 0.f)
                {
                    style.roundness = lerp(
                            cfg->ui_wire_bezier_roundness.x, // min
                            cfg->ui_wire_bezier_roundness.y, // max
                              1.0f - normalize( linear_dist, 100.0f, 10000.0f )
                            + 1.0f - normalize( linear_dist, 0.0f, 200.0f)
                    );

                    if ( slot->has_flags(SlotFlag_TYPE_CODEFLOW) )
                    {
                        style.thickness *= 3.0f;
                        // style.roundness *= 0.25f;
                    }

                    ImGuiID id = make_wire_id( slot, adjacent_slot );
                    ImGuiEx::DrawWire(id, draw_list, curve, style);

                    // TODO: this block is repeated twice
                    if ( ImGui::GetHoveredID() == id )
                    {
                        curr_frame.is_any_wire_hovered = true;
                        curr_frame.hovered_wire_start  = slot;
                        curr_frame.hovered_wire_end    = adjacent_slot;
                    }
                }
            }
        }

        /*
            NodeViews
        */
		for ( NodeView* node_view: NodeUtils::get_components<NodeView>( m_graph->get_node_registry() ) )
		{
            if ( !node_view->is_visible)
                continue;

            changed |= node_view->draw();

            if ( node_view->is_hovered )
            {
                EXPECT( curr_frame.hovered_node == nullptr, "Multiple items are hovered, forbidden, we should decided what to do.");
                curr_frame.hovered_node = node_view;

                // Selection by mouse (left or right click)
                if ( node_view->is_hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
                {
                    bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                           ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                    SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                          : SelectionMode_REPLACE;
                    set_selected( node_view->m_id, flags);
                }
            }


            // Mouse dragging
            if ( !is_dragged(node_view->m_id) )
            {
                if( m_dragged_nodeview.empty() && m_dragged_slotview == nullptr && ImGui::IsMouseDown(0) && node_view->is_hovered && ImGui::IsMouseDragPastThreshold(0))
                {
                    m_dragged_nodeview.clear();
                    m_dragged_nodeview.emplace_back(node_view->m_id );
                }
            }

            // Collapse on/off
            if( node_view->is_hovered && ImGui::IsMouseDoubleClicked(0))
            {
                node_view->expand_toggle();
            }

            if( virtual_machine->is_debugging() && virtual_machine->is_next_node( node_view->get_owner() ) )
            {
                ImGui::SetScrollHereY();
            }

            // dragging
            if ( is_dragged(node_view->m_id) && ImGui::IsMouseDragging(0) )
            {
                Vec2 mouse_drag_delta = ImGui::GetMouseDragDelta();
                node_view->translate(mouse_drag_delta, true);
                ImGui::ResetMouseDragDelta();
                node_view->pinned( true );
            }

            curr_frame.is_any_node_hovered |= node_view->is_hovered;
        }
	}

	// Virtual Machine cursor
    if ( virtual_machine->is_program_running() )
    {
        const Node* node = virtual_machine->get_next_node();
        if( NodeView* view = node->get_component<NodeView>().get() )
        {
            Vec2 left = view->rect( WORLD_SPACE ).left();
            Vec2 vm_cursor_pos = Vec2::round( left );
            draw_list->AddCircleFilled( vm_cursor_pos, 5.0f, ImColor(255,0,0) );

            Vec2 linePos = vm_cursor_pos + Vec2(- 10.0f, 0.5f);
            linePos += Vec2(sin(float( BaseApp::elapsed_time()) * 12.0f ) * 4.0f, 0.f ); // wave
            float size = 20.0f;
            float width = 2.0f;
            ImColor color = ImColor(255,255,255);

            // arrow ->
            draw_list->AddLine( linePos - Vec2(1.f, 0.0f), linePos - Vec2(size, 0.0f), color, width);
            draw_list->AddLine( linePos, linePos - Vec2(size * 0.5f, -size * 0.5f), color, width);
            draw_list->AddLine( linePos, linePos - Vec2(size * 0.5f, size * 0.5f) , color, width);
        }
    }

	/*
		Deselection (by double click)
	*/
	if ( !m_selected_nodeview.empty())
	    if ( !curr_frame.is_any_node_hovered)
	        if ( ImGui::IsMouseDoubleClicked(0) )
	            if ( ImGui::IsWindowFocused())
                    m_selected_nodeview.clear();

    // Dragging STOP
    if ( ImGui::IsMouseReleased(0))
        m_dragged_nodeview.clear();

	// Mouse PAN (global)
    bool is_mouse_dragging_background = ImGui::IsMouseDragging(0) &&
                                        ImGui::IsWindowFocused() &&
                                        m_dragged_nodeview.empty() &&
                                        !curr_frame.is_any_wire_dragged;
	if (is_mouse_dragging_background)
    {
        pan( ImGui::GetMouseDragDelta() );
        ImGui::ResetMouseDragDelta();
    }

    // Decides whether contextual menu should be opened.
    bool right_click_on_bg = enable_edition && !curr_frame.is_any_node_hovered && ImGui::IsMouseClicked(1);
    if ( curr_frame.slotview_dropped_on_background || right_click_on_bg )
        if ( !ImGui::IsPopupOpen( k_CONTEXT_MENU_POPUP ) )
        {
            if ( ImGui::IsWindowHovered() )
                ImGui::OpenPopup( k_CONTEXT_MENU_POPUP );

            m_create_node_menu.reset_state( m_dragged_slotview );
            m_dragged_slotview = nullptr;
        }

    // Defines contextual menu popup (not rendered if popup is closed)
	if ( ImGui::BeginPopup( k_CONTEXT_MENU_POPUP ) )
    {
        // 1) Actions relative to hovered wire
        if ( curr_frame.is_any_wire_hovered )
        {
            if ( ImGui::Button("Delete Edge") )
            {
                LOG_MESSAGE("GraphView", "Delete Edge Button clicked!\n");
                // Generate an event from this action, add some info to the state and dispatch it.
                auto& event_manager = EventManager::get_instance();
                auto* event = new Event_DeleteEdge();
                event->data.first = *curr_frame.hovered_wire_start;
                event->data.second = *curr_frame.hovered_wire_end;
                event_manager.dispatch(event);

                ImGui::CloseCurrentPopup();
            }
        }

        // 2) Actions relative to currently focused node

        if ( NodeView* hovered_view = curr_frame.hovered_node )
        {
            if ( ImGui::MenuItem( "Arrange" ) )
            {
                hovered_view->arrange_recursively();
            }

            ImGui::MenuItem( "Pinned", "", &hovered_view->m_pinned, true );

            if ( ImGui::MenuItem( "Expanded", "", &hovered_view->m_expanded, true ) )
            {
                hovered_view->set_expanded( hovered_view->m_expanded );
            }

            ImGui::Separator();

            auto flags = enable_edition ? ImGuiSelectableFlags_None
                                        : ImGuiSelectableFlags_Disabled;
            if ( ImGui::Selectable( "Delete", flags ) )
            {
                hovered_view->get_owner()->flagged_to_delete = true;
            }
        }

        // 3) Actions relative to node creation

        // Title :
        ImGuiEx::ColoredShadowedText( Vec2( 1, 1 ), Color( 0, 0, 0, 255 ), Color(255, 255, 255, 127 ), "Create new node :" );
        ImGui::Separator();
        /*
        *  In case user has created a new node we need to connect it to the graph depending
        *  on if a slot is being dragged and  what is its nature.
        */
        if ( Action_CreateNode* action = m_create_node_menu.draw_search_input( 10 ) )
        {
            // Generate an event from this action, add some info to the state and dispatch it.
            auto& event_manager = EventManager::get_instance();
            auto* event = action->make_event();
            event->data.graph               = m_graph;
            event->data.dragged_slot        = m_create_node_menu.dragged_slot;
            event->data.node_view_local_pos = m_create_node_menu.opened_at_pos;
            event_manager.dispatch(event);

            ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

void GraphView::draw_grid( ImDrawList* draw_list ) const
{
    Config* cfg = get_config();
    Rect area = ImGuiEx::GetContentRegion(WORLD_SPACE);
    int  grid_subdiv_size      = cfg->ui_grid_subdiv_size();
    int  vertical_line_count   = int( area.size().x) / grid_subdiv_size;
    int  horizontal_line_count = int( area.size().y) / grid_subdiv_size;
    Vec4 grid_color            = cfg->ui_graph_grid_color_major;
    Vec4 grid_color_light      = cfg->ui_graph_grid_color_minor;

    for(int coord = 0; coord <= vertical_line_count; ++coord)
    {
        float pos = area.tl().x + float(coord) * float(grid_subdiv_size);
        Vec2 line_start{pos, area.tl().y};
        Vec2 line_end{pos, area.bl().y};
        bool is_major = coord % cfg->ui_grid_subdiv_count == 0;
        ImColor color{ is_major ? grid_color : grid_color_light };
        draw_list->AddLine(line_start, line_end, color);
    }

    for(int coord = 0; coord <= horizontal_line_count; ++coord)
    {
        float pos = area.tl().y + float(coord) * float(grid_subdiv_size);
        Vec2 line_start{ area.tl().x, pos};
        Vec2 line_end{ area.br().x, pos};
        bool is_major = coord % cfg->ui_grid_subdiv_count == 0;
        ImColor color{is_major ? grid_color : grid_color_light};
        draw_list->AddLine(line_start, line_end, color);
    }
}

bool GraphView::update(float delta_time, i16_t subsample_count)
{
    const float subsample_delta_time = delta_time / float(subsample_count);
    for(i16_t i = 0; i < subsample_count; i++)
        update( subsample_delta_time );
    return true;
}

bool GraphView::update(float delta_time)
{
    // 1. Update Physics Components
    std::vector<Physics*> physics_components = NodeUtils::get_components<Physics>( m_graph->get_node_registry() );
    // 1.1 Apply constraints (but apply no translation, we want to be sure order does no matter)
    for (auto physics_component : physics_components)
    {
        physics_component->apply_constraints(delta_time);
    }
    // 1.3 Apply forces (translate views)
    for(auto physics_component : physics_components)
    {
        physics_component->apply_forces(delta_time, false);
    }

    // 2. Update NodeViews
    std::vector<NodeView*> nodeview_components = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    for (auto eachView : nodeview_components)
    {
        eachView->update(delta_time);
    }

    return true;
}

bool GraphView::update()
{
    Config* cfg = get_config();
    return update( ImGui::GetIO().DeltaTime, cfg->ui_node_animation_subsample_count );
}

void GraphView::frame_all_node_views()
{
    Node* root = m_graph->get_root().get();
    if(!root)
    {
        return;
    }
    // frame the root's view (top-left corner)
    auto root_view = root->get_component<NodeView>().get();
    frame_views( {root_view}, true);
}

void GraphView::frame_selected_node_views()
{
    if( !m_selected_nodeview.empty() )
    {
        std::vector<NodeView*> views = get_pool_manager()->get_pool()->get( m_selected_nodeview );
        frame_views(views, false);
    }
}

void GraphView::frame_views(const std::vector<NodeView*>& _views, bool _align_top_left_corner)
{
    if (_views.empty())
    {
        LOG_VERBOSE("GraphView", "Unable to frame views vector. Reason: is empty.\n")
        return;
    }
    Rect frame = parent_content_region.rect();

    // get selection rectangle
    Rect selection = NodeView::get_rect(_views, WORLD_SPACE);

    // debug
    ImGuiEx::DebugRect( selection.min, selection.max, IM_COL32(0, 255, 0, 127 ), 5.0f );
    ImGuiEx::DebugRect( frame.min, frame.max, IM_COL32( 255, 255, 0, 127 ), 5.0f );

    // align
    Vec2 move;
    if (_align_top_left_corner)
    {
        // Align with the top-left corner
        selection.expand( Vec2( 20.0f ) ); // add a padding to avoid alignment too close from the border
        move = frame.tl() - selection.tl();
    }
    else
    {
        // Align the center of the node rectangle with the frame center
        move = frame.center() - selection.center();
    }

    // apply the translation
    // TODO: Instead of applying a translation to all views, we could translate a Camera.
    auto node_views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all( node_views, move, NodeViewFlag_NONE);

    // debug
    ImGuiEx::DebugLine( selection.center(), selection.center() + move, IM_COL32(255, 0, 0, 255 ), 20.0f);
}

void GraphView::translate_all(const std::vector<NodeView*>& _views, Vec2 delta, NodeViewFlags flags )
{
    for (auto node_view : _views )
    {
        node_view->translate(delta, flags);
    }
}

void GraphView::unfold()
{
    Config* cfg = get_config();
    update( cfg->graph_unfold_dt, cfg->graph_unfold_iterations );
}

void GraphView::pan( Vec2 delta)
{
    auto views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(views, delta, NodeViewFlag_NONE);

    // TODO: implement a better solution, storing an offset. And then substract it in draw();
    // m_view_origin += delta;
}

void GraphView::add_action_to_context_menu( Action_CreateNode* _action )
{
    m_create_node_menu.items.push_back(_action);
}

void GraphView::frame( FrameMode mode )
{
    // TODO: use an rect instead of a FrameMode enum, it will be easier to handle undo/redo
    if ( mode == FRAME_ALL )
    {
        return frame_all_node_views();
    }
    return frame_selected_node_views();
}

Action_CreateNode* CreateNodeContextMenu::draw_search_input( size_t _result_max_count )
{
    bool validated;

    if ( must_be_reset_flag )
    {
        ImGui::SetKeyboardFocusHere();

        //
        update_cache_based_on_signature();

        // Initial search
        update_cache_based_on_user_input( 100 );

        // Ensure we reset once
        must_be_reset_flag = false;
    }

    // Draw search input and update_cache_based_on_user_input on input change
    if ( ImGui::InputText("Search", search_input, 255, ImGuiInputTextFlags_EscapeClearsAll ))
    {
        update_cache_based_on_user_input( 100 );
    }

    if ( !items_matching_search.empty() )
    {
        // When a single item is filtered, pressing enter will press the item's button.
        if ( items_matching_search.size() == 1)
        {
            auto action = items_matching_search.front();
            if ( ImGui::SmallButton( action->label.c_str()) || ImGui::IsKeyDown( ImGuiKey_Enter ) )
            {
                return action;
            }
        }
        else
        {
            size_t more = items_matching_search.size() > _result_max_count ? items_matching_search.size() : 0;
            if ( more )
            {
                ImGui::Text("Found %zu result(s)", items_matching_search.size() );
            }
            // Otherwise, user has to move with arrow keys and press enter to trigger the highlighted button.
            auto it = items_matching_search.begin();
            while( it != items_matching_search.end() && std::distance(items_matching_search.begin(), it) != _result_max_count)
            {
                auto* action = *it;
                if ( ImGui::Button( action->label.c_str()) || // User can click on the button...
                     (ImGui::IsKeyDown( ImGuiKey_Enter ) && ImGui::IsItemFocused() ) // ...or press enter if this item is the first
                )
                {
                    return action;
                }
                it++;
            }
            if ( more )
            {
                ImGui::Text(".. %zu more ..", more );
            }
        }
    }
    else
    {
        ImGui::Text("No matches...");
    }

    return nullptr;
}
void CreateNodeContextMenu::update_cache_based_on_signature()
{
    items_with_compatible_signature.clear();

    // 1) When NO slot is dragged
    //---------------------------

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        items_with_compatible_signature = items;
        return;
    }

    // 2) When a slot is dragged
    //--------------------------

    for (auto& action: items )
    {
        const type* dragged_property_type = dragged_slot->get_property_type();

        switch ( action->event_data.node_type )
        {
            case NodeType_BLOCK_CONDITION:
            case NodeType_BLOCK_FOR_LOOP:
            case NodeType_BLOCK_WHILE_LOOP:
            case NodeType_BLOCK_SCOPE:
            case NodeType_BLOCK_PROGRAM:
                // Blocks are only for code flow slots
                if ( !dragged_slot->allows(SlotFlag_TYPE_CODEFLOW) )
                    continue;
                break;

            default:

                if ( dragged_slot->allows(SlotFlag_TYPE_CODEFLOW))
                {
                    // we can connect anything to a code flow slot
                }
                else if ( dragged_slot->allows(SlotFlag_INPUT) && dragged_slot->get_property_type()->is<PoolID<Node>>() )
                {
                    // we can connect anything to a Node ref input
                }
                else if ( action->event_data.node_signature )
                {
                    // discard incompatible signatures

                    if ( dragged_slot->allows( SlotFlag_ORDER_FIRST ) &&
                         !action->event_data.node_signature->has_an_arg_of_type(dragged_property_type)
                       )
                        continue;

                    if ( !action->event_data.node_signature->get_return_type()->equals(dragged_property_type) )
                        continue;

                }
        }
        items_with_compatible_signature.push_back( action );
    }
}

void CreateNodeContextMenu::update_cache_based_on_user_input( size_t _limit )
{
    items_matching_search.clear();
    for ( auto& menu_item : items_with_compatible_signature )
    {
        if( menu_item->label.find( search_input ) != std::string::npos )
        {
            items_matching_search.push_back(menu_item);
            if ( items_matching_search.size() == _limit )
            {
                break;
            }
        }
    }
}

void CreateNodeContextMenu::reset_state( SlotView* _dragged_slot )
{
    must_be_reset_flag   = true;
    search_input[0]      = '\0';
    opened_at_pos        = (Vec2)(ImGui::GetMousePos() - ImGui::GetCursorScreenPos());
    opened_at_screen_pos = ImGui::GetMousePos();
    dragged_slot         = _dragged_slot;

    items_matching_search.clear();
    items_with_compatible_signature.clear();
}

void GraphView::set_selected( PoolID<NodeView> new_selection, SelectionMode mode )
{
    if( is_selected( new_selection ) )
        return;

    NodeViewVec old_selected = m_selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        for(auto& each : m_selected_nodeview )
        {
            each->m_is_selected = false;
        }
        m_selected_nodeview.clear();
    }

    m_selected_nodeview.emplace( m_selected_nodeview.begin(), new_selection );
    new_selection->m_is_selected = true;
    EventPayload_NodeViewSelectionChange event{ m_selected_nodeview, old_selected };
    EventManager& event_manager = EventManager::get_instance();
    event_manager.dispatch<Event_SelectionChange>(event);
}

const GraphView::NodeViewVec& GraphView::get_selected() const
{
    return m_selected_nodeview;
}

bool GraphView::is_any_dragged() const
{
    return !m_dragged_nodeview.empty();
}

const GraphView::NodeViewVec& GraphView::get_dragged() const
{
    return m_dragged_nodeview;
}

bool GraphView::is_selected(PoolID<NodeView> view) const
{
    return std::find( m_selected_nodeview.begin(), m_selected_nodeview.end(), view) != m_selected_nodeview.end();
}

bool GraphView::is_dragged(PoolID<NodeView> view) const
{
    return std::find( m_dragged_nodeview.begin(), m_dragged_nodeview.end(), view) != m_dragged_nodeview.end();
}

void GraphView::set_hovered_slotview(SlotView* view)
{
    m_hovered_slotview = view;
}

SlotView *GraphView::get_dragged_slotview() const
{
    return m_dragged_slotview;
}

void GraphView::set_dragged_slotview(SlotView *view)
{
    ASSERT(m_dragged_slotview == nullptr || view == nullptr)
    m_dragged_slotview = view;
}

SlotView *GraphView::get_hovered_slotview() const
{
    return m_dragged_slotview;
}
