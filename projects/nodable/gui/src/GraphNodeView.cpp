#include <ndbl/gui/GraphNodeView.h>

#include <algorithm>
#include <memory> // std::shared_ptr
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <fw/core/log.h>
#include <fw/core/system.h>

#include <ndbl/gui/App.h>
#include <ndbl/gui/PropertyConnector.h>
#include <ndbl/gui/NodeConnector.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/gui/Config.h>
#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/language/Nodlang.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/LiteralNode.h>
#include <ndbl/core/ForLoopNode.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/VariableNode.h>

using namespace ndbl;
using namespace ndbl::assembly;

REGISTER
{
    fw::registration::push_class<GraphNodeView>("GraphNodeView")
        .extends<Node>()
        .extends<fw::View>();
}

bool GraphNodeView::on_draw()
{
    bool            changed          = false;
    App&            app              = App::get_instance();
    VirtualMachine& virtual_machine  = VirtualMachine::get_instance();
    const bool      enable_edition   = virtual_machine.is_program_stopped();
    Node*           new_node         = nullptr;
    GraphNode*      graph            = get_graph_node();
    ImVec2          origin           = ImGui::GetCursorScreenPos();
    const std::vector<Node*>& node_registry    = graph->get_node_registry();

	const PropertyConnector* dragged_property_conn = PropertyConnector::get_gragged();
    const PropertyConnector* hovered_property_conn = PropertyConnector::get_hovered();
    const NodeConnector*   dragged_node_conn   = NodeConnector::get_gragged();
    const NodeConnector*   hovered_node_conn   = NodeConnector::get_hovered();
    
    ImGui::SetCursorPos(ImVec2(0,0));

    /*
    * Function to draw an invocable menu (operators or functions)
    */
    auto draw_invocable_menu = [&](
        const PropertyConnector* dragged_property_conn,
        const std::string _key) -> void
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
                    fw::type dragged_property_type = dragged_property_conn->get_property_type();

                    if ( dragged_property_conn->m_way == Way_Out )
                    {
                        has_compatible_signature = menu_item.function_signature->has_an_arg_of_type(dragged_property_type);
                    }
                    else
                    {
                        has_compatible_signature = menu_item.function_signature->get_return_type() == dragged_property_type;
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
                        LOG_WARNING("GraphNodeView", "The function associated to the key %s is nullptr",
                                    menu_item.label.c_str())
                    }
                }
            }

            ImGui::EndMenu();
        }	
    };

    auto create_instr = [&]( Scope* _scope ) -> InstructionNode*
    {
        InstructionNode* instr_node = graph->create_instr();
        std::shared_ptr<Token> token = std::make_shared<Token>(Token_t::end_of_instruction);
        token->append_to_suffix("\n");
        instr_node->end_of_instr_token(token);
        return instr_node;
    };

    auto create_variable = [&](const fw::type& _type, const char*  _name, Scope*  _scope) -> VariableNode*
    {
        VariableNode* var_node;
        Scope* scope = _scope ? _scope : graph->get_root()->get<Scope>();

        var_node = graph->create_variable(_type, _name, scope );
        var_node->set_declared(true);

        std::shared_ptr<Token> token  = std::make_shared<Token>();
        token->m_type = Token_t::keyword_operator;
        token->append_to_prefix(" ");
        token->append_to_word("=");
        token->append_to_suffix(" ");
        
        var_node->set_assignment_operator_token(token);
        return var_node;
    };

    /*
       Draw Code Flow.
       Code flow is the set of green lines that links  a set of nodes.
     */
    for( Node* each_node : node_registry)
    {
        size_t slot_index = 0;
        size_t slot_count = each_node->successors().get_limit();
        float padding     = 2.0f;
        float linePadding = 5.0f;
        for (Node* each_successor_node : each_node->successors() )
        {
            NodeView *each_view           = NodeView::substitute_with_parent_if_not_visible( each_node->get<NodeView>() );
            NodeView *each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->get<NodeView>() );

            if (each_view && each_successor_view && each_view->is_visible() && each_successor_view->is_visible() )
            {
                float viewWidthMin = std::min(each_successor_view->get_rect().GetSize().x, each_view->get_rect().GetSize().x);
                float lineWidth = std::min(app.config.ui_node_connector_width,
                                           viewWidthMin / float(slot_count) - (padding * 2.0f));

                ImVec2 start = each_view->get_screen_position();
                start.x -= std::max(each_view->get_size().x * 0.5f, lineWidth * float(slot_count) * 0.5f);
                start.x += lineWidth * 0.5f + float(slot_index) * lineWidth;
                start.y += each_view->get_size().y * 0.5f; // align bottom
                start.y += app.config.ui_node_connector_height * 0.25f;

                ImVec2 end = each_successor_view->get_screen_position();
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
                    get_color(ColorType_BorderHighlights),
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
            const std::vector<Property*>& properties = eachNode->props()->by_index();

            for (auto& dst_property : properties)
            {
                if ( const Property * src_property = dst_property->get_input() )
                {
                    Node *src_owner = src_property->get_owner();
                    Node *dst_owner = dst_property->get_owner();
                    auto src_node_view = src_owner->get<NodeView>();
                    auto dst_node_view = eachNode->get<NodeView>(); // equival to dst_property->getOwner()->get<NodeView>();

                    if (src_node_view->is_visible() && dst_node_view->is_visible() )
                    {
                        const PropertyView* src_property_view = src_node_view->get_property_view(src_property);
                        const PropertyView* dst_property_view = dst_node_view->get_property_view(dst_property);

                        if ( src_property_view && dst_property_view )
                        {
                            ImVec2 src_pos = src_property_view->m_out->get_pos();
                            ImVec2 dst_pos = dst_property_view->m_in->get_pos();

                            // do not draw long lines between a variable value
                            bool skip_wire = false;
                            if ( !NodeView::is_selected(src_property_view->m_nodeView) && !NodeView::is_selected(dst_property_view->m_nodeView) &&
                                    ((dst_property->get_type().is_ptr() && dst_owner->is<InstructionNode>() && src_owner->is<VariableNode>()) ||
                                     (src_owner->is<VariableNode>() && src_property == src_owner->props()->get(k_value_property_name))))
                            {
                                ImVec2 delta = src_pos - dst_pos;
                                if( abs(delta.x) > 100.0f || abs(delta.y) > 100.0f )
                                {
                                    skip_wire = true;
                                }
                            }

                            if ( !skip_wire )
                            {
                                // straight wide lines for node connections
                                if (fw::type::is_ptr(src_property->get_type()))
                                {
                                    fw::ImGuiEx::DrawVerticalWire(
                                            ImGui::GetWindowDrawList(),
                                            src_pos, dst_pos,
                                            app.config.ui_codeFlow_lineColor,
                                            app.config.ui_codeFlow_lineShadowColor,
                                            app.config.ui_wire_bezier_thickness * 3.0f,
                                            app.config.ui_wire_bezier_roundness * 0.25f);
                                }
                                // curved thin for the others
                                else
                                {
                                    fw::ImGuiEx::DrawVerticalWire(
                                            ImGui::GetWindowDrawList(),
                                            src_pos, dst_pos,
                                            app.config.ui_wire_fillColor,
                                            app.config.ui_wire_shadowColor,
                                            app.config.ui_wire_bezier_thickness,
                                            app.config.ui_wire_bezier_roundness);
                                }
                            }
                        }
                    }
                }
            }
        }

        /*
            NodeViews
        */
        std::vector<NodeView*> node_views;
        Node::get_components(node_registry, node_views);
		for (auto each_node_view : node_views)
		{
            if (each_node_view->is_visible())
            {
                each_node_view->enable_edition(enable_edition);
                changed |= each_node_view->on_draw();

                if( virtual_machine.is_debugging() && virtual_machine.is_next_node(each_node_view->get_owner() ) )
                {
                    ImGui::SetScrollHereY();
                }

                // dragging
                if (NodeView::get_dragged() == each_node_view && ImGui::IsMouseDragging(0))
                {
                    each_node_view->translate(ImGui::GetMouseDragDelta(), true);
                    ImGui::ResetMouseDragDelta();
                    each_node_view->set_pinned(true);
                }

                isAnyNodeDragged |= NodeView::get_dragged() == each_node_view;
                isAnyNodeHovered |= each_node_view->is_hovered();
            }
		}
	}

	isAnyNodeDragged |= NodeConnector::is_dragging();
	isAnyNodeDragged |= PropertyConnector::is_dragging();

	// Virtual Machine cursor
    if ( virtual_machine.is_program_running() )
    {
        const Node* node = virtual_machine.get_next_node();
        if( auto view = node->get<NodeView>())
        {
            ImVec2 vm_cursor_pos = view->get_screen_position();
            vm_cursor_pos += view->get_property_view(node->get_this_property())->relative_pos();
            vm_cursor_pos.x -= view->get_size().x * 0.5f;

            auto draw_list = ImGui::GetWindowDrawList();
            draw_list->AddCircleFilled( vm_cursor_pos, 5.0f, ImColor(255,0,0) );

            ImVec2 linePos = vm_cursor_pos + ImVec2(- 10.0f, 0.5f);
            linePos += ImVec2(sin(float(app.framework.elapsed_time()) * 12.0f ) * 4.0f, 0.f ); // wave
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
        std::vector<NodeView*> views;
        Node::get_components<NodeView>( node_registry, views);
        translate_all( ImGui::GetMouseDragDelta(), views);
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
		    draw_invocable_menu( dragged_property_conn, k_operator_menu_label );
            draw_invocable_menu( dragged_property_conn, k_function_menu_label );
            ImGui::Separator();
        }

        if ( !dragged_node_conn )
        {
            // If dragging a property we create a VariableNode with the same type.
            if ( dragged_property_conn && !dragged_property_conn->get_property_type().is_ptr() )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
                {
                    new_node = create_variable(dragged_property_conn->get_property_type(), "var", nullptr);
                }

                // we allows literal only if connected to variables.
                // why? behavior when connecting a literal to a non var node is to digest it.
                if ( dragged_property_conn->get_property()->get_owner()->is<VariableNode>()
                     && ImGui::MenuItem(ICON_FA_FILE "Literal") )
                {
                    new_node = graph->create_literal(dragged_property_conn->get_property_type() );
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

                    if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                        new_node = create_variable(fw::type::get<std::string>(), "var", nullptr);

                    ImGui::EndMenu();
                }

                if ( ImGui::BeginMenu("Literal") )
                {
                    if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                        new_node = graph->create_literal(fw::type::get<bool>());

                    if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                        new_node = graph->create_literal(fw::type::get<double>());

                    if (ImGui::MenuItem(ICON_FA_FILE " String"))
                        new_node = graph->create_literal(fw::type::get<std::string>());

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
                new_node = graph->create_cond_struct();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Scope"))
                new_node = graph->create_scope();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Program"))
            {
                graph->clear();
                new_node = graph->create_root();
            }
                
        }

        /*
        *  In case user has created a new node we need to connect it to the graph depending
        *  on if a connector is being dragged and  what is its nature.
        */
        if (new_node)
        {

            // dragging node connector ?
            if ( dragged_node_conn )
            {
                Node* dragged_node = dragged_node_conn->get_node();
                Edge_t edge_type = dragged_node_conn->m_way == Way_Out ? Edge_t::IS_SUCCESSOR_OF : Edge_t::IS_PREDECESSOR_OF;
                graph->connect( {new_node, edge_type, dragged_node} );
                NodeConnector::stop_drag();
            }
            else if ( dragged_property_conn )
            {
                if ( dragged_property_conn->m_way == Way_In )
                {
                    Property * dst_property = dragged_property_conn->get_property();
                    Property * src_property = new_node->props()->get_first(Way_Out, dst_property->get_type());
                    graph->connect( src_property, dst_property );
                }
                //  [ dragged connector ](out) ---- dragging this way ----> (in)[ new node ]
                else
                {
                    // connect dragged (out) to first input on new node.
                    Property * src_property = dragged_property_conn->get_property();
                    Property * dst_property = new_node->props()->get_first(Way_In, src_property->get_type());
                    graph->connect( src_property, dst_property);
                }
                PropertyConnector::stop_drag();
            }
            else if ( new_node != graph->get_root() && app.config.experimental_graph_autocompletion )
            {
                graph->ensure_has_root();
                // graph->connect( new_node, graph->get_root(), RelType::IS_CHILD_OF  );
            }

            // set new_node's view position
            if( auto* view = new_node->get<NodeView>() )
            {
                view->set_position(m_new_node_desired_position);
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

void GraphNodeView::add_contextual_menu_item(
        const std::string &_category,
        const std::string &_label,
        std::function<Node *(void)> _function,
        const fw::func_type *_signature)
{
	m_contextual_menus.insert( {_category, {_label, _function, _signature }} );
}

GraphNode* GraphNodeView::get_graph_node() const
{
    return get_owner()->as<GraphNode>();
}

void GraphNodeView::create_child_view_constraints()
{
    auto nodeRegistry = get_graph_node()->get_node_registry();

    for(Node* _eachNode: nodeRegistry)
    {
        if ( auto each_view = _eachNode->get<NodeView>() )
        {
            fw::type t = _eachNode->get_type();

            // Follow predecessor Node(s), except if first predecessor is a Conditional if/else
            //---------------------------------------------------------------------------------

            const std::vector<Node*>& predecessor_nodes = _eachNode->predecessors().content();
            std::vector<NodeView*> predecessor_views;
            Node::get_components<NodeView>(predecessor_nodes, predecessor_views);
            if (!predecessor_nodes.empty() && predecessor_nodes[0]->get_type().is_not_child_of<IConditionalStruct>() )
            {
                ViewConstraint constraint("follow predecessor except if IConditionalStruct", ViewConstraint_t::FollowWithChildren);
                constraint.add_drivers(predecessor_views);
                constraint.add_target(each_view);
                each_view->add_constraint(constraint);

                constraint.apply_when(ViewConstraint::always);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            std::vector<NodeView*>& children = each_view->children().content();
            if(!children.empty() && t.is_child_of<IConditionalStruct>() )
            {
                ViewConstraint constraint("align IConditionalStruct children", ViewConstraint_t::MakeRowAndAlignOnBBoxBottom);
                constraint.apply_when(ViewConstraint::drivers_are_expanded);
                constraint.add_driver(each_view);
                constraint.add_targets(children);

                if (t.is_child_of<ForLoopNode>() )
                {
                    constraint.add_targets(each_view->successors().content());
                }
                each_view->add_constraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !each_view->inputs().empty() )
            {
                ViewConstraint constraint("align inputs", ViewConstraint_t::MakeRowAndAlignOnBBoxTop);
                constraint.add_driver(each_view);
                constraint.add_targets(each_view->inputs().content());
                each_view->add_constraint(constraint);
                constraint.apply_when(ViewConstraint::always);
            }
        }
    }
}

bool GraphNodeView::update(float delta_time, u8_t subsample_count)
{
    const float subsample_delta_time = delta_time / float(subsample_count);
    for(u8_t i = 0; i < subsample_count; i++)
        update( subsample_delta_time );
    return true;
}

bool GraphNodeView::update(float delta_time)
{
    GraphNode* graph                        = get_graph_node();
    const std::vector<Node*>& node_registry = graph->get_node_registry();

    // Find NodeView components
    std::vector<NodeView*> views;
    Node::get_components(node_registry, views);

    // Apply constraints
    for (auto eachView : views)
        if(eachView->is_visible() )
            eachView->apply_constraints(delta_time);

    // Update
    for (auto eachView : views)
        eachView->update();

    return true;
}

bool GraphNodeView::update()
{
    if (get_graph_node()->is_dirty() )
    {
        create_child_view_constraints();
    }
    return update( ImGui::GetIO().DeltaTime, App::get_instance().config.ui_node_animation_subsample_count );
}

void GraphNodeView::set_owner(Node *_owner)
{
    Component::set_owner(_owner);

    // create contextual menu items (not sure this is relevant, but it is better than in File class ^^)
    auto           graph    = fw::cast<GraphNode>(_owner);
    const Nodlang& language = Nodlang::get_instance();

    for (auto& each_fct : language.get_api())
    {
        const fw::func_type* type = &each_fct->get_type();
        bool is_operator = language.find_operator_fct(type) != nullptr;


        auto create_node = [graph, each_fct, is_operator]() -> Node*
        {
            return graph->create_function(each_fct.get(), is_operator);
        };

        std::string label;
        language.serialize(label, type);
        std::string category = is_operator ? k_operator_menu_label : k_function_menu_label;
        add_contextual_menu_item(category, label, create_node, type);
    }
}

void GraphNodeView::frame_all_node_views()
{
    std::vector<NodeView*> views;
    Node::get_components(get_graph_node()->get_node_registry(), views);
    frame_views( views );
}

void GraphNodeView::frame_selected_node_views()
{
    std::vector<NodeView*> views; // we use a vector to send it to a generic function
    if( auto selected = NodeView::get_selected())
    {
        views.push_back(selected);
    }
   frame_views( views );
}

void GraphNodeView::frame_views(std::vector<NodeView*>& _views) {

    if (_views.empty()) {
        LOG_VERBOSE("GraphNodeView", "Unable to frame views vector. Reason: is empty.\n")
        return;
    }

    // get selection rectangle
    ImRect rect = NodeView::get_rect(_views);

    // align graph to center
    ImVec2 delta = m_visible_rect.GetSize() * 0.5f - rect.GetCenter();

    // if there is x overflow, align left
    if (m_visible_rect.GetSize().x <= rect.GetSize().x)
    {
        delta.x = - rect.GetTL().x + 50.0f;
    }

    // if there is y overflow, align top
    if (m_visible_rect.GetSize().y <= rect.GetSize().y)
    {
        // align graph to top-left corner
        delta.y = - rect.GetTL().y + 50.0f;
    }

    std::vector<NodeView*> all_views;
    Node::get_components(get_graph_node()->get_node_registry(), all_views);
    translate_all(delta , all_views);
}

void GraphNodeView::translate_all(ImVec2 delta, const std::vector<NodeView*>& _views)
{
    for (auto node_view : _views )
    {
        node_view->translate(delta);
    }
}

void GraphNodeView::destroy_child_view_constraints()
{
    m_child_view_constraints.clear();

    for(Node* _eachNode: get_graph_node()->get_node_registry())
    {
        if (auto eachView = _eachNode->get<NodeView>())
        {
            eachView->clear_constraints();
        }
    }
}
