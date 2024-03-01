#include "GraphView.h"

#include <algorithm>
#include <memory> // std::shared_ptr
#include <utility>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "core/types.h"
#include "fw/core/log.h"
#include "fw/core/system.h"

#include "Action.h"
#include "Condition.h"
#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "PropertyView.h"
#include "SlotView.h"
#include "core/ForLoopNode.h"
#include "core/Graph.h"
#include "core/IfNode.h"
#include "core/LiteralNode.h"
#include "core/NodeUtils.h"
#include "core/Scope.h"
#include "core/Slot.h"
#include "core/VariableNode.h"
#include "core/language/Nodlang.h"
#include "core/math.h"

using namespace ndbl;
using namespace ndbl::assembly;
using namespace fw;

const char* k_context_menu_popup = "GraphView.CreateNodeContextMenu";

REGISTER
{
    fw::registration::push_class<GraphView>("GraphView").extends<fw::View>();
}

GraphView::GraphView(Graph* graph)
    : fw::View()
    , m_graph(graph)
    , m_create_node_context_menu()
{
}

bool GraphView::draw()
{
    bool            changed          = false;
    bool            pixel_perfect    = true;
    ImDrawList*     draw_list        = ImGui::GetWindowDrawList();
    Nodable &       app              = Nodable::get_instance();
    const bool      enable_edition   = app.virtual_machine.is_program_stopped();
    auto            node_registry    = Pool::get_pool()->get( m_graph->get_node_registry() );
    const SlotView* dragged_slot     = SlotView::get_dragged();
    const SlotView* hovered_slot     = SlotView::get_hovered();
    bool drop_behavior_requires_a_new_node = false;
    bool is_any_node_dragged               = false;
    bool is_any_node_hovered               = false;

    // Draw grid in the background
    draw_grid( draw_list, app.config );

    /*
       Draw Code Flow.
       Code flow is the set of green lines that links  a set of nodes.
     */
    float line_width  = app.config.ui_node_slot_size.x * app.config.ui_codeflow_thickness_ratio;
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

                if ( each_successor_view && each_view->is_visible() && each_successor_view->is_visible() )
                {
                    ImRect start = each_view->get_slot_rect( *slot, app.config, slot_index );
                    ImRect end = each_successor_view->get_slot_rect( *adjacent_slot, app.config, 0 );// there is only 1 previous slot

                    fw::ImGuiEx::DrawVerticalWire(
                            ImGui::GetWindowDrawList(),
                            start.GetCenter(),
                            end.GetCenter(),
                            app.config.ui_codeflow_color,      // color
                            app.config.ui_codeflow_shadowColor,// shadowColor,
                            line_width,
                            0.0f );
                }
            }
        }
    }

    // slot Drag'n Drop
    if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
    {
        // Get the current dragged slot, or the slot that was dragged when context menu opened
        const SlotView* _dragged_slot = dragged_slot ? dragged_slot : m_create_node_context_menu.dragged_slot;

        // Draw temporary edge
        if ( _dragged_slot )
        {
            // When dragging, edge follows mouse cursor. Otherwise, it sticks the contextual menu.
            ImVec2 edge_end = m_create_node_context_menu.dragged_slot ? m_create_node_context_menu.opened_at_screen_pos : ImGui::GetMousePos();

            if ( _dragged_slot->slot().type() == SlotFlag_TYPE_CODEFLOW )
            {
                // Thick line
                fw::ImGuiEx::DrawVerticalWire(
                        ImGui::GetWindowDrawList(),
                        _dragged_slot->rect(app.config).GetCenter(),
                        hovered_slot ? hovered_slot->rect(app.config).GetCenter(): edge_end,
                        app.config.ui_codeflow_color,
                        app.config.ui_codeflow_shadowColor,
                        app.config.ui_node_slot_size.x * app.config.ui_codeflow_thickness_ratio,
                        0.f // roundness
                );
            }
            else
            {
                // Simple line
                ImGui::GetWindowDrawList()->AddLine(
                        _dragged_slot->position(),
                    hovered_slot ? hovered_slot->position() : edge_end,
                    ImGui::ColorConvertFloat4ToU32(app.config.ui_node_borderHighlightedColor),
                    app.config.ui_wire_bezier_thickness
                );
            }
        }

        // Determine whether the current dragged SlotView should be dropped or not, and if a new node is required
        SlotView::drop_behavior( drop_behavior_requires_a_new_node, enable_edition);
    }

	{
        /*
            Wires
        */
        for (auto each_node: node_registry )
        {
            for (const Slot* slot: each_node->filter_slots( SlotFlag_OUTPUT ))
            {
                Slot* adjacent_slot = slot->first_adjacent().get();
                if( adjacent_slot == nullptr )
                {
                    continue;
                }
                NodeView* node_view          = slot->node->get_component<NodeView>().get();
                NodeView* adjacent_node_view = adjacent_slot->node->get_component<NodeView>().get();

                if ( !node_view->is_visible() || !adjacent_node_view->is_visible())
                {
                    continue;
                }

                ImVec2 slot_pos           = node_view->get_slot_pos( *slot );
                ImVec2 slot_norm          = node_view->get_slot_normal( *slot );
                ImVec2 adjacent_slot_pos  = adjacent_node_view->get_slot_pos( *adjacent_slot );
                ImVec2 adjacent_slot_norm = adjacent_node_view->get_slot_normal( *adjacent_slot );

                // do not draw long lines between a variable value
                ImVec4 line_color   = app.config.ui_wire_color;
                ImVec4 shadow_color = app.config.ui_wire_shadowColor;

                if ( NodeView::is_selected( node_view->poolid() ) ||
                     NodeView::is_selected( adjacent_node_view->poolid() ) )
                {
                    // blink wire colors
                    float blink = 1.f + std::sin(float(app.elapsed_time()) * 10.f) * 0.25f;
                    line_color.x *= blink;
                    line_color.y *= blink;
                    line_color.z *= blink;
                }
                else
                {
                    // transparent depending on wire length
                    ImVec2 delta = slot_pos - adjacent_slot_pos;
                    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                    if (dist > app.config.ui_wire_bezier_fade_length_minmax.x )
                    {
                        float factor = ( dist - app.config.ui_wire_bezier_fade_length_minmax.x ) /
                                       ( app.config.ui_wire_bezier_fade_length_minmax.y - app.config.ui_wire_bezier_fade_length_minmax.x );
                        line_color = ImLerp(line_color, ImColor(0, 0, 0, 0), factor);
                        shadow_color = ImLerp(shadow_color, ImColor(0, 0, 0, 0), factor);
                    }
                }

                // draw the wire if necessary
                if (line_color.w != 0.f)
                {
                    float thickness = app.config.ui_wire_bezier_thickness;
                    ImVec2 delta = adjacent_slot_pos - slot_pos;
                    float roundness = fw::math::lerp(
                            app.config.ui_wire_bezier_roundness.x, // min
                            app.config.ui_wire_bezier_roundness.y, // max
                              1.0f - fw::math::normalize( ImLengthSqr(delta), 100.0f, 10000.0f )
                            + 1.0f - fw::math::normalize( abs(delta.y), 0.0f, 200.0f)
                            );

                    if ( slot->has_flags(SlotFlag_TYPE_CODEFLOW) )
                    {
                        thickness *= 3.0f;
                        // roundness *= 0.25f;
                    }

                    fw::ImGuiEx::DrawWire(draw_list,
                                          slot_pos, adjacent_slot_pos,
                                          slot_norm, adjacent_slot_norm,
                                          line_color, shadow_color, thickness, roundness);
                }
            }
        }

        /*
            NodeViews
        */
		for ( NodeView* each_node_view : NodeUtils::get_components<NodeView>( m_graph->get_node_registry() ) )
		{
            if (each_node_view->is_visible())
            {
                each_node_view->enable_edition(enable_edition);
                View::use_available_region(each_node_view);
                changed |= each_node_view->draw();

                if( app.virtual_machine.is_debugging() && app.virtual_machine.is_next_node( each_node_view->get_owner() ) )
                {
                    ImGui::SetScrollHereY();
                }

                // dragging
                if (NodeView::get_dragged() == each_node_view->poolid() && ImGui::IsMouseDragging(0))
                {
                    ImVec2 mouse_drag_delta = ImGui::GetMouseDragDelta();
                    each_node_view->translate(mouse_drag_delta, true);
                    ImGui::ResetMouseDragDelta();
                    each_node_view->pinned( true );
                }

                is_any_node_dragged |= NodeView::get_dragged() == each_node_view->poolid();
                is_any_node_hovered |= each_node_view->is_hovered();
            }
		}
	}

    is_any_node_dragged |= SlotView::is_dragging();

	// Virtual Machine cursor
    if ( app.virtual_machine.is_program_running() )
    {
        const Node* node = app.virtual_machine.get_next_node();
        if( NodeView* view = node->get_component<NodeView>().get() )
        {
            ImVec2 vm_cursor_pos = view->get_position(fw::Space_Screen, pixel_perfect);
            vm_cursor_pos.x -= view->get_size().x * 0.5f;

            draw_list->AddCircleFilled( vm_cursor_pos, 5.0f, ImColor(255,0,0) );

            ImVec2 linePos = vm_cursor_pos + ImVec2(- 10.0f, 0.5f);
            linePos += ImVec2(sin(float(app.elapsed_time()) * 12.0f ) * 4.0f, 0.f ); // wave
            float size = 20.0f;
            float width = 2.0f;
            ImColor color = ImColor(255,255,255);

            // arrow ->
            draw_list->AddLine( linePos - ImVec2(1.f, 0.0f), linePos - ImVec2(size, 0.0f), color, width);
            draw_list->AddLine( linePos, linePos - ImVec2(size * 0.5f, -size * 0.5f), color, width);
            draw_list->AddLine( linePos, linePos - ImVec2(size * 0.5f, size * 0.5f) , color, width);
        }
    }

	/*
		Deselection (by double click)
	*/
	if ( NodeView::is_any_selected() && !is_any_node_hovered && ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowFocused())
    {
        NodeView::set_selected({});
    }

	/*
		Mouse PAN (global)
	*/
	if (ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused() && !is_any_node_dragged )
    {
        translate_view(ImGui::GetMouseDragDelta());
        ImGui::ResetMouseDragDelta();
    }

	// Decides whether contextual menu should be opened.
    if ( drop_behavior_requires_a_new_node || (enable_edition && !is_any_node_hovered && ImGui::IsMouseClicked(1) ) )
    {
        if ( !ImGui::IsPopupOpen( k_context_menu_popup ) )
        {
            ImGui::OpenPopup( k_context_menu_popup );
            m_create_node_context_menu.reset_state( SlotView::get_dragged() );
            SlotView::reset_dragged();
        }
    }

    // Defines contextual menu popup (not rendered if popup is closed)
	if ( ImGui::BeginPopup(k_context_menu_popup) )
    {
        // Title :
        fw::ImGuiEx::ColoredShadowedText( ImVec2( 1, 1 ), ImColor( 0.00f, 0.00f, 0.00f, 1.00f ), ImColor( 1.00f, 1.00f, 1.00f, 0.50f ), "Create new node :" );
        ImGui::Separator();
        /*
        *  In case user has created a new node we need to connect it to the m_graph depending
        *  on if a slot is being dragged and  what is its nature.
        */
        if ( Action_CreateNode* action = m_create_node_context_menu.draw_search_input( 10 ) )
        {
            // Generate an event from this action, add some info to the state and dispatch it.
            auto& event_manager = EventManager::get_instance();
            auto* event = action->make_event();
            event->data.graph               = m_graph;
            event->data.dragged_slot        = m_create_node_context_menu.dragged_slot;
            event->data.node_view_local_pos =  m_create_node_context_menu.opened_at_pos;
            event_manager.dispatch(event);

            ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	} else {
        m_create_node_context_menu.reset_state();
    }

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

void GraphView::draw_grid( ImDrawList* draw_list, const Config& config ) const
{
    const int    grid_size             = config.ui_graph_grid_size;
    const int    grid_subdiv_size      = config.ui_graph_grid_size / config.ui_graph_grid_subdivs;
    const int    vertical_line_count   = int( m_screen_space_content_region.GetSize().x) / grid_subdiv_size;
    const int    horizontal_line_count = int( m_screen_space_content_region.GetSize().y) / grid_subdiv_size;
    ImColor      grid_color            = config.ui_graph_grid_color_major;
    ImColor      grid_color_light      = config.ui_graph_grid_color_minor;

    for(int coord = 0; coord <= vertical_line_count; ++coord)
    {
        float pos = m_screen_space_content_region.GetTL().x + float(coord) * float(grid_subdiv_size);
        const ImVec2 line_start{pos, m_screen_space_content_region.GetTL().y};
        const ImVec2 line_end{pos, m_screen_space_content_region.GetBL().y};
        bool is_major = coord % config.ui_graph_grid_subdivs == 0;
        draw_list->AddLine(line_start, line_end, is_major ? grid_color : grid_color_light);
    }

    for(int coord = 0; coord <= horizontal_line_count; ++coord)
    {
        float pos = m_screen_space_content_region.GetTL().y + float(coord) * float(grid_subdiv_size);
        const ImVec2 line_start{ m_screen_space_content_region.GetTL().x, pos};
        const ImVec2 line_end{ m_screen_space_content_region.GetBR().x, pos};
        bool is_major = coord % config.ui_graph_grid_subdivs == 0;
        draw_list->AddLine(line_start, line_end, is_major ? grid_color : grid_color_light);
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
    return update( ImGui::GetIO().DeltaTime, Nodable::get_instance().config.ui_node_animation_subsample_count );
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
    if( auto selected_view = NodeView::get_selected().get())
    {
        frame_views({selected_view}, false);
    }
}

void GraphView::frame_views(const std::vector<NodeView*>& _views, bool _align_top_left_corner)
{
    if (_views.empty())
    {
        LOG_VERBOSE("GraphView", "Unable to frame views vector. Reason: is empty.\n")
        return;
    }
    ImRect screen = m_screen_space_content_region;

    // get selection rectangle
    ImRect nodes_screen_rect = NodeView::get_rect(_views);
    nodes_screen_rect.Translate(screen.Min); // convert to screen space

    // debug
    fw::ImGuiEx::DebugRect(nodes_screen_rect.Min, nodes_screen_rect.Max, IM_COL32(0, 255, 0, 127 ), 5.0f );
    fw::ImGuiEx::DebugRect(screen.Min, screen.Max, IM_COL32( 255, 255, 0, 127 ), 5.0f );

    // align
    ImVec2 translate_vec;
    if (_align_top_left_corner)
    {
        // Align with the top-left corner
        nodes_screen_rect.Expand(20.0f); // add a padding to avoid alignment too close from the border
        translate_vec = screen.GetTL() - nodes_screen_rect.GetTL();
    }
    else
    {
        // Align the center of the node rectangle with the screen center
        translate_vec = screen.GetCenter() - nodes_screen_rect.GetCenter();
    }

    // apply the translation
    // TODO: Instead of applying a translation to all views, we could translate a Camera.
    //       See if we can use matrices in the shaders of ImGui...
    auto all_views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(translate_vec, all_views);

    // debug
    fw::ImGuiEx::DebugLine(nodes_screen_rect.GetCenter(), nodes_screen_rect.GetCenter() + translate_vec, IM_COL32(255, 0, 0, 255 ), 20.0f);
}

void GraphView::translate_all(ImVec2 delta, const std::vector<NodeView*>& _views)
{
    for (auto node_view : _views )
    {
        node_view->translate(delta);
    }
}

void GraphView::unfold()
{
    auto& config = Nodable::get_instance().config;
    update( config.graph_unfold_dt, config.graph_unfold_iterations );
}

void GraphView::translate_view(ImVec2 delta)
{
    auto views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(delta, views);

    // TODO: implement a better solution, storing an offset. And then substract it in draw();
    // m_view_origin += delta;
}

void GraphView::add_action_to_context_menu( Action_CreateNode* _action )
{
    m_create_node_context_menu.items.push_back(_action);
}

void GraphView::frame( FrameMode mode )
{
    // TODO: use an ImRect instead of a FrameMode enum, it will be easier to handle undo/redo
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
                auto& action = *it;
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

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        items_with_compatible_signature = items;
    }
    else
    {
        for (auto& action: items )
        {
            if ( action->event_id == EventID_REQUEST_CREATE_BLOCK )
            {
                if ( dragged_slot->is_this() )
                {
                    items_with_compatible_signature.push_back( action );
                }
            }
            else
            {
                if ( !dragged_slot->is_this() )
                {
                    const type* dragged_property_type = dragged_slot->get_property_type();

                        if ( action->event_initial_state.node_signature )
                        {
                            if ( dragged_slot->allows( SlotFlag_ORDER_FIRST ) )
                            {
                                if ( !action->event_initial_state.node_signature->has_an_arg_of_type(dragged_property_type) )
                                {
                                    continue;
                                }
                            }
                            else if ( !action->event_initial_state.node_signature->get_return_type()->equals(dragged_property_type) )
                            {
                                continue;
                            }
                        } else {
                            // by default, we accept any item not having a signature
                        }
                        items_with_compatible_signature.push_back( action );
                    }
            }
        }
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
    opened_at_pos        = ImGui::GetMousePos() - ImGui::GetCursorScreenPos();
    opened_at_screen_pos = ImGui::GetMousePos();
    dragged_slot         = _dragged_slot;

    items_matching_search.clear();
    items_with_compatible_signature.clear();
}
