#include "GraphView.h"

#include <algorithm>
#include <memory> // std::shared_ptr
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "core/types.h"
#include "fw/core/log.h"
#include "fw/core/system.h"

#include "Config.h"
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

using namespace ndbl;
using namespace ndbl::assembly;
using namespace fw;

REGISTER
{
    fw::registration::push_class<GraphView>("GraphView").extends<fw::View>();
}

GraphView::GraphView(Graph* graph)
    : fw::View()
    , m_graph(graph)
    , m_new_node_desired_position(-1, -1)
{   
    const Nodlang& language = Nodlang::get_instance();
    for (auto& each_fct : language.get_api())
    {
        const fw::func_type* type = each_fct->get_type();
        bool is_operator = language.find_operator_fct(type) != nullptr;

        auto create_node = [this, each_fct, is_operator]() -> PoolID<Node>
        {
            return m_graph->create_function(each_fct.get(), is_operator);
        };

        std::string label;
        language.serialize_func_sig(label, type);
        std::string category = is_operator ? k_operator_menu_label : k_function_menu_label;
        add_contextual_menu_item(category, label, create_node, type);
    }
}

bool GraphView::draw()
{
    bool            changed          = false;
    bool            pixel_perfect    = true;
    ImDrawList*     draw_list        = ImGui::GetWindowDrawList();
    Nodable &       app              = Nodable::get_instance();
    const bool      enable_edition   = app.virtual_machine.is_program_stopped();
    PoolID<Node>    new_node_id;
    ImVec2          origin           = ImGui::GetCursorScreenPos();
    auto            node_registry    = Pool::get_pool()->get( m_graph->get_node_registry() );
    const SlotView* dragged_slot     = SlotView::get_dragged();
    const SlotView* hovered_slot     = SlotView::get_hovered();

    /*
    * Function to draw an invocable menu (operators or functions)
    */
    auto draw_invokable_menu = [&](
        const SlotView* dragged_slot_view,
        const std::string& _key) -> void
    {
        char menuLabel[255];
        snprintf( menuLabel, 255, ICON_FA_CALCULATOR" %s", _key.c_str());

        if (ImGui::BeginMenu(menuLabel))
        {		
            auto range = m_contextual_menus.equal_range(_key);
            for (auto it = range.first; it != range.second; it++)
            {
                FunctionMenuItem menu_item = it->second;

                /*
                * First  we determine  if the current menu_item points to a function with compatible signature.
                */
                bool has_compatible_signature;

                if ( !dragged_slot )
                {
                    has_compatible_signature = true;
                }
                else if ( !dragged_slot->is_this() )
                {
                    const fw::type* dragged_property_type = dragged_slot->get_property_type();

                    if ( dragged_slot->allows( SlotFlag_ORDER_FIRST ) )
                    {
                        has_compatible_signature = menu_item.function_signature->has_an_arg_of_type(dragged_property_type);
                    }
                    else
                    {
                        has_compatible_signature = menu_item.function_signature->get_return_type()->equals(dragged_property_type);
                    }
                }

                /*
                * Then, since we know signature  compatibility, we add or not a new MenuItem.
                */
                if ( has_compatible_signature && ImGui::MenuItem( menu_item.label.c_str() ))
                {
                    if ( menu_item.create_node_fct  )
                    {
                        new_node_id = menu_item.create_node_fct()->poolid();
                    }
                    else
                    {
                        LOG_WARNING("GraphView", "The function associated to the key %s is nullptr",
                                    menu_item.label.c_str())
                    }
                }
            }

            ImGui::EndMenu();
        }	
    };

    auto create_variable = [&](const fw::type* _type, const char*  _name, PoolID<Scope>  _scope) -> PoolID<VariableNode>
    {
        if( !_scope)
        {
           _scope = m_graph->get_root()->get_component<Scope>();
        }

        PoolID<VariableNode> var_node = m_graph->create_variable(_type, _name, _scope );
        var_node->set_declared(true);

        Token token(Token_t::keyword_operator, " = ");
        token.m_word_start_pos = 1;
        token.m_word_size = 1;

        var_node->assignment_operator_token = token;
        return var_node;
    };

    /*
        Grid
        Draw X vertical and Y horizontal lines every grid_size pixels
     */
    const int    grid_size             = app.config.ui_graph_grid_size;
    const int    grid_subdiv_size      = app.config.ui_graph_grid_size / app.config.ui_graph_grid_subdivs;
    const int    vertical_line_count   = int(m_screen_space_content_region.GetSize().x) / grid_subdiv_size;
    const int    horizontal_line_count = int(m_screen_space_content_region.GetSize().y) / grid_subdiv_size;
    ImColor      grid_color            = app.config.ui_graph_grid_color_major;
    ImColor      grid_color_light      = app.config.ui_graph_grid_color_minor;

    for(int coord = 0; coord <= vertical_line_count; ++coord)
    {
        float pos = m_screen_space_content_region.GetTL().x + float(coord) * grid_subdiv_size;
        const ImVec2 line_start{pos, m_screen_space_content_region.GetTL().y};
        const ImVec2 line_end{pos, m_screen_space_content_region.GetBL().y};
        bool is_major = coord % app.config.ui_graph_grid_subdivs == 0;
        draw_list->AddLine(line_start, line_end, is_major ? grid_color : grid_color_light);
    }

    for(int coord = 0; coord <= horizontal_line_count; ++coord)
    {
        float pos = m_screen_space_content_region.GetTL().y + float(coord) * grid_subdiv_size;
        const ImVec2 line_start{m_screen_space_content_region.GetTL().x, pos};
        const ImVec2 line_end{m_screen_space_content_region.GetBR().x, pos};
        bool is_major = coord % app.config.ui_graph_grid_subdivs == 0;
        draw_list->AddLine(line_start, line_end, is_major ? grid_color : grid_color_light);
    }


    /*
       Draw Code Flow.
       Code flow is the set of green lines that links  a set of nodes.
     */
    float line_width  = app.config.ui_node_slot_width * app.config.ui_codeflow_thickness_ratio;
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

            for( auto adjacent_slot : slot->adjacent() )
            {
                Node* each_successor_node = adjacent_slot->get_node();
                NodeView* each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->get_component<NodeView>().get() );

                if ( each_successor_view && each_view->is_visible() && each_successor_view->is_visible() )
                {
                    ImRect start = each_view->get_slot_rect( *slot, app.config, slot_index );
                    ImRect end = each_successor_view->get_slot_rect( *adjacent_slot.get(), app.config, 0 );// there is only 1 previous slot

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
        // Draw temporary edge
        if (dragged_slot)
        {
            if (  dragged_slot->slot().type() == SlotFlag_TYPE_CODEFLOW )
            {
                // Thick line
                fw::ImGuiEx::DrawVerticalWire(
                        ImGui::GetWindowDrawList(),
                        dragged_slot->rect(app.config).GetCenter(),
                        hovered_slot ? hovered_slot->rect(app.config).GetCenter(): ImGui::GetMousePos(),
                        app.config.ui_codeflow_color,
                        app.config.ui_codeflow_shadowColor,
                        app.config.ui_node_slot_width * app.config.ui_codeflow_thickness_ratio,
                        0.f // roundness
                );
            }
            else
            {
                // Simple line
                ImGui::GetWindowDrawList()->AddLine(
                    dragged_slot->position(),
                    hovered_slot ? hovered_slot->position() : ImGui::GetMousePos(),
                    ImGui::ColorConvertFloat4ToU32(app.config.ui_node_borderHighlightedColor),
                    app.config.ui_wire_bezier_thickness
                );
            }
        }

        // Drops ?
        bool require_new_node   = false;
        SlotView::drop_behavior(require_new_node, enable_edition);

        // Need a need node ?
        if (require_new_node)
        {
            if (!ImGui::IsPopupOpen(k_context_menu_popup) )
            {
                ImGui::OpenPopup(k_context_menu_popup);
                m_new_node_desired_position = ImGui::GetMousePos() - origin;
            }
        }
    }

	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
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

                ImVec2 slot_pos          = node_view->get_slot_pos( *slot );
                ImVec2 adjacent_slot_pos = adjacent_node_view->get_slot_pos( *adjacent_slot );

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
                    float roundness = app.config.ui_wire_bezier_roundness;

                    if ( slot->has_flags(SlotFlag_TYPE_CODEFLOW) )
                    {
                        thickness *= 3.0f;
                        roundness *= 0.25f;
                    }

                    fw::ImGuiEx::DrawVerticalWire(draw_list, slot_pos, adjacent_slot_pos, line_color, shadow_color, thickness, roundness);
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

                isAnyNodeDragged |= NodeView::get_dragged() == each_node_view->poolid();
                isAnyNodeHovered |= each_node_view->is_hovered();
            }
		}
	}

	isAnyNodeDragged |= SlotView::is_dragging();

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
	if ( NodeView::is_any_selected() && !isAnyNodeHovered && ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowFocused())
    {
        NodeView::set_selected({});
    }

	/*
		Mouse PAN (global)
	*/
	if (ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused() && !isAnyNodeDragged )
    {
        translate_view(ImGui::GetMouseDragDelta());
        ImGui::ResetMouseDragDelta();
    }

	/*
		Mouse right-click popup menu
	*/

	if ( enable_edition && !isAnyNodeHovered && ImGui::BeginPopupContextWindow(k_context_menu_popup) )
    {
        // Title :
        fw::ImGuiEx::ColoredShadowedText( ImVec2( 1, 1 ), ImColor( 0.00f, 0.00f, 0.00f, 1.00f ), ImColor( 1.00f, 1.00f, 1.00f, 0.50f ), "Create new node :" );
        ImGui::Separator();

        if ( !dragged_slot )
        {
            draw_invokable_menu( dragged_slot, k_operator_menu_label );
            draw_invokable_menu( dragged_slot, k_function_menu_label );
            ImGui::Separator();
        }

        if ( dragged_slot )
        {
            SlotFlags slot_type = dragged_slot->slot().type();
            switch ( slot_type )
            {
                case SlotFlag_TYPE_CODEFLOW:
                {
                    if ( ImGui::MenuItem( ICON_FA_CODE " Condition" ) )
                        new_node_id = m_graph->create_cond_struct();
                    if ( ImGui::MenuItem( ICON_FA_CODE " For Loop" ) )
                        new_node_id = m_graph->create_for_loop();
                    if ( ImGui::MenuItem( ICON_FA_CODE " While Loop" ) )
                        new_node_id = m_graph->create_while_loop();

                    ImGui::Separator();

                    if ( ImGui::MenuItem( ICON_FA_CODE " Scope" ) )
                        new_node_id = m_graph->create_scope();

                    ImGui::Separator();

                    if ( ImGui::MenuItem( ICON_FA_CODE " Program" ) )
                    {
                        m_graph->clear();
                        new_node_id = m_graph->create_root();
                    }
                    break;
                }
                default:
                {
                    if ( !dragged_slot->is_this() )
                    {
                        if ( ImGui::MenuItem( ICON_FA_DATABASE " Variable" ) )
                        {
                            new_node_id = create_variable( dragged_slot->get_property_type(), "var", {} );
                        }

                        // we allow literal only if connected to variables.
                        // why? behavior when connecting a literal to a non var node is to digest it.
                        if ( dragged_slot->get_node()->get_type()->is<VariableNode>() && ImGui::MenuItem( ICON_FA_FILE "Literal" ) )
                        {
                            new_node_id = m_graph->create_literal( dragged_slot->get_property_type() );
                        }
                    }
                    else
                    {
                        if ( ImGui::BeginMenu( "Variable" ) )
                        {
                            if ( ImGui::MenuItem( ICON_FA_DATABASE " Boolean" ) )
                                new_node_id = create_variable( fw::type::get<bool>(), "var", {} );

                            if ( ImGui::MenuItem( ICON_FA_DATABASE " Double" ) )
                                new_node_id = create_variable( fw::type::get<double>(), "var", {} );

                            if ( ImGui::MenuItem( ICON_FA_DATABASE " Int (16bits)" ) )
                                new_node_id = create_variable( fw::type::get<i16_t>(), "var", {} );

                            if ( ImGui::MenuItem( ICON_FA_DATABASE " String" ) )
                                new_node_id = create_variable( fw::type::get<std::string>(), "var", {} );

                            ImGui::EndMenu();
                        }

                        if ( ImGui::BeginMenu( "Literal" ) )
                        {
                            if ( ImGui::MenuItem( ICON_FA_FILE " Boolean" ) )
                                new_node_id = m_graph->create_literal( fw::type::get<bool>() );

                            if ( ImGui::MenuItem( ICON_FA_FILE " Double" ) )
                                new_node_id = m_graph->create_literal( fw::type::get<double>() );

                            if ( ImGui::MenuItem( ICON_FA_FILE " Int (16bits)" ) )
                                new_node_id = m_graph->create_literal( fw::type::get<i16_t>() );

                            if ( ImGui::MenuItem( ICON_FA_FILE " String" ) )
                                new_node_id = m_graph->create_literal( fw::type::get<std::string>() );

                            ImGui::EndMenu();
                        }
                    }
                }
            }
        }

        /*
        *  In case user has created a new node we need to connect it to the m_graph depending
        *  on if a slot is being dragged and  what is its nature.
        */
        if ( new_node_id )
        {

            // dragging node slot ?
            if ( dragged_slot )
            {
                SlotFlags    complementary_flags = flip_order( dragged_slot->slot().static_flags() );
                Slot*        complementary_slot  = new_node_id->find_slot_by_property_type( complementary_flags, dragged_slot->get_property()->get_type() );
                ConnectFlags connect_flags       = ConnectFlag_ALLOW_SIDE_EFFECTS;

                Slot* out = &dragged_slot->slot();
                Slot* in  = complementary_slot;

                if( out->has_flags(SlotFlag_ORDER_SECOND) ) std::swap(out, in);

                m_graph->connect( *out, *in, connect_flags );

                SlotView::reset_dragged();
            }
            else if (new_node_id != m_graph->get_root() && app.config.experimental_graph_autocompletion )
            {
                m_graph->ensure_has_root();
                // m_graph->connect( new_node, m_graph->get_root(), RelType::CHILD  );
            }

            // set new_node's view position
            if( PoolID<NodeView> view = new_node_id->get_component<NodeView>() )
            {
                view->set_position(m_new_node_desired_position, fw::Space_Local);
            }
		}

		ImGui::EndPopup();
	}

	// reset dragged if right click
	if ( ImGui::IsMouseClicked(1) )
    {
        ImGui::CloseCurrentPopup();
        SlotView::reset_dragged();
    }

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

void GraphView::add_contextual_menu_item(
        const std::string &_category,
        const std::string &_label,
        std::function<PoolID<Node>(void)> _function,
        const fw::func_type *_signature)
{
	m_contextual_menus.insert( {_category, {_label, _function, _signature }} );
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
