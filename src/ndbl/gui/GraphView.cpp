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
#include "ndbl/core/ComponentFactory.h"

using namespace ndbl;
using namespace tools;

const char* k_CONTEXT_MENU_POPUP = "GraphView.ContextMenuPopup";

REFLECT_STATIC_INIT
{
    StaticInitializer<GraphView>("GraphView").extends<View>();
}

GraphView::GraphView(Graph* graph, View* parent)
    : View(parent)
    , m_graph(graph)
{
    ASSERT(graph)

    // When a new node is added
    graph->on_add.connect(
        [this](Node* node) -> void
        {
            // Add a NodeView and Physics component
            ComponentFactory* component_factory = get_component_factory();
            NodeView* new_view_id = component_factory->create<NodeView>( this );
            Physics* physics_id   = component_factory->create<Physics>( new_view_id );
            node->add_component( new_view_id );
            node->add_component( physics_id );
        }
    );
}

bool GraphView::draw()
{
    View::draw();

    Config*         cfg                    = get_config();
    VirtualMachine* virtual_machine        = get_virtual_machine();
    bool            changed                = false;
    ImDrawList*     draw_list              = ImGui::GetWindowDrawList();
    const bool      enable_edition         = virtual_machine->is_program_stopped();
    std::vector<Node*> node_registry       = m_graph->get_node_registry();
    const ImVec2    mouse_pos              = ImGui::GetMousePos();
    SlotView*       hovered_wire_start     = nullptr;
    SlotView*       hovered_wire_end       = nullptr;
    NodeView*       hovered_node           = nullptr;
    Vec2            mouse_start_drag_pos   = mouse_pos - ImGui::GetMouseDragDelta(0);

    auto make_wire_id = [](const Slot* ptr1, const Slot* ptr2) -> ImGuiID
    {
        string128 id;
        id.append_fmt("wire %zu->%zu", ptr1, ptr2);
        return ImGui::GetID(id.c_str());
    };

    auto draw_wire_from_slot_to_pos = [&](SlotView* from, const Vec2& end_pos) -> void
    {
        EXPECT(from != nullptr, "from slot can't be nullptr")

        // Style

        ImGuiEx::WireStyle style;
        style.shadow_color = cfg->ui_codeflow_shadowColor,
        style.roundness    = 0.f;

        if ( from->slot().type() == SlotFlag_TYPE_CODEFLOW )
        {
            style.color     = cfg->ui_codeflow_color,
            style.thickness = cfg->ui_slot_size.x * cfg->ui_codeflow_thickness_ratio;
        }
        else
        {
            style.color     = cfg->ui_node_borderHighlightedColor;
            style.thickness = cfg->ui_wire_bezier_thickness;
        }

        // Draw

        ImGuiID id     = make_wire_id(&m_active_slotview->slot(), nullptr);
        Vec2 start_pos = from->get_pos(SCREEN_SPACE);

        BezierCurveSegment segment{
            start_pos, start_pos,
            end_pos, end_pos
        }; // straight line

        ImGuiEx::DrawWire(id, draw_list, segment, style);
    };

    auto draw_grid = []( ImDrawList* draw_list ) -> void
    {
        Config* cfg = get_config();
        Rect area = ImGuiEx::GetContentRegion(SCREEN_SPACE);
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
    };

    // Background Grid
    draw_grid( draw_list );

    // Draw Wires (code flow ONLY)
    const ImGuiEx::WireStyle code_flow_style{
        cfg->ui_codeflow_color,
        cfg->ui_codeflow_color, // hover
        cfg->ui_codeflow_shadowColor,
        cfg->ui_codeflow_thickness(),
        0.0f
    };
    for( Node* each_node : node_registry ) // TODO: Consider iterating over all the DirectedEdges instead
    {
        NodeView *each_view = NodeView::substitute_with_parent_if_not_visible( each_node->get_component<NodeView>() );

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
                NodeView* each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->get_component<NodeView>() );

                if ( !each_successor_view )
                    continue;
                if ( !each_view->visible )
                    continue;
                if ( !each_successor_view->visible )
                    continue;

                SlotView* start = slot->get_view();
                SlotView* end   = adjacent_slot->get_view();

                ImGuiID id = make_wire_id( slot, adjacent_slot);
                Vec2 start_pos = start->get_pos(SCREEN_SPACE);
                Vec2 end_pos = end->get_pos(SCREEN_SPACE);
                BezierCurveSegment segment{
                    start_pos,
                    start_pos,
                    end_pos,
                    end_pos,
                };
                // TODO: this block is repeated twice
                ImGuiEx::DrawWire(id, draw_list, segment, code_flow_style );
                if ( ImGui::GetHoveredID() == id)
                {
                    hovered_wire_start  = start;
                    hovered_wire_end    = end;
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
    for (auto each_node: node_registry )
    {
        for (const Slot* slot: each_node->filter_slots( SlotFlag_OUTPUT ))
        {
            ImGuiEx::WireStyle style = default_wire_style;
            Slot* adjacent_slot = slot->first_adjacent();

            if( adjacent_slot == nullptr )
                continue;

            NodeView* node_view          = slot->get_node()->get_component<NodeView>();
            NodeView* adjacent_nodeview = adjacent_slot->get_node()->get_component<NodeView>();

            if ( !node_view->visible )
                continue;
            if ( !adjacent_nodeview->visible )
                continue;

            SlotView* slotview          = slot->get_view();
            SlotView* adjacent_slotview = adjacent_slot->get_view();

            float linear_dist = Vec2::distance(
                slotview->get_pos(SCREEN_SPACE),
                adjacent_slotview->get_pos(SCREEN_SPACE)
            );
            float linear_dist_half  = linear_dist * 0.5f;

            BezierCurveSegment curve{
                slotview->get_pos(SCREEN_SPACE),
                slotview->get_pos(SCREEN_SPACE) + slotview->normal() * linear_dist_half,
                adjacent_slotview->get_pos(SCREEN_SPACE) + adjacent_slotview->normal() * linear_dist_half,
                adjacent_slotview->get_pos(SCREEN_SPACE)
            };

            // do not draw long lines between a variable value
            if (is_selected(node_view ) ||
                is_selected(adjacent_nodeview ) )
            {
                style.color.w *= wave(0.5f, 1.f, (float)BaseApp::elapsed_time(), 10.f);
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

                // TODO: this block is repeated twice
                ImGuiID id = make_wire_id( slot, adjacent_slot );
                ImGuiEx::DrawWire(id, draw_list, curve, style);
                if ( ImGui::GetHoveredID() == id )
                {
                    hovered_wire_start  = slotview;
                    hovered_wire_end    = adjacent_slotview;
                }
            }
        }
    }

    // Draw NodeViews
    for ( NodeView* nodeview : get_all_nodeviews() )
    {
        if ( !nodeview->visible )
            continue;

        changed |= nodeview->draw();

        // VM Cursor (scroll to the next node when VM is debugging)
        if( virtual_machine->is_debugging() )
            if( virtual_machine->is_next_node(nodeview->get_owner() ) )
                ImGui::SetScrollHereY();

        if ( nodeview->hovered )
            hovered_node = nodeview; // last hovered win
    }

	// Virtual Machine cursor
    if ( virtual_machine->is_program_running() )
    {
        Node* node = virtual_machine->get_next_node();
        if( NodeView* view = node->get_component<NodeView>() )
        {
            Vec2 left = view->get_rect(SCREEN_SPACE).left();
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

    switch (m_tool)
    {
        case Tool_NONE:
        {
            if ( hovered_node != nullptr )
            {
                if ( ImGui::IsMouseDown(0) && ImGui::IsMouseDragPastThreshold(0))
                {
                    set_tool(Tool_DRAG_NODEVIEWS);
                }
                else if ( hovered_node->m_last_hovered_slotview && ImGui::IsMouseDown(0))
                {
                    m_active_slotview = hovered_node->m_last_hovered_slotview;
                    set_tool(Tool_CREATE_WIRE);
                }
                else if( !hovered_node->selected && ImGui::IsMouseClicked(0) )
                {
                    // Add/Replace selection
                    bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                           ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                    SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                          : SelectionMode_REPLACE;
                    set_selected({hovered_node}, flags);
                    m_active_nodeview = hovered_node;
                }
                else if( ImGui::IsMouseDoubleClicked(0) )
                {
                    hovered_node->expand_toggle();
                }
            }
            else if ( ImGui::IsMouseClicked(0) )
            {
                set_selected({}, SelectionMode_REPLACE);
            }
            else if( ImGui::IsMouseDragging(0) )
            {
                if(  ImGui::IsKeyDown(ImGuiKey_Space) )
                {
                    set_tool(Tool_DRAG_ALL);
                }
                else
                {
                    set_tool(Tool_DEFINE_ROI);
                }
            }
            break;
        }

        case Tool_DRAG_NODEVIEWS:
        {
            if( !ImGui::IsMouseDragging(0) )
            {
                set_tool(Tool_NONE);
                break;
            }
            Vec2 delta = ImGui::GetMouseDragDelta();
            if( delta.lensqr() < 1 )
                break;
            NodeViewFlags flags = NodeViewFlag_IGNORE_SELECTED // we don't want to indirectly translate node that will also translated from here
                                | NodeViewFlag_IGNORE_PINNED;
            for(auto& node_view : m_selected_nodeview)
                node_view->translate(delta, flags);
            ImGui::ResetMouseDragDelta();
            break;
        }

        case Tool_DRAG_ALL:
        {
            if( !ImGui::IsMouseDragging(0))
            {
                set_tool(Tool_NONE);
                break;
            }
            Vec2 delta = ImGui::GetMouseDragDelta();
            if( delta.lensqr() < 1 )
                break;
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            translate_all(delta);
            ImGui::ResetMouseDragDelta();
            break;
        }

        case Tool_DEFINE_ROI:
        {
            // Define the ROI rectangle
            Rect roi_rect;
            roi_rect.min.x = fmin(mouse_start_drag_pos.x, mouse_pos.x);
            roi_rect.min.y = fmin(mouse_start_drag_pos.y, mouse_pos.y);
            roi_rect.max.x = fmax(mouse_start_drag_pos.x, mouse_pos.x);
            roi_rect.max.y = fmax(mouse_start_drag_pos.y, mouse_pos.y);

            if ( ImGui::IsMouseReleased(0) )
            {
                // Select the views included in the ROI
                NodeViewVec nodeview_in_roi;
                for(NodeView* nodeview : get_all_nodeviews())
                {
                    Rect r = nodeview->get_rect(SCREEN_SPACE);
                    if( Rect::contains(roi_rect, r))
                        nodeview_in_roi.emplace_back(nodeview);
                }

                bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                       ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                      : SelectionMode_REPLACE;
                set_selected(nodeview_in_roi, flags);

                // Switch back to default tool
                set_tool(Tool_NONE);
            }
            else
            {
                // Draw the ROI rectangle
                float alpha = wave(0.5f, 0.75f, (float)BaseApp::elapsed_time(), 10.f);
                draw_list->AddRect(roi_rect.min, roi_rect.max, ImColor(0.f, 1.f, 0.f, alpha) );
            }
            break;
        }

        case Tool_CREATE_WIRE:
        {
            if(!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) break;
            if(!ImGui::IsMouseReleased( 0 )) break;
            if( hovered_node == nullptr ) break;
            if( hovered_node->m_last_hovered_slotview == nullptr ) break;

            auto& event_manager = EventManager::get_instance();
            auto event = new Event_SlotDropped();
            event->data.first  = &m_active_slotview->slot();
            event->data.second = &hovered_node->m_last_hovered_slotview->slot();
            event_manager.dispatch(event);

            break;
        }
    }

    // Context menu (open)
    if ( ImGui::IsMouseClicked(1) && !ImGui::IsPopupOpen( k_CONTEXT_MENU_POPUP ) )
    {
        if ( ImGui::IsWindowHovered() )
        {
            ImGui::OpenPopup( k_CONTEXT_MENU_POPUP );
        }
        m_create_node_menu.reset_state();
    } // Context menu (open)

    // Context menu (draw)
	if ( ImGui::BeginPopup( k_CONTEXT_MENU_POPUP ) )
    {
        // Draw temporary Wire
        if( m_active_slotview != nullptr)
        {
            draw_wire_from_slot_to_pos(m_active_slotview, ImGui::GetMousePosOnOpeningCurrentPopup() );
        }

        // SlotView focused menu
        if (m_active_slotview != nullptr )
        {
            // Delete focused Wire?
            if ( ImGui::Button("Delete Edge") )
            {
                LOG_MESSAGE("GraphView", "Delete Edge Button clicked!\n");
                // Generate an event from this action, add some info to the state and dispatch it.
                auto& event_manager = EventManager::get_instance();
                auto* event = new Event_DeleteEdge();
                event->data.first = &hovered_wire_start->slot();
                event->data.second = &hovered_wire_end->slot();
                event_manager.dispatch(event);

                ImGui::CloseCurrentPopup();
            }

            // Disconnect focused SlotView?
            bool can_disconnect = m_active_slotview->has_node_connected();
            if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect", nullptr, can_disconnect)  )
            {
                auto &event_manager = EventManager::get_instance();
                event_manager.dispatch<Event_SlotDisconnected>({&m_active_slotview->slot()});
                ImGui::CloseCurrentPopup();
            }

            // 2) Actions relative to currently focused node

            if ( ImGui::MenuItem( "Arrange" ) )
            {
                m_active_nodeview->arrange_recursively();
            }

            ImGui::MenuItem("Pinned", "", &m_active_nodeview->m_pinned, true );

            if ( ImGui::MenuItem("Expanded", "", &m_active_nodeview->m_expanded, true ) )
            {
                m_active_nodeview->set_expanded(m_active_nodeview->m_expanded );
            }

            ImGui::Separator();

            auto flags = enable_edition ? ImGuiSelectableFlags_None
                                        : ImGuiSelectableFlags_Disabled;
            if ( ImGui::Selectable( "Delete", flags ) )
            {
                m_active_nodeview->get_owner()->flagged_to_delete = true;
            }
        }
        else
        {
            // 3) Actions relative to node creation

            // Title :
            ImGuiEx::ColoredShadowedText( Vec2( 1, 1 ), Color( 0, 0, 0, 255 ), Color(255, 255, 255, 127 ), "Create new node :" );
            ImGui::Separator();
            /*
            *  In case user has created a new node we need to connect it to the graph depending
            *  on if a slot is being dragged and  what is its nature.
            */
            if ( Action_CreateNode* triggered_action = m_create_node_menu.draw_search_input(m_active_slotview, 10 ) )
            {
                // Generate an event from this action, add some info to the state and dispatch it.
                auto& event_manager = EventManager::get_instance();
                auto* event = triggered_action->make_event();
                event->data.graph              = m_graph;
                event->data.active_slotview    = m_active_slotview;
                event->data.desired_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
                event_manager.dispatch(event);

                ImGui::CloseCurrentPopup();
            }
        }

		ImGui::EndPopup();
	}
    else // popup is closed
    {
        // If user was dragging a slot, we draw a temporary wire
        if( m_active_slotview != nullptr)
        {
            draw_wire_from_slot_to_pos(m_active_slotview, mouse_pos );
        }
    } // Context menu (draw)

    // add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

bool GraphView::update(float delta_time, u16_t subsample_count)
{
    const float subsample_delta_time = delta_time / float(subsample_count);
    for(u16_t i = 0; i < subsample_count; i++)
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

void GraphView::frame_views(const std::vector<NodeView*>& _views, bool _align_top_left_corner)
{
    if (_views.empty())
    {
        LOG_VERBOSE("GraphView", "Unable to frame views vector. Reason: is empty.\n")
        return;
    }

    Rect frame = get_content_region(SCREEN_SPACE);

    // Get views' bbox
    Rect views_bbox = NodeView::get_rect(_views, SCREEN_SPACE);

    // debug
    ImGuiEx::DebugRect(views_bbox.min, views_bbox.max, IM_COL32(0, 255, 0, 127 ), 5.0f );
    ImGuiEx::DebugRect( frame.min, frame.max, IM_COL32( 255, 255, 0, 127 ), 5.0f );

    // align
    Vec2 move;
    if (_align_top_left_corner)
    {
        // Align with the top-left corner
        views_bbox.expand(Vec2(20.0f ) ); // add a padding to avoid alignment too close from the border
        move = frame.tl() - views_bbox.tl();
    }
    else
    {
        // Align the center of the node rectangle with the frame center
        move = frame.center() - views_bbox.center();
    }

    // apply the translation
    // TODO: Instead of applying a translation to all views, we could translate a Camera.
    auto node_views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(node_views, move, NodeViewFlag_NONE);

    // debug
    ImGuiEx::DebugLine(views_bbox.center(), views_bbox.center() + move, IM_COL32(255, 0, 0, 255 ), 20.0f);
}

void GraphView::translate_all(const std::vector<NodeView*>& _views, const Vec2& delta, NodeViewFlags flags )
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

void GraphView::translate_all(const Vec2& delta)
{
    translate_all(get_all_nodeviews(), delta, NodeViewFlag_NONE);
}

void GraphView::add_action_to_context_menu( Action_CreateNode* _action )
{
    m_create_node_menu.add_action(_action);
}

void GraphView::frame_nodes(FrameMode mode )
{
    switch (mode)
    {
        case FRAME_ALL:
        {
            if(m_graph->is_empty())
                return;

            // frame the root's view (top-left corner)
            auto& root_view = *m_graph->get_root()->get_component<NodeView>();
            frame_views( {&root_view}, true);
            break;
        }

        case FRAME_SELECTION_ONLY:
        {
            if ( !m_selected_nodeview.empty())
                frame_views(m_selected_nodeview, false);
            break;
        }
        default:
            EXPECT(false, "unhandled case!")
    }
}

Action_CreateNode* CreateNodeContextMenu::draw_search_input(SlotView* dragged_slot, size_t _result_max_count )
{
    if ( must_be_reset_flag )
    {
        ImGui::SetKeyboardFocusHere();

        //
        update_cache_based_on_signature(dragged_slot);

        // Initial search
        update_cache_based_on_user_input(dragged_slot, 100 );

        // Ensure we reset once
        must_be_reset_flag = false;
    }

    // Draw search input and update_cache_based_on_user_input on input change
    if ( ImGui::InputText("Search", search_input, 255, ImGuiInputTextFlags_EscapeClearsAll ))
    {
        update_cache_based_on_user_input(dragged_slot, 100 );
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
void CreateNodeContextMenu::update_cache_based_on_signature(SlotView* dragged_slot)
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
                else if ( dragged_slot->allows(SlotFlag_INPUT) && dragged_slot->get_property_type()->is<Node*>() )
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

void CreateNodeContextMenu::update_cache_based_on_user_input(SlotView* _dragged_slot, size_t _limit )
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

void CreateNodeContextMenu::reset_state()
{
    must_be_reset_flag   = true;
    search_input[0]      = '\0';
    items_matching_search.clear();
    items_with_compatible_signature.clear();
}

void CreateNodeContextMenu::add_action(Action_CreateNode* action)
{
    items.push_back(action);
}

void GraphView::set_selected(const NodeViewVec& views, SelectionMode mode )
{
    NodeViewVec curr_selection = m_selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        m_selected_nodeview.clear();
        for(auto& each : curr_selection )
            each->selected = false;
    }

    for(auto& each : views)
    {
        m_selected_nodeview.emplace_back(each);
        each->selected = true;
    }

    EventPayload_NodeViewSelectionChange event{ m_selected_nodeview, curr_selection };
    EventManager& event_manager = EventManager::get_instance();
    event_manager.dispatch<Event_SelectionChange>(event);
}

const GraphView::NodeViewVec& GraphView::get_selected() const
{
    return m_selected_nodeview;
}

bool GraphView::is_selected(NodeView* view) const
{
    return std::find( m_selected_nodeview.begin(), m_selected_nodeview.end(), view) != m_selected_nodeview.end();
}

void GraphView::start_drag_nodeviews()
{
    for(auto& each : m_selected_nodeview)
        each->m_pinned = true;
}

bool GraphView::selection_empty() const
{
    return m_selected_nodeview.empty();
}

void GraphView::reset()
{
    if ( m_graph->is_empty() )
        return;

    // unfold the graph (does not work great when nodes are rendered for the first time)
    unfold();

    // make sure views are outside viewable rectangle (to avoid flickering)
    auto views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(views, Vec2(-1000.f, -1000.0f), NodeViewFlag_NONE);

    // frame all (33ms delayed to ensure layout is correct)
    auto& event_manager = EventManager::get_instance();
    event_manager.dispatch_delayed<Event_FrameSelection>( 33, { FRAME_ALL } );
}

std::vector<NodeView*> GraphView::get_all_nodeviews() const
{
     return NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
}

void GraphView::set_tool(Tool tool)
{
    ASSERT(tool != m_tool)

    switch (tool)
    {
        case Tool_DRAG_NODEVIEWS:
        {
            start_drag_nodeviews();
        }
        case Tool_NONE:
        case Tool_DEFINE_ROI:
        case Tool_DRAG_ALL:
        case Tool_CREATE_WIRE:
            break;
    }
    m_tool = tool;
}

bool GraphView::has_no_tool_active() const
{
    return m_tool != Tool_NONE;
}

void GraphView::reset_all_properties()
{
    for( NodeView* each : get_all_nodeviews() )
        for( PropertyView* property_view : each->m_property_views )
            property_view->reset();
}
