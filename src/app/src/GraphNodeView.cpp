#include <nodable/GraphNodeView.h>

#include <algorithm>
#include <utility>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <nodable/Settings.h>
#include <nodable/Log.h>
#include <nodable/Wire.h>
#include <nodable/App.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/NodeView.h>
#include <nodable/MemberConnector.h>
#include <nodable/NodeConnector.h>
#include <nodable/Scope.h>

using namespace Nodable;
using namespace Nodable::Asm;
using namespace Nodable::Reflect;

bool GraphNodeView::draw()
{
    bool edited = false;
    Settings* settings = m_context->settings;
    GraphNode* graph = get_graph_node();
    auto nodeRegistry = graph->get_node_registry();

	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(vec2(0,0));

    /*
       Draw Code Flow
     */
    for( Node* each_node : nodeRegistry)
    {
        int slot_index = 0;
        int slot_count = each_node->successor_slots().get_limit();
        float padding = 2.0f;
        float linePadding = 5.0f;
        for (Node* each_successor_node : each_node->successor_slots() )
        {
            NodeView *each_view           = each_node->get<NodeView>();
            NodeView *each_successor_view = each_successor_node->get<NodeView>();

            if (each_view && each_successor_view && each_view->isVisible() && each_successor_view->isVisible() )
            {
                float viewWidthMin = std::min(each_successor_view->getRect().GetSize().x, each_view->getRect().GetSize().x);
                float lineWidth = std::min(settings->ui_node_connector_width,
                                           viewWidthMin / float(slot_count) - (padding * 2.0f));

                vec2 start = each_view->getScreenPos();
                start.x -= std::max(each_view->getSize().x * 0.5f, lineWidth * float(slot_count) * 0.5f);
                start.x += lineWidth * 0.5f + float(slot_index) * lineWidth;
                start.y += each_view->getSize().y * 0.5f; // align bottom
                start.y += settings->ui_node_connector_height * 0.25f;

                vec2 end = each_successor_view->getScreenPos();
                end.x -= each_successor_view->getSize().x * 0.5f;
                end.x += lineWidth * 0.5f;
                end.y -= each_successor_view->getSize().y * 0.5f; // align top
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
        if (auto draggedMemberConnector = MemberConnector::GetDragged())
        {
            auto hoveredMemberConnector = MemberConnector::GetHovered();
            vec2 start = draggedMemberConnector->getPos();
            vec2 end   = hoveredMemberConnector ? hoveredMemberConnector->getPos() : ImGui::GetMousePos();
            ImGui::GetWindowDrawList()->AddLine(start, end,getColor(ColorType_BorderHighlights), settings->ui_wire_bezier_thickness);
        }

        // Draw temporary Node connection
        if (auto draggedNodeConnector = NodeConnector::GetDragged())
        {
            auto hoveredNodeConnector = NodeConnector::GetHovered();
            vec2 start = draggedNodeConnector->getPos();
            vec2 end   = hoveredNodeConnector ? hoveredNodeConnector->getPos() : ImGui::GetMousePos();
            ImColor color(settings->ui_codeFlow_lineColor);
            ImColor shadowColor(settings->ui_codeFlow_lineShadowColor);
            ImGuiEx::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor, settings->ui_node_connector_width, 0.0f);
        }

        // Drops ?
        bool needsANewNode = false;
        MemberConnector::DropBehavior(needsANewNode);
        NodeConnector::DropBehavior(needsANewNode);

        // Need a need node ?
        if (needsANewNode && !ImGui::IsPopupOpen("ContainerViewContextualMenu"))
        {
            ImGui::OpenPopup("ContainerViewContextualMenu");
        }
    }

	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
        /*
            Wires
        */
        for (auto eachNode : nodeRegistry)
        {
            const Members& members = eachNode->props()->get_members();

            for (auto pair : members)
            {
                const Member* dst_member = pair.second;

                if ( const Member* src_member = dst_member->get_input() )
                {
                    auto src_node_view = src_member->get_owner()->get<NodeView>();
                    auto dst_node_view = eachNode->get<NodeView>(); // equival to dst_member->getOwner()->get<NodeView>();

                    if ( src_node_view->isVisible() && dst_node_view->isVisible() )
                    {
                        const MemberView* src_member_view = src_node_view->getMemberView(src_member);
                        const MemberView* dst_member_view = dst_node_view->getMemberView(dst_member);

                        if ( src_member_view && dst_member_view )
                        {
                            vec2 src_pos = src_member_view->m_out->getPos();
                            vec2 dst_pos = dst_member_view->m_in->getPos();

                            // TODO: add multiple wire type settings

                            // straight wide lines for node connections
                            if (src_member->is_type(Type_Pointer) )
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
                            else{
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

        /*
            NodeViews
        */
        std::vector<NodeView*> nodeViews;
        Node::get_components(nodeRegistry, nodeViews);
		for (auto eachNodeView : nodeViews)
		{
            if (eachNodeView->isVisible())
            {
                eachNodeView->draw();

                if(m_context->vm && m_context->vm->is_debugging() && m_context->vm->get_next_node() == eachNodeView->get_owner())
                    ImGui::SetScrollHereY();

                // dragging
                if ( NodeView::GetDragged() == eachNodeView && ImGui::IsMouseDragging(0))
                {
                    eachNodeView->translate(ImGui::GetMouseDragDelta(), true);
                    ImGui::ResetMouseDragDelta();
                    eachNodeView->setPinned(true);
                }

                isAnyNodeDragged |= NodeView::GetDragged() == eachNodeView;
                isAnyNodeHovered |= eachNodeView->isHovered();
            }
		}
	}

	isAnyNodeDragged |= NodeConnector::IsDragging();
	isAnyNodeDragged |= MemberConnector::IsDragging();

	// Virtual Machine cursor
	if( m_context->vm )
    {
	    if ( !m_context->vm->is_program_stopped())
        {
	        auto node = m_context->vm->get_next_node();
	        if( auto view = node->get<NodeView>())
            {
	            vec2 vm_cursor_pos = view->getScreenPos();
	            vm_cursor_pos += view->getMemberView( node->get_this_member() )->relative_pos();
	            vm_cursor_pos.x -= view->getSize().x * 0.5f;

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
		Deselection
	*/
	// Deselection
	if (NodeView::GetSelected() != nullptr && !isAnyNodeHovered && ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
		NodeView::SetSelected(nullptr);

	/*
		Mouse PAN (global)
	*/

	if (ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused() && !isAnyNodeDragged )
    {
        auto drag = ImGui::GetMouseDragDelta();
        for (auto eachNode : nodeRegistry)
        {
            if (auto view = eachNode->get<NodeView>() )
                view->translate(drag);
        }
        ImGui::ResetMouseDragDelta();
    }

	/*
		Mouse right-click popup menu
	*/

	if ( !isAnyNodeHovered && ImGui::BeginPopupContextWindow("ContainerViewContextualMenu") )
	{
		Node* new_node = nullptr;
        bool is_dragging_node_connector = NodeConnector::GetDragged() != nullptr;
        bool is_dragging_member_connector = MemberConnector::GetDragged() != nullptr;
        Member *dragged_member_conn = is_dragging_member_connector ? MemberConnector::GetDragged()->getMember() : nullptr;

		// Title :
		ImGuiEx::ColoredShadowedText( vec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		// lambda to draw operator/function submenus
		auto drawMenu = [&](const std::string _key)-> void {
			char menuLabel[255];
			snprintf( menuLabel, 255, ICON_FA_CALCULATOR" %s", _key.c_str());


			if (ImGui::BeginMenu(menuLabel))
			{		
				auto range = m_contextual_menus.equal_range(_key);
				for (auto it = range.first; it != range.second; it++)
				{
				    auto menu_item = it->second;

				    bool has_compatible_signature;

				    if ( !is_dragging_member_connector )
				    {
				        has_compatible_signature = true;
				    }
				    else
                    {
                        const MemberConnector* dragged_member_conn = MemberConnector::GetDragged();
                        Type dragged_member_type = dragged_member_conn->getMember()->get_type();

                        if ( dragged_member_conn->m_way == Way_Out )
                        {
                            has_compatible_signature = menu_item.function_signature->has_an_arg_of_type(
                                    dragged_member_type);
                        }
                        else
                        {
                            has_compatible_signature =
                                    menu_item.function_signature->get_return_type() == dragged_member_type;
                        }
                    }

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

		if ( !is_dragging_node_connector )
		{
		    drawMenu("Operators");
            drawMenu("Functions");
            ImGui::Separator();
        }



        if ( !is_dragging_node_connector )
        {
            Node *root_node = graph->get_root();
            if ( is_dragging_member_connector )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
                    new_node = graph->create_variable(dragged_member_conn->get_type(), "var",
                                                      root_node->get<Scope>());
            }
            else if ( ImGui::BeginMenu("Variable") )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                    new_node = graph->create_variable(Type_Boolean, "var", root_node->get<Scope>());

                if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                    new_node = graph->create_variable(Type_Double, "var", root_node->get<Scope>());

                if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                    new_node = graph->create_variable(Type_String, "var", root_node->get<Scope>());

                ImGui::EndMenu();
            }
        }

        if ( !is_dragging_node_connector )
        {
            if ( is_dragging_member_connector )
            {
                if (ImGui::MenuItem(ICON_FA_FILE " Literal"))
                    new_node = graph->create_literal(dragged_member_conn->get_type() );
            }
            else if ( ImGui::BeginMenu("Literal") )
            {
                if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                    new_node = graph->create_literal(Type_Boolean);

                if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                    new_node = graph->create_literal(Type_Double);

                if (ImGui::MenuItem(ICON_FA_FILE " String"))
                    new_node = graph->create_literal(Type_String);

                ImGui::EndMenu();
            }
        }

        ImGui::Separator();

        if ( !is_dragging_member_connector || MemberConnector::GetDragged()->m_way == Way_Out)
        {
            if ( ImGui::MenuItem(ICON_FA_CODE " Instruction") )
            {
                new_node = graph->create_instr_user();
            }
        }

        if( !is_dragging_member_connector )
        {
            if (ImGui::MenuItem(ICON_FA_CODE " Condition"))
                new_node = graph->create_cond_struct();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Scope"))
                new_node = graph->create_scope();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Program"))
                new_node = graph->create_root();
        }

        if (new_node)
        {

            // dragging node connector ?
            if (auto dragged = NodeConnector::GetDragged())
            {
                //  [ dragged ]
                //       |
                //       |   (drag direction)
                //       v
                //  [ new node ]
                if (dragged->m_way == Way_Out )
                {
                    graph->connect(new_node, dragged->getNode()->get_parent(), Relation_t::IS_CHILD_OF);
                }
                //  [ new node ]
                //       ^
                //       |   (drag direction)
                //       |
                //  [ dragged ]
                else
                {
                    graph->connect(new_node, dragged->getNode()->get_parent(), Relation_t::IS_CHILD_OF);
                }
                NodeConnector::StopDrag();
            }

            // dragging member connector ?
            if (auto draggedMemberConnector = MemberConnector::GetDragged())
            {
                // [ new node ](out) <---- dragging this way ---- (in)[ dragged connector ]
                if ( draggedMemberConnector->m_way == Way_In )
                {
                    graph->connect(
                            new_node->props()->get_first_member_with_conn(Way_Out),
                            draggedMemberConnector->m_memberView->m_member);
                }
                //  [ dragged connector ](out) ---- dragging this way ----> (in)[ new node ]
                else
                {
                    // connect dragged (out) to first input on new node.
                    graph->connect(
                            draggedMemberConnector->m_memberView->m_member,
                            new_node->props()->get_first_member_with_conn(Way_In));
                }
                MemberConnector::StopDrag();
            }

            // Set New Node's currentPosition were mouse cursor is
			if (auto view = new_node->get<NodeView>())
			{
				auto pos = ImGui::GetMousePos() - origin;
				view->setPosition(pos);
			}
		}

		ImGui::EndPopup();

	}

	// reset dragged if right click
	if ( ImGui::IsMouseClicked(1) )
    {
        ImGui::CloseCurrentPopup();
        MemberConnector::StopDrag();
        NodeConnector::StopDrag();
    }

	// add some empty space
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

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
    LOG_VERBOSE("GraphNodeView", "updateViewConstraints()\n")

    auto nodeRegistry = get_graph_node()->get_node_registry();

    for(Node* _eachNode: nodeRegistry)
    {
        if (auto eachView = _eachNode->get<NodeView>())
        {
            eachView->clearConstraints();
        }
    }

    for(Node* _eachNode: nodeRegistry)
    {
        if ( auto each_node_view = _eachNode->get<NodeView>() )
        {
            auto clss = _eachNode->get_class();

            // Follow predecessor Node(s), except if first predecessor is a Conditional if/else
            //---------------------------------------------------------------------------------

            Nodes& predecessor_nodes = _eachNode->predecessor_slots().content();
            std::vector<NodeView*> predecessor_node_views;
            Node::get_components<NodeView>(predecessor_nodes, predecessor_node_views);
            if (!predecessor_nodes.empty() && predecessor_nodes[0]->get_class()->is_not<IConditionalStruct>() )
            {
                NodeViewConstraint constraint(m_context, NodeViewConstraint::Type::FollowWithChildren);
                constraint.addMasters(predecessor_node_views);
                constraint.addSlave(each_node_view);
                each_node_view->addConstraint(constraint);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            NodeViews children = each_node_view->children_slots().content();
            if( !children.empty() && clss->is<IConditionalStruct>() )
            {
                NodeViewConstraint constraint(m_context,NodeViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                constraint.addMaster(each_node_view);
                constraint.addSlaves(children);

                if ( clss->is<ForLoopNode>() )
                {
                    constraint.addSlaves(each_node_view->successor_slots().content() );
                }

                each_node_view->addConstraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !each_node_view->input_slots().empty() )
            {
                NodeViewConstraint constraint(m_context,NodeViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.addMaster(each_node_view);
                constraint.addSlaves(each_node_view->input_slots().content());
                each_node_view->addConstraint(constraint);
            }
        }
    }
}

bool GraphNodeView::update()
{
    GraphNode* graph                 = get_graph_node();
    std::vector<Node*>& nodeRegistry = graph->get_node_registry();

    // Find NodeView components
    auto deltaTime = ImGui::GetIO().DeltaTime;
    std::vector<NodeView*> views;
    Node::get_components(nodeRegistry, views);

    // updateContraints if needed
    if (graph->is_dirty() )
    {
        update_child_view_constraints();
    }

    // Apply constraints
    for (auto eachView : views)
        if( eachView->isVisible() )
            eachView->applyConstraints(deltaTime);

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
