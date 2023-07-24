#include "GraphView.h"

#include <algorithm>
#include <memory> // std::shared_ptr
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "core/types.h"
#include "fw/core/log.h"
#include "fw/core/system.h"

#include "core/ConditionalStructNode.h"
#include "core/ForLoopNode.h"
#include "core/Graph.h"
#include "core/InstructionNode.h"
#include "core/LiteralNode.h"
#include "core/Scope.h"
#include "core/VariableNode.h"
#include "core/language/Nodlang.h"

#include "Config.h"
#include "Nodable.h"
#include "NodeConnector.h"
#include "NodeView.h"
#include "PropertyView.h"
#include "PropertyConnector.h"
#include "Physics.h"
#include "core/ComponentManager.h"

using namespace ndbl;
using namespace ndbl::assembly;

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

        auto create_node = [this, each_fct, is_operator]() -> Node*
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
    Node*           new_node         = nullptr;
    ImVec2          origin           = ImGui::GetCursorScreenPos();
    const std::vector<Node*>& node_registry    = m_graph->get_node_registry();
    std::vector<NodeView*> node_views          = ComponentManager::collect<NodeView>(node_registry);

	const PropertyConnector* dragged_property_conn = PropertyConnector::get_gragged();
    const PropertyConnector* hovered_property_conn = PropertyConnector::get_hovered();
    const NodeConnector*   dragged_node_conn   = NodeConnector::get_gragged();
    const NodeConnector*   hovered_node_conn   = NodeConnector::get_hovered();

    /*
    * Function to draw an invocable menu (operators or functions)
    */
    auto draw_invokable_menu = [&](
        const PropertyConnector* dragged_property_conn,
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

                if ( !dragged_property_conn )
                {
                    has_compatible_signature = true;
                }
                else
                {
                    const fw::type* dragged_property_type = dragged_property_conn->get_property_type();

                    if ( dragged_property_conn->m_way == Way_Out )
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
                        new_node = menu_item.create_node_fct();
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

    auto create_instr = [&]( Scope* _scope ) -> InstructionNode*
    {
        InstructionNode* instr_node = m_graph->create_instr();
        Token token(Token_t::end_of_instruction, "\n");
        token.m_word_start_pos = 1;
        token.m_word_size = 0; // '\n' is the prefix

        instr_node->token_end = token;
        return instr_node;
    };

    auto create_variable = [&](const fw::type* _type, const char*  _name, Scope*  _scope) -> VariableNode*
    {
        VariableNode* var_node;
        Scope*        scope = _scope;

        if( !scope)
        {
           scope = m_graph->get_root()->components.get<Scope>();
        }

        var_node = m_graph->create_variable(_type, _name, scope );
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
    const float  grid_size = app.config.ui_graph_grid_size;
    const float  grid_subdiv_size = app.config.ui_graph_grid_size / app.config.ui_graph_grid_subdivs;
    const int    vertical_line_count = m_screen_space_content_region.GetSize().x / grid_subdiv_size;
    const int    horizontal_line_count = m_screen_space_content_region.GetSize().y / grid_subdiv_size;
    ImColor      grid_color = app.config.ui_graph_grid_color_major;
    ImColor      grid_color_light = app.config.ui_graph_grid_color_minor;

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
    for( Node* each_node : node_registry)
    {
        size_t slot_index = 0;
        size_t slot_count = each_node->successors.get_limit();
        float padding     = 2.0f;
        float linePadding = 5.0f;
        for (Node* each_successor_node : each_node->successors )
        {
            NodeView *each_view           = NodeView::substitute_with_parent_if_not_visible( each_node->components.get<NodeView>() );
            NodeView *each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->components.get<NodeView>() );

            if (each_view && each_successor_view && each_view->is_visible() && each_successor_view->is_visible() )
            {
                float viewWidthMin = std::min(each_successor_view->get_rect().GetSize().x, each_view->get_rect().GetSize().x);
                float lineWidth = std::min(app.config.ui_node_connector_width,
                                           viewWidthMin / float(slot_count) - (padding * 2.0f));

                ImVec2 start = each_view->get_position(fw::Space_Screen, pixel_perfect);
                start.x -= std::max(each_view->get_size().x * 0.5f, lineWidth * float(slot_count) * 0.5f);
                start.x += lineWidth * 0.5f + float(slot_index) * lineWidth;
                start.y += each_view->get_size().y * 0.5f; // align bottom
                start.y += app.config.ui_node_connector_height * 0.25f;

                ImVec2 end = each_successor_view->get_position(fw::Space_Screen, pixel_perfect);
                end.x -= each_successor_view->get_size().x * 0.5f;
                end.x += lineWidth * 0.5f;
                end.y -= each_successor_view->get_size().y * 0.5f; // align top
                end.y -= app.config.ui_node_connector_height * 0.25f;

                ImColor color(app.config.ui_codeFlow_lineColor);
                ImColor shadowColor(app.config.ui_codeFlow_lineShadowColor);
                fw::ImGuiEx::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor,
                                          lineWidth - linePadding * 2.0f, 0.0f);
            }
            ++slot_index;
        }
    }

    // Connector Drag'n Drop
    if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
    {
        // Draw temporary Property connection
        if ( dragged_property_conn )
        {
            ImVec2 src = dragged_property_conn->get_pos();
            ImVec2 dst = hovered_property_conn ? hovered_property_conn->get_pos() : ImGui::GetMousePos();
            ImGui::GetWindowDrawList()->AddLine(
                    src, dst,
                    get_color(Color_BORDER_HIGHLIGHT),
                    app.config.ui_wire_bezier_thickness
                );
        }

        // Draw temporary Node connection
        if ( dragged_node_conn )
        {
            ImVec2 src = dragged_node_conn->get_pos();
            ImVec2 dst = hovered_node_conn ? hovered_node_conn->get_pos() : ImGui::GetMousePos();
            fw::ImGuiEx::DrawVerticalWire(
                ImGui::GetWindowDrawList(),
                src, dst,
                app.config.ui_codeFlow_lineColor,
                app.config.ui_codeFlow_lineShadowColor,
                app.config.ui_node_connector_width,
                0.f // roundness
                );
        }

        // Drops ?
        bool require_new_node   = false;
        PropertyConnector::drop_behavior(require_new_node, enable_edition);
        NodeConnector::drop_behavior(require_new_node, enable_edition);

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
        for (auto eachNode : node_registry)
        {
            for (auto& dst_property : eachNode->props.by_index())
            {
                if ( const Property * src_property = dst_property->get_input() )
                {
                    Node *src_owner = src_property->get_owner();
                    Node *dst_owner = dst_property->get_owner();
                    NodeView* src_node_view = src_owner->components.get<NodeView>();
                    NodeView* dst_node_view = eachNode->components.get<NodeView>(); // equival to dst_property->getOwner()->get<NodeView>();

                    if (src_node_view->is_visible() && dst_node_view->is_visible() )
                    {
                        const PropertyView* src_property_view = src_node_view->get_property_view(src_property);
                        const PropertyView* dst_property_view = dst_node_view->get_property_view(dst_property);

                        if ( src_property_view && dst_property_view )
                        {
                            ImVec2 src_pos = src_property_view->m_out->get_pos();
                            ImVec2 dst_pos = dst_property_view->m_in->get_pos();

                            // do not draw long lines between a variable value
                            ImVec4 line_color   = app.config.ui_codeFlow_lineColor;
                            ImVec4 shadow_color = app.config.ui_codeFlow_lineShadowColor;


                            if ( NodeView::is_selected(src_property_view->m_nodeView) || NodeView::is_selected(dst_property_view->m_nodeView))
                            {
                                // blink wire colors
                                float blink = 1.f + std::sin(float(app.core.elapsed_time()) * 10.f) * 0.25f;
                                line_color.x *= blink;
                                line_color.y *= blink;
                                line_color.z *= blink;
                            }
                            else
                            {
                                // transparent depending on wire length
                                ImVec2 delta = src_pos - dst_pos;
                                float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                                if (dist > app.config.ui_wire_bezier_length_min)
                                {
                                    float factor = (dist - app.config.ui_wire_bezier_length_min) /
                                                   (app.config.ui_wire_bezier_length_max -
                                                    app.config.ui_wire_bezier_length_min);
                                    line_color = ImLerp(line_color, ImColor(0, 0, 0, 0), factor);
                                    shadow_color = ImLerp(shadow_color, ImColor(0, 0, 0, 0), factor);
                                }
                            }

                            // draw the wire if necessary
                            if ( line_color.w != 0.f )
                            {
                                float thickness = app.config.ui_wire_bezier_thickness;
                                float roundness = app.config.ui_wire_bezier_roundness;

                                // straight wide lines for node connections
                                if (fw::type::is_ptr(src_property->get_type()))
                                {
                                    thickness *= 3.0f;
                                    roundness *= 0.25f;
                                }

                                fw::ImGuiEx::DrawVerticalWire(draw_list, src_pos, dst_pos, line_color, shadow_color, thickness, roundness);
                            }
                        }
                    }
                }
            }
        }

        /*
            NodeViews
        */
		for (NodeView* each_node_view : node_views)
		{
            if (each_node_view->is_visible())
            {
                each_node_view->enable_edition(enable_edition);
                View::use_available_region(each_node_view);
                changed |= each_node_view->draw();

                if( app.virtual_machine.is_debugging() && app.virtual_machine.is_next_node(each_node_view->get_owner() ) )
                {
                    ImGui::SetScrollHereY();
                }

                // dragging
                if (NodeView::get_dragged() == each_node_view && ImGui::IsMouseDragging(0))
                {
                    ImVec2 mouse_drag_delta = ImGui::GetMouseDragDelta();
                    each_node_view->translate(mouse_drag_delta, true);
                    ImGui::ResetMouseDragDelta();
                    each_node_view->pinned = true;
                }

                isAnyNodeDragged |= NodeView::get_dragged() == each_node_view;
                isAnyNodeHovered |= each_node_view->is_hovered();
            }
		}
	}

	isAnyNodeDragged |= NodeConnector::is_dragging();
	isAnyNodeDragged |= PropertyConnector::is_dragging();

	// Virtual Machine cursor
    if ( app.virtual_machine.is_program_running() )
    {
        const Node* node = app.virtual_machine.get_next_node();
        if( NodeView* view = node->components.get<NodeView>())
        {
            ImVec2 vm_cursor_pos = view->get_position(fw::Space_Screen, pixel_perfect);
            vm_cursor_pos.x -= view->get_size().x * 0.5f;

            draw_list->AddCircleFilled( vm_cursor_pos, 5.0f, ImColor(255,0,0) );

            ImVec2 linePos = vm_cursor_pos + ImVec2(- 10.0f, 0.5f);
            linePos += ImVec2(sin(float(app.core.elapsed_time()) * 12.0f ) * 4.0f, 0.f ); // wave
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
	if ( NodeView::get_selected() && !isAnyNodeHovered && ImGui::IsMouseDoubleClicked(0) && ImGui::IsWindowFocused())
    {
        NodeView::set_selected(nullptr);
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
        fw::ImGuiEx::ColoredShadowedText( ImVec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		if ( !dragged_node_conn )
		{
		    draw_invokable_menu(dragged_property_conn, k_operator_menu_label );
            draw_invokable_menu(dragged_property_conn, k_function_menu_label );
            ImGui::Separator();
        }

        if ( !dragged_node_conn )
        {
            // If dragging a property we create a VariableNode with the same type.
            if ( dragged_property_conn && !dragged_property_conn->get_property_type()->is_ptr() )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
                {
                    new_node = create_variable(dragged_property_conn->get_property_type(), "var", nullptr);
                }

                // we allows literal only if connected to variables.
                // why? behavior when connecting a literal to a non var node is to digest it.
                if ( fw::extends<VariableNode>( dragged_property_conn->get_property()->get_owner())
                     && ImGui::MenuItem(ICON_FA_FILE "Literal") )
                {
                    new_node = m_graph->create_literal(dragged_property_conn->get_property_type() );
                }
            }
            // By not knowing anything, we propose all possible types to the user.
            else
            {   
                if ( ImGui::BeginMenu("Variable") )
                {
                    if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                        new_node = create_variable(fw::type::get<bool>(), "var", nullptr);

                    if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                        new_node = create_variable(fw::type::get<double>(), "var", nullptr);
                    
                    if (ImGui::MenuItem(ICON_FA_DATABASE " Int (16bits)"))
                        new_node = create_variable(fw::type::get<i16_t>(), "var", nullptr);

                    if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                        new_node = create_variable(fw::type::get<std::string>(), "var", nullptr);

                    ImGui::EndMenu();
                }

                if ( ImGui::BeginMenu("Literal") )
                {
                    if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                        new_node = m_graph->create_literal(fw::type::get<bool>());

                    if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                        new_node = m_graph->create_literal(fw::type::get<double>());

                    if (ImGui::MenuItem(ICON_FA_FILE " Int (16bits)"))
                        new_node = m_graph->create_literal(fw::type::get<i16_t>());

                    if (ImGui::MenuItem(ICON_FA_FILE " String"))
                        new_node = m_graph->create_literal(fw::type::get<std::string>());

                    ImGui::EndMenu();
                }
            }
        }

        ImGui::Separator();

        if ( !dragged_property_conn )
        {
            if ( ImGui::MenuItem(ICON_FA_CODE " Instruction") )
            {
                new_node = create_instr(nullptr);
            }
        }

        if( !dragged_property_conn )
        {
            if (ImGui::MenuItem(ICON_FA_CODE " Condition"))
                new_node = m_graph->create_cond_struct();
            if (ImGui::MenuItem(ICON_FA_CODE " For Loop"))
                new_node = m_graph->create_for_loop();
            if (ImGui::MenuItem(ICON_FA_CODE " While Loop"))
                new_node = m_graph->create_while_loop();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Scope"))
                new_node = m_graph->create_scope();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Program"))
            {
                m_graph->clear();
                new_node = m_graph->create_root();
            }
                
        }

        /*
        *  In case user has created a new node we need to connect it to the m_graph depending
        *  on if a connector is being dragged and  what is its nature.
        */
        if (new_node)
        {

            // dragging node connector ?
            if ( dragged_node_conn )
            {
                Node* dragged_node = dragged_node_conn->get_node();
                Edge_t edge_type = dragged_node_conn->m_way == Way_Out ? Edge_t::IS_SUCCESSOR_OF : Edge_t::IS_PREDECESSOR_OF;
                m_graph->connect( {new_node, edge_type, dragged_node} );
                NodeConnector::stop_drag();
            }
            else if ( dragged_property_conn )
            {
                if ( dragged_property_conn->m_way == Way_In )
                {
                    Property * dst_property = dragged_property_conn->get_property();
                    Property * src_property = new_node->props.get_first(Way_Out, dst_property->get_type());
                    m_graph->connect( src_property, dst_property );
                }
                //  [ dragged connector ](out) ---- dragging this way ----> (in)[ new node ]
                else
                {
                    // connect dragged (out) to first input on new node.
                    Property * src_property = dragged_property_conn->get_property();
                    Property * dst_property = new_node->props.get_first(Way_In, src_property->get_type());
                    m_graph->connect( src_property, dst_property);
                }
                PropertyConnector::stop_drag();
            }
            else if ( new_node != m_graph->get_root() && app.config.experimental_graph_autocompletion )
            {
                m_graph->ensure_has_root();
                // m_graph->connect( new_node, m_graph->get_root(), RelType::IS_CHILD_OF  );
            }

            // set new_node's view position
            if( NodeView* view = new_node->components.get<NodeView>() )
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
        PropertyConnector::stop_drag();
        NodeConnector::stop_drag();
    }

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

void GraphView::add_contextual_menu_item(
        const std::string &_category,
        const std::string &_label,
        std::function<Node *(void)> _function,
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
    const std::vector<Node*>& nodes = m_graph->get_node_registry();
    std::vector<Physics*>     physics_components  = ComponentManager::collect<Physics>(nodes);
    std::vector<NodeView*>    nodeview_components = ComponentManager::collect<NodeView>(nodes);

    // 1. Update Physics Components
    // 1.1 Apply constraints (but apply no translation, we want to be sure order does no matter)
    for (Physics* physics_component : physics_components)
    {
        physics_component->apply_constraints(delta_time);
    }
    // 1.3 Apply forces (translate views)
    for(Physics* physics_component : physics_components)
    {
        physics_component->apply_forces(delta_time, false);
    }

    // 2. Update NodeViews
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
    Node* root = m_graph->get_root();
    if(!root)
    {
        return;
    }
    std::vector<const NodeView*> views{root->components.get<NodeView>()};
    // frame the root's view (top-left corner)
    frame_views(&views, true);
}

void GraphView::frame_selected_node_views()
{
    std::vector<const NodeView*> views; // we use a vector to send it to a generic function
    if( auto selected = NodeView::get_selected())
    {
        views.push_back(selected);
    }
    // frame selected node (centered)
    frame_views(&views, false);
}

void GraphView::frame_views(const std::vector<const NodeView*>* _views, bool _align_top_left_corner)
{
    if (_views->empty())
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
    std::vector<NodeView*> all_views = ComponentManager::collect<NodeView>(m_graph->get_node_registry());
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
    auto views = ComponentManager::collect<NodeView>(m_graph->get_node_registry());
    translate_all(delta, views);

    // TODO: implement a better solution, storing an offset. And then substract it in draw();
    // m_view_origin += delta;
}
