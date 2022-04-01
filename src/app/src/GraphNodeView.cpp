#include <nodable/app/GraphNodeView.h>

#include <algorithm>
#include <memory> // std::shared_ptr
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/app/Settings.h>
#include <nodable/core/Log.h>
#include <nodable/core/Wire.h>
#include <nodable/app/App.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/app/NodeView.h>
#include <nodable/app/MemberConnector.h>
#include <nodable/app/NodeConnector.h>
#include <nodable/core/Scope.h>
#include <nodable/core/InstructionNode.h>

using namespace Nodable;
using namespace Nodable::assembly;
using namespace Nodable::R;

bool GraphNodeView::draw()
{
    bool           edited         = false;
    const bool     enable_edition = m_context->vm->is_program_stopped();
    Node*          new_node       = nullptr;
    Settings*      settings       = m_context->settings;
    GraphNode*     graph          = get_graph_node();
    const NodeVec& node_registry  = graph->get_node_registry();
	vec2           origin         = ImGui::GetCursorScreenPos();

	const MemberConnector* dragged_member_conn = MemberConnector::get_gragged();
    const MemberConnector* hovered_member_conn = MemberConnector::get_hovered();
    const NodeConnector*   dragged_node_conn   = NodeConnector::get_gragged();
    const NodeConnector*   hovered_node_conn   = NodeConnector::get_hovered();
    
    ImGui::SetCursorPos(vec2(0,0));

    /*
    * Function to draw an invocable menu (operators or functions)
    */
    auto draw_invocable_menu = [&](
        const MemberConnector* dragged_member_conn,
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

                if ( !dragged_member_conn )
                {
                    has_compatible_signature = true;
                }
                else
                {
                    std::shared_ptr<const R::MetaType> dragged_member_type = dragged_member_conn->get_member_type();

                    if ( dragged_member_conn->m_way == Way_Out )
                    {
                        has_compatible_signature = menu_item.function_signature->has_an_arg_of_type(dragged_member_type);
                    }
                    else
                    {
                        has_compatible_signature = menu_item.function_signature->get_return_type() == dragged_member_type;
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
        std::shared_ptr<Token> token = std::make_shared<Token>(TokenType_EndOfInstruction);
        m_context->language->getSerializer()->serialize(token->m_suffix, TokenType_EndOfLine);
        instr_node->end_of_instr_token(token);
        return instr_node;
    };

    auto create_variable = [&](std::shared_ptr<const R::MetaType> _type, const char*  _name, Scope*  _scope) -> VariableNode*
    {
        VariableNode* var_node;
        Scope* scope = _scope ? _scope : graph->get_root()->get<Scope>();

        var_node = graph->create_variable(_type, _name, scope );

        std::shared_ptr<Token> token  = std::make_shared<Token>();
        token->m_type = TokenType_Operator;
        token->m_prefix  = " ";
        token->m_suffix  = " ";
        token->m_word    = "=";
        
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
        size_t slot_count = each_node->successor_slots().get_limit();
        float padding     = 2.0f;
        float linePadding = 5.0f;
        for (Node* each_successor_node : each_node->successor_slots() )
        {
            NodeView *each_view           = NodeView::substitute_with_parent_if_not_visible( each_node->get<NodeView>() );
            NodeView *each_successor_view = NodeView::substitute_with_parent_if_not_visible( each_successor_node->get<NodeView>() );

            if (each_view && each_successor_view && each_view->is_visible() && each_successor_view->is_visible() )
            {
                float viewWidthMin = std::min(each_successor_view->get_rect().GetSize().x, each_view->get_rect().GetSize().x);
                float lineWidth = std::min(settings->ui_node_connector_width,
                                           viewWidthMin / float(slot_count) - (padding * 2.0f));

                vec2 start = each_view->get_screen_position();
                start.x -= std::max(each_view->get_size().x * 0.5f, lineWidth * float(slot_count) * 0.5f);
                start.x += lineWidth * 0.5f + float(slot_index) * lineWidth;
                start.y += each_view->get_size().y * 0.5f; // align bottom
                start.y += settings->ui_node_connector_height * 0.25f;

                vec2 end = each_successor_view->get_screen_position();
                end.x -= each_successor_view->get_size().x * 0.5f;
                end.x += lineWidth * 0.5f;
                end.y -= each_successor_view->get_size().y * 0.5f; // align top
                end.y -= settings->ui_node_connector_height * 0.25f;

                ImColor color(settings->ui_codeFlow_lineColor);
                ImColor shadowColor(settings->ui_codeFlow_lineShadowColor);
                ImGuiEx::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor,
                                          lineWidth - linePadding * 2.0f, 0.0f);
            }
            ++slot_index;
        }
    }

    // Connector Drag'n Drop
    if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
    {
        // Draw temporary Member connection
        if ( dragged_member_conn )
        {
            vec2 src = dragged_member_conn->get_pos();
            vec2 dst = hovered_member_conn ? hovered_member_conn->get_pos() : ImGui::GetMousePos();
            ImGui::GetWindowDrawList()->AddLine(
                src, dst,
                getColor(Color_BorderHighlights),
                settings->ui_wire_bezier_thickness
                );
        }

        // Draw temporary Node connection
        if ( dragged_node_conn )
        {
            vec2 src = dragged_node_conn->get_pos();
            vec2 dst = hovered_node_conn ? hovered_node_conn->get_pos() : ImGui::GetMousePos();
            ImGuiEx::DrawVerticalWire(
                ImGui::GetWindowDrawList(),
                src, dst,
                settings->ui_codeFlow_lineColor,
                settings->ui_codeFlow_lineShadowColor,
                settings->ui_node_connector_width,
                0.f // roundness
                );
        }

        // Drops ?
        bool require_new_node   = false;
        MemberConnector::drop_behavior(require_new_node, enable_edition);
        NodeConnector::drop_behavior(require_new_node, enable_edition);

        // Need a need node ?
        if (require_new_node)
        {
            if (!ImGui::IsPopupOpen(k_context_menu_popup) )
            {
                ImGui::OpenPopup(k_context_menu_popup);
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
            const MemberMap& members = eachNode->props()->by_name();

            for (auto pair : members)
            {
                const Member* dst_member = pair.second;

                if ( const Member* src_member = dst_member->get_input() )
                {
                    Node *src_owner = src_member->get_owner();
                    Node *dst_owner = dst_member->get_owner();
                    auto src_node_view = src_owner->get<NodeView>();
                    auto dst_node_view = eachNode->get<NodeView>(); // equival to dst_member->getOwner()->get<NodeView>();

                    if (src_node_view->is_visible() && dst_node_view->is_visible() )
                    {
                        const MemberView* src_member_view = src_node_view->get_member_view(src_member);
                        const MemberView* dst_member_view = dst_node_view->get_member_view(dst_member);

                        if ( src_member_view && dst_member_view )
                        {
                            vec2 src_pos = src_member_view->m_out->get_pos();
                            vec2 dst_pos = dst_member_view->m_in->get_pos();

                            // do not draw long lines between a variable value
                            bool skip_wire = false;
                            if ( !NodeView::is_selected(src_member_view->m_nodeView) && !NodeView::is_selected(dst_member_view->m_nodeView) &&
                                    ((dst_member->get_meta_type()->is_ptr() && dst_owner->is<InstructionNode>() && src_owner->is<VariableNode>()) ||
                                (src_owner->is<VariableNode>() && src_member == src_owner->props()->get(k_value_member_name))))
                            {
                                vec2 delta = src_pos - dst_pos;
                                if( abs(delta.x) > 100.0f || abs(delta.y) > 100.0f )
                                {
                                    skip_wire = true;
                                }
                            }

                            // TODO: add multiple wire type settings
                            if ( !skip_wire )
                            {
                                // straight wide lines for node connections
                                if (MetaType::is_ptr(src_member->get_meta_type()))
                                {
                                    ImGuiEx::DrawVerticalWire(
                                            ImGui::GetWindowDrawList(),
                                            src_pos, dst_pos,
                                            settings->ui_codeFlow_lineColor,
                                            settings->ui_codeFlow_lineShadowColor,
                                            settings->ui_wire_bezier_thickness * 3.0f,
                                            settings->ui_wire_bezier_roundness * 0.25f);
                                }
                                // curved thin for the others
                                else
                                {
                                    ImGuiEx::DrawVerticalWire(
                                            ImGui::GetWindowDrawList(),
                                            src_pos, dst_pos,
                                            settings->ui_wire_fillColor,
                                            settings->ui_wire_shadowColor,
                                            settings->ui_wire_bezier_thickness,
                                            settings->ui_wire_bezier_roundness);
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
        std::vector<NodeView*> nodeViews;
        Node::get_components(node_registry, nodeViews);
		for (auto eachNodeView : nodeViews)
		{
            if (eachNodeView->is_visible())
            {
                eachNodeView->enable_edition(enable_edition);
                edited |= eachNodeView->draw();

                if(m_context->vm && m_context->vm->is_debugging() && m_context->vm->get_next_node() == eachNodeView->get_owner())
                    ImGui::SetScrollHereY();

                // dragging
                if (NodeView::get_dragged() == eachNodeView && ImGui::IsMouseDragging(0))
                {
                    eachNodeView->translate(ImGui::GetMouseDragDelta(), true);
                    ImGui::ResetMouseDragDelta();
                    eachNodeView->set_pinned(true);
                }

                isAnyNodeDragged |= NodeView::get_dragged() == eachNodeView;
                isAnyNodeHovered |= eachNodeView->isHovered();
            }
		}
	}

	isAnyNodeDragged |= NodeConnector::is_dragging();
	isAnyNodeDragged |= MemberConnector::is_dragging();

	// Virtual Machine cursor
	if( m_context->vm )
    {
	    if ( !m_context->vm->is_program_stopped())
        {
	        auto node = m_context->vm->get_next_node();
	        if( auto view = node->get<NodeView>())
            {
	            vec2 vm_cursor_pos = view->get_screen_position();
	            vm_cursor_pos += view->get_member_view(node->get_this_member())->relative_pos();
	            vm_cursor_pos.x -= view->get_size().x * 0.5f;

	            auto draw_list = ImGui::GetWindowDrawList();
	            draw_list->AddCircleFilled( vm_cursor_pos, 5.0f, ImColor(255,0,0) );

	            vec2 linePos = vm_cursor_pos + vec2(- 10.0f, 0.5f);
                linePos += vec2( sin(m_context->elapsed_time * 12.0f ) * 4.0f, 0.f ); // wave
	            float size = 20.0f;
	            float width = 2.0f;
	            ImColor color = ImColor(255,255,255);
	            draw_list->AddLine(
	                    linePos- vec2(1.f, 0.0f),
                        linePos - vec2(size, 0.0f),
                        color,
                        width);
                draw_list->AddLine(
                        linePos,
                        linePos - vec2(size * 0.5f, -size * 0.5f),
                        color,
                        width);
                draw_list->AddLine(
                        linePos,
                        linePos - vec2(size * 0.5f, size * 0.5f),
                        color,
                        width);
            }
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
        auto drag = ImGui::GetMouseDragDelta();
        for (auto eachNode : node_registry)
        {
            if (auto view = eachNode->get<NodeView>() )
                view->translate(drag);
        }
        ImGui::ResetMouseDragDelta();
    }

	/*
		Mouse right-click popup menu
	*/

	if ( enable_edition && !isAnyNodeHovered && ImGui::BeginPopupContextWindow(k_context_menu_popup) )
	{
		// Title :
		ImGuiEx::ColoredShadowedText( vec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		if ( !dragged_node_conn )
		{
		    draw_invocable_menu( dragged_member_conn, "Operators");
            draw_invocable_menu( dragged_member_conn, "Functions");
            ImGui::Separator();
        }

        if ( !dragged_node_conn )
        {
            Node *root_node = graph->get_root();

            // If dragging a member we create a VariableNode with the same type.
            if ( dragged_member_conn && !MetaType::is_ptr(dragged_member_conn->get_member_type()) )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
                {
                    new_node = create_variable(dragged_member_conn->get_member_type(), "var", nullptr);
                }

                // we allows literal only if connected to variables.
                // why? behavior when connecting a literal to a non var node is to digest it.
                if ( dragged_member_conn->get_member()->get_owner()->is<VariableNode>()
                     && ImGui::MenuItem(ICON_FA_FILE "Literal") )
                {
                    new_node = graph->create_literal(dragged_member_conn->get_member_type() );
                }
            }
            // By not knowing anything, we propose all possible types to the user.
            else
            {   
                if ( ImGui::BeginMenu("Variable") )
                {
                    if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                        new_node = create_variable(R::get_meta_type<bool>(), "var", nullptr);

                    if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                        new_node = create_variable(R::get_meta_type<double>(), "var", nullptr);

                    if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                        new_node = create_variable(R::get_meta_type<std::string>(), "var", nullptr);

                    ImGui::EndMenu();
                }

                if ( ImGui::BeginMenu("Literal") )
                {
                    if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                        new_node = graph->create_literal(R::get_meta_type<bool>());

                    if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                        new_node = graph->create_literal(R::get_meta_type<double>());

                    if (ImGui::MenuItem(ICON_FA_FILE " String"))
                        new_node = graph->create_literal(R::get_meta_type<std::string>());

                    ImGui::EndMenu();
                }
            }
        }

        ImGui::Separator();

        if ( !dragged_member_conn )
        {
            if ( ImGui::MenuItem(ICON_FA_CODE " Instruction") )
            {
                new_node = create_instr(nullptr);
            }
        }

        if( !dragged_member_conn )
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
            edited = true;

            // dragging node connector ?
            if ( dragged_node_conn )
            {
                Node* dragged_node = dragged_node_conn->get_node();
                EdgeType relation_type = dragged_node_conn->m_way == Way_Out ?
                                         EdgeType::IS_SUCCESSOR_OF : EdgeType::IS_PREDECESSOR_OF;
                graph->connect( {new_node, relation_type, dragged_node} );
                NodeConnector::stop_drag();
            }
            else if ( dragged_member_conn )
            {
                if ( dragged_member_conn->m_way == Way_In )
                {
                    Member* dst_member = dragged_member_conn->get_member();
                    Member* src_member = new_node->props()->get_first_member_with(Way_Out, dst_member->get_meta_type());
                    graph->connect( src_member, dst_member );
                }
                //  [ dragged connector ](out) ---- dragging this way ----> (in)[ new node ]
                else
                {
                    // connect dragged (out) to first input on new node.
                    Member* src_member = dragged_member_conn->get_member();
                    Member* dst_member = new_node->props()->get_first_member_with(Way_In, src_member->get_meta_type());
                    graph->connect( src_member, dst_member);
                }
                MemberConnector::stop_drag();
            }
            else if ( new_node != graph->get_root() && m_context->settings->experimental_graph_autocompletion )
            {
                graph->ensure_has_root();
                // graph->connect( new_node, graph->get_root(), RelType::IS_CHILD_OF  );
            }

            // Set New Node's currentPosition were mouse cursor is
			if (auto view = new_node->get<NodeView>())
			{
				auto pos = ImGui::GetMousePos() - origin;
                view->set_position(pos);
			}
		}

		ImGui::EndPopup();

	}

	// reset dragged if right click
	if ( ImGui::IsMouseClicked(1) )
    {
        ImGui::CloseCurrentPopup();
        MemberConnector::stop_drag();
        NodeConnector::stop_drag();
    }

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    if ( edited )
    {
        graph->set_dirty();
    }

	return edited;
}

void GraphNodeView::add_contextual_menu_item(
        const std::string &_category,
        const std::string &_label,
        std::function<Node *(void)> _function,
        const FunctionSignature *_signature)
{
	m_contextual_menus.insert( {_category, {_label, _function, _signature }} );
}

GraphNode* GraphNodeView::get_graph_node() const
{
    return get_owner()->as<GraphNode>();
}

void GraphNodeView::update_child_view_constraints()
{
    auto nodeRegistry = get_graph_node()->get_node_registry();

    for(Node* _eachNode: nodeRegistry)
    {
        if (auto eachView = _eachNode->get<NodeView>())
        {
            eachView->clear_constraints();
        }
    }

    for(Node* _eachNode: nodeRegistry)
    {
        if ( auto each_node_view = _eachNode->get<NodeView>() )
        {
            auto clss = _eachNode->get_class();

            // Follow predecessor Node(s), except if first predecessor is a Conditional if/else
            //---------------------------------------------------------------------------------

            const NodeVec& predecessor_nodes = _eachNode->predecessor_slots().content();
            std::vector<NodeView*> predecessor_node_views;
            Node::get_components<NodeView>(predecessor_nodes, predecessor_node_views);
            if (!predecessor_nodes.empty() && predecessor_nodes[0]->get_class()->is_not_child_of<IConditionalStruct>() )
            {
                NodeViewConstraint constraint(m_context, NodeViewConstraint::Type::FollowWithChildren);
                constraint.add_drivers(predecessor_node_views);
                constraint.add_target(each_node_view);
                each_node_view->add_constraint(constraint);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            NodeViewVec children = each_node_view->children_slots().content();
            if( !children.empty() && clss->is_child_of<IConditionalStruct>() )
            {
                NodeViewConstraint constraint(m_context,NodeViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                constraint.add_driver(each_node_view);
                constraint.add_targets(children);

                if (clss->is_child_of<ForLoopNode>() )
                {
                    constraint.add_targets(each_node_view->successor_slots().content());
                }

                each_node_view->add_constraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !each_node_view->input_slots().empty() )
            {
                NodeViewConstraint constraint(m_context,NodeViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.add_driver(each_node_view);
                constraint.add_targets(each_node_view->input_slots().content());
                each_node_view->add_constraint(constraint);
            }
        }
    }
}

bool GraphNodeView::update()
{
    GraphNode* graph                        = get_graph_node();
    const std::vector<Node*>& node_registry = graph->get_node_registry();

    // Find NodeView components
    auto deltaTime = ImGui::GetIO().DeltaTime;
    std::vector<NodeView*> views;
    Node::get_components(node_registry, views);

    // updateContraints if needed
    if (graph->is_dirty() )
    {
        update_child_view_constraints();
    }

    // Apply constraints
    for (auto eachView : views)
        if(eachView->is_visible() )
            eachView->apply_constraints(deltaTime);

    // Update
    for (auto eachView : views)
        eachView->update();

    return true;
}

void GraphNodeView::set_owner(Node *_owner)
{
    Component::set_owner(_owner);

    // create contextual menu items (not sure this is relevant, but it is better than in File class ^^)
    auto graphNode = _owner->as<GraphNode>();
    const Language* language = m_context->language;
    const auto api = m_context->language->getAllFunctions();

    for ( auto it = api.cbegin(); it != api.cend(); it++)
    {
        IInvokable* function = *it;
        auto op = language->findOperator(function->get_signature());

        std::string label;
        const FunctionSignature* signature = function->get_signature();
        language->getSerializer()->serialize(label, signature);

        if (op != nullptr )
        {
            auto lambda = [graphNode, op]()->Node*
            {
                return graphNode->create_operator(op);
            };

            add_contextual_menu_item("Operators", label, lambda, signature);
        }
        else
        {
            auto lambda = [graphNode, function]()->Node*
            {
                return graphNode->create_function(function);
            };

            add_contextual_menu_item("Functions", label, lambda, signature);
        }

    }
}
