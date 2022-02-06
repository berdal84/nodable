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
#include <nodable/WireView.h>
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
    App* app = App::Get();
    NODABLE_ASSERT(app != nullptr) // app needs to be defined
    VM* vm = &app->getVM();

    Settings* settings = Settings::Get();
    GraphNode* graph = get_graph_node();
    auto nodeRegistry = graph->getNodeRegistry();

	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImVec2(0,0));

    /*
       Draw Code Flow
     */
    for( auto& each_node : nodeRegistry)
    {
        int slot_index = 0;
        int slot_count = each_node->getNextMaxCount();
        float padding = 2.0f;
        float linePadding = 5.0f;
        for (auto& each_next : each_node->getNext() )
        {
            NodeView *each_view      = each_node->get<NodeView>();
            NodeView *each_next_view = each_next->get<NodeView>();
            if (each_view && each_next_view && each_view->isVisible() && each_next_view->isVisible() )
            {
                float viewWidthMin = std::min(each_next_view->getRect().GetSize().x, each_view->getRect().GetSize().x);
                float lineWidth = std::min(Settings::Get()->ui_node_connector_width,
                                           viewWidthMin / float(slot_count) - (padding * 2.0f));

                ImVec2 start = each_view->getScreenPos();
                start.x -= std::max(each_view->getSize().x * 0.5f, lineWidth * float(slot_count) * 0.5f);
                start.x += lineWidth * 0.5f + float(slot_index) * lineWidth;
                start.y += each_view->getSize().y * 0.5f; // align bottom
                start.y += settings->ui_node_connector_height * 0.25f;

                ImVec2 end = each_next_view->getScreenPos();
                end.x -= each_next_view->getSize().x * 0.5f;
                end.x += lineWidth * 0.5f;
                end.y -= each_next_view->getSize().y * 0.5f; // align top
                end.y -= settings->ui_node_connector_height * 0.25f;

                ImColor color(Settings::Get()->ui_codeFlow_lineColor);
                ImColor shadowColor(Settings::Get()->ui_codeFlow_lineShadowColor);
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
            ImVec2 start = draggedMemberConnector->getPos();
            ImVec2 end   = hoveredMemberConnector ? hoveredMemberConnector->getPos() : ImGui::GetMousePos();
            ImGui::GetWindowDrawList()->AddLine(start, end,getColor(ColorType_BorderHighlights), settings->ui_wire_bezier_thickness);
        }

        // Draw temporary Node connection
        if (auto draggedNodeConnector = NodeConnector::GetDragged())
        {
            auto hoveredNodeConnector = NodeConnector::GetHovered();
            auto settings     = Settings::Get();
            ImVec2 start = draggedNodeConnector->getPos();
            ImVec2 end   = hoveredNodeConnector ? hoveredNodeConnector->getPos() : ImGui::GetMousePos();
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

	/*
		NodeViews
	*/
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
		//  Draw Wires
        for (auto eachNode : nodeRegistry)
        {
            auto members = eachNode->getProps()->getMembers();

            for (auto pair : members)
            {
                auto end = pair.second;

                if ( auto start = end->getInput() )
                {
                    auto endNodeView   = eachNode->get<NodeView>();
                    auto startNodeView = start->getOwner()->get<NodeView>();

                    if ( startNodeView->isVisible() && endNodeView->isVisible() )
                    {
                        auto endView   = endNodeView->getMemberView(end);
                        auto startView = startNodeView->getMemberView(start);

                        if ( endView && startView )
                        {
                            WireView::Draw(ImGui::GetWindowDrawList(), startView->m_out->getPos(), endView->m_in->getPos() );
                        }
                    }
                }
            }
        }

        // Draw NodeViews
        std::vector<NodeView*> nodeViews;
        Node::GetComponents(nodeRegistry, nodeViews);
		for (auto eachNodeView : nodeViews)
		{
            if (eachNodeView->isVisible())
            {
                eachNodeView->draw();

                if(vm && vm->is_debugging() && vm->get_next_node() == eachNodeView->get_owner())
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
	if( vm )
    {
	    if ( !vm->is_program_stopped())
        {
	        auto node = vm->get_next_node();
	        if( auto view = node->get<NodeView>())
            {
	            auto draw_list = ImGui::GetWindowDrawList();
	            draw_list->AddCircleFilled(
	                    view->getScreenPos() - view->getSize() * 0.5f, 5.0f,
	                    ImColor(255,0,0) );

	            ImVec2 linePos = view->getScreenPos() + ImVec2(-view->getSize().x * 0.5f - 10.0f, 0.5f);
	            float size = 20.0f;
	            float width = 2.0f;
	            ImColor color = ImColor(255,255,255);
	            draw_list->AddLine(
	                    linePos,
                        linePos - ImVec2(size, 0.0f),
                        color,
                        width);
                draw_list->AddLine(
                        linePos,
                        linePos - ImVec2(size * 0.5f, -size * 0.5f),
                        color,
                        width);
                draw_list->AddLine(
                        linePos,
                        linePos - ImVec2(size * 0.5f, size * 0.5f),
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
		Node* newNode = nullptr;
        bool is_dragging_node_connector = NodeConnector::GetDragged() != nullptr;
        bool is_dragging_member_connector = MemberConnector::GetDragged() != nullptr;

		// Title :
		ImGuiEx::ColoredShadowedText( ImVec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
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
                        Type dragged_member_type = dragged_member_conn->getMember()->getType();

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
                            newNode = menu_item.create_node_fct();
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
            if ( is_dragging_member_connector )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
                    newNode = graph->newVariable(MemberConnector::GetDragged()->getMember()->getType(), "var", graph->getProgram()->get<Scope>() );
            }
            else if ( ImGui::BeginMenu("Variable") )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                    newNode = graph->newVariable(Type_Boolean, "var", graph->getProgram()->get<Scope>() );

                if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                    newNode = graph->newVariable(Type_Double, "var", graph->getProgram()->get<Scope>() );

                if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                    newNode = graph->newVariable(Type_String, "var", graph->getProgram()->get<Scope>() );

                ImGui::EndMenu();
            }
        }

        if ( !is_dragging_node_connector )
        {
            if ( is_dragging_member_connector )
            {
                if (ImGui::MenuItem(ICON_FA_FILE " Literal"))
                    newNode = graph->newLiteral(MemberConnector::GetDragged()->getMember()->getType());
            }
            else if ( ImGui::BeginMenu("Literal") )
            {
                if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                    newNode = graph->newLiteral(Type_Boolean);

                if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                    newNode = graph->newLiteral(Type_Double);

                if (ImGui::MenuItem(ICON_FA_FILE " String"))
                    newNode = graph->newLiteral(Type_String);

                ImGui::EndMenu();
            }
        }

        ImGui::Separator();

        if ( !is_dragging_member_connector || MemberConnector::GetDragged()->m_way == Way_Out)
        {
            if ( ImGui::MenuItem(ICON_FA_CODE " Instruction") )
            {
                newNode = graph->newInstruction_UserCreated();
            }
        }

        if( !is_dragging_member_connector )
        {
            if (ImGui::MenuItem(ICON_FA_CODE " Condition"))
                newNode = graph->newConditionalStructure();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Scope"))
                newNode = graph->newScope();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Program"))
                newNode = graph->newProgram();
        }

        if (newNode)
        {
            // dragging node connector ?
            if (auto draggedNodeConnector = NodeConnector::GetDragged())
            {
                //  [ dragged ]
                //       |
                //       |   (drag direction)
                //       v
                //  [ new node ]
                if ( draggedNodeConnector->m_way == Way_Out )
                {
                    graph->connect(newNode, draggedNodeConnector->getNode(), Relation_t::IS_NEXT_OF);
                }
                //  [ new node ]
                //       ^
                //       |   (drag direction)
                //       |
                //  [ dragged ]
                else
                {
                    graph->connect(draggedNodeConnector->getNode(), newNode, Relation_t::IS_NEXT_OF);
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
                            newNode->getProps()->getFirstWithConn(Way_Out),
                            draggedMemberConnector->m_memberView->m_member);
                }
                //  [ dragged connector ](out) ---- dragging this way ----> (in)[ new node ]
                else
                {
                    // connect dragged (out) to first input on new node.
                    graph->connect(
                            draggedMemberConnector->m_memberView->m_member,
                            newNode->getProps()->getFirstWithConn(Way_In));
                }
                MemberConnector::StopDrag();
            }

            // Set New Node's currentPosition were mouse cursor is
			if (auto view = newNode->get<NodeView>())
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

    auto nodeRegistry = get_graph_node()->getNodeRegistry();

    for(Node* _eachNode: nodeRegistry)
    {
        if (auto eachView = _eachNode->get<NodeView>())
        {
            eachView->clearConstraints();
        }
    }

    for(Node* _eachNode: nodeRegistry)
    {
        if ( auto eachView = _eachNode->get<NodeView>() )
        {
            auto clss = _eachNode->get_class();

            // Follow previous Node(s), except if previous is a Conditional if/else
            //-------------------------------------------------------------

            auto previousNodes = _eachNode->getPrev();
            std::vector<NodeView*> previousNodesView;
            Node::GetComponents<NodeView>( previousNodes, previousNodesView);
            if ( !previousNodes.empty() && previousNodes[0]->get_class()->is_not<AbstractConditionalStruct>() )
            {
                NodeViewConstraint constraint(NodeViewConstraint::Type::FollowWithChildren);
                constraint.addMasters(previousNodesView);
                constraint.addSlave(eachView);
                eachView->addConstraint(constraint);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            auto children = eachView->getChildren();
            if( !children.empty() && clss->is<AbstractConditionalStruct>() )
            {
                NodeViewConstraint constraint(NodeViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                constraint.addMaster(eachView);
                constraint.addSlaves(children);

                if ( clss->is<ForLoopNode>() )
                {
                    std::vector<NodeView*> next;
                    eachView->getNext(next);
                    constraint.addSlaves(next);
                }

                eachView->addConstraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !eachView->getInputs().empty() )
            {
                NodeViewConstraint constraint(NodeViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.addMaster(eachView);
                constraint.addSlaves(eachView->getInputs());
                eachView->addConstraint(constraint);
            }
        }
    }
}

bool GraphNodeView::update()
{
    GraphNode* graph                 = get_graph_node();
    std::vector<Node*>& nodeRegistry = graph->getNodeRegistry();

    // Find NodeView components
    auto deltaTime = ImGui::GetIO().DeltaTime;
    std::vector<NodeView*> views;
    Node::GetComponents(nodeRegistry, views);

    // updateContraints if needed
    if ( graph->isDirty() )
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
    auto language  = graphNode->getLanguage();
    auto api       = language->getAllFunctions();

    for ( auto it = api.cbegin(); it != api.cend(); it++)
    {
        Invokable* function = *it;
        auto op = language->findOperator(function->get_signature());

        std::string label;
        const FunctionSignature* signature = function->get_signature();
        language->getSerializer()->serialize(label, signature);

        if (op != nullptr )
        {
            auto lambda = [graphNode, op]()->Node*
            {
                return graphNode->newOperator(op);
            };

            add_contextual_menu_item("Operators", label, lambda, signature);
        }
        else
        {
            auto lambda = [graphNode, function]()->Node*
            {
                return graphNode->newFunction(function);
            };

            add_contextual_menu_item("Functions", label, lambda, signature);
        }

    }
}
