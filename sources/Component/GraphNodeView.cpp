#include "GraphNodeView.h"

#include <algorithm>
#include <utility>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "Core/Settings.h"
#include "Core/Log.h"
#include "Core/Wire.h"
#include "Core/Application.h"
#include "Node/ProgramNode.h"
#include "Node/GraphNode.h"
#include "Node/VariableNode.h"
#include "Node/LiteralNode.h"
#include "Component/WireView.h"
#include "Component/NodeView.h"

using namespace Nodable;

bool GraphNodeView::draw()
{
    bool edited = false;
    VirtualMachine* vm = &Application::s_instance->getVirtualMachine();

    Settings* settings = Settings::GetCurrent();
    GraphNode* graph = getGraphNode();
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
        for (auto& each_next : each_node->getNext() )
        {
            NodeView *each_view      = each_node->getComponent<NodeView>();
            NodeView *each_next_view = each_next->getComponent<NodeView>();
            if (each_view && each_next_view && each_view->isVisible() && each_next_view->isVisible() )
            {
                DrawCodeFlowLine(each_view, each_next_view, slot_count, slot_index);
            }
            ++slot_index;
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

                if ( auto start = end->getInputMember() )
                {
                    auto endNodeView   = eachNode->getComponent<NodeView>();
                    auto startNodeView = start->getOwner()->getComponent<NodeView>();

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

                if( vm && vm->isDebugging() && vm->getCurrentNode() == eachNodeView->getOwner())
                    ImGui::SetScrollHereY();

                // dragging
                if (GetDragged() == eachNodeView && ImGui::IsMouseDragging(0))
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

	// Connector Drag'n Drop
    if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
    {
        // Draw temporary Member connection
        if (auto draggedMemberConnector = MemberConnector::GetDragged())
        {
            auto hoveredMemberConnector = MemberConnector::GetHovered();
            ImVec2 lineScreenPosStart = draggedMemberConnector->getPos();
            ImVec2 lineScreenPosEnd   = hoveredMemberConnector ? hoveredMemberConnector->getPos() : ImGui::GetMousePos();

            ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                  lineScreenPosEnd,
                                                  getColor(ColorType_BorderHighlights),
                                                  settings->ui.wire.bezier.thickness * float(0.9));

        }

        // Draw temporary Node connection
        if (auto draggedNodeConnector = NodeConnector::GetDragged())
        {
            auto hoveredNodeConnector = NodeConnector::GetHovered();
            ImVec2 lineScreenPosStart = draggedNodeConnector->getPos();
            ImVec2 lineScreenPosEnd   = hoveredNodeConnector ? hoveredNodeConnector->getPos() : ImGui::GetMousePos();

            ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                  lineScreenPosEnd,
                                                  getColor(ColorType_BorderHighlights),
                                                  settings->ui.codeFlow.lineWidthMax);
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

	// Virtual Machine cursor
	if( vm )
    {
	    if ( !vm->isStopped())
        {
	        auto node = vm->getCurrentNode();
	        if( auto view = node->getComponent<NodeView>())
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
            if (auto view = eachNode->getComponent<NodeView>() )
                view->translate(drag);
        }
        ImGui::ResetMouseDragDelta();
    }

	/*
		Mouse right-click popup menu
	*/

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
	{
		if ( !MemberConnector::IsDragging() && !NodeConnector::IsDragging())
        {
            ImGui::OpenPopup("ContainerViewContextualMenu");
        }
	}

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
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
				auto range = contextualMenus.equal_range(_key);
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
                        TokenType dragged_member_type = graph->getLanguage()->getSemantic()->typeToTokenType(dragged_member_conn->getMember()->getType());

                        if ( dragged_member_conn->m_way == Way_Out )
                        {
                            has_compatible_signature = menu_item.function_signature.hasAtLeastOneArgOfType(dragged_member_type);
                        }
                        else
                        {
                            has_compatible_signature = menu_item.function_signature.getType() == dragged_member_type;
                        }
                    }

					if ( has_compatible_signature && ImGui::MenuItem( menu_item.label.c_str() ))
					{
						if ( menu_item.create_node_fct  )
							newNode = menu_item.create_node_fct();
						else
							LOG_WARNING( "GraphNodeView", "The function associated to the key %s is nullptr", menu_item.label.c_str() );
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
                    newNode = graph->newVariable(MemberConnector::GetDragged()->getMember()->getType(), "var", graph->getProgram());
            }
            else if ( ImGui::BeginMenu("Variable") )
            {
                if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                    newNode = graph->newVariable(Type_Boolean, "var", graph->getProgram());

                if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                    newNode = graph->newVariable(Type_Double, "var", graph->getProgram());

                if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                    newNode = graph->newVariable(Type_String, "var", graph->getProgram());

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

        if (ImGui::MenuItem(ICON_FA_CODE " Instruction"))
            newNode = graph->newInstruction_UserCreated();

        if( !is_dragging_member_connector )
        {
            if (ImGui::MenuItem(ICON_FA_CODE " Condition"))
                newNode = graph->newConditionalStructure();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Block"))
                newNode = graph->newCodeBlock();

            if (ImGui::MenuItem(ICON_FA_CODE " Scope"))
                newNode = graph->newScopedCodeBlock();

            ImGui::Separator();

            if (ImGui::MenuItem(ICON_FA_CODE " Program"))
                newNode = graph->newProgram();
        }

        if (newNode)
        {
            // dragging node connector ?
            if (auto draggedNodeConnector = NodeConnector::GetDragged())
            {
                //  [ dragged ](next) ---- dragging this way ----> (prev)[ new node ]
                if ( draggedNodeConnector->m_way == Way_Out )
                {
                    graph->connect(newNode, draggedNodeConnector->getNode(), RelationType::IS_NEXT_OF);
                }
                //  [ new node ](next) <--- dragging this way ---- (prev)[ dragged ]
                else
                {
                    graph->connect(draggedNodeConnector->getNode(), newNode, RelationType::IS_NEXT_OF);
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
			if (auto view = newNode->getComponent<NodeView>())
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

void Nodable::GraphNodeView::addContextualMenuItem(
        const std::string& _category,
        std::string _label,
        std::function<Node*(void)> _function,
        const FunctionSignature& _signature)
{
	contextualMenus.insert( {_category, {std::move(_label), std::move(_function), _signature }} );
}

GraphNode* GraphNodeView::getGraphNode() const
{
    return getOwner()->as<GraphNode>();
}

void GraphNodeView::DrawCodeFlowLine(NodeView *startView, NodeView *endView, short _slotCount, short _slotPosition)
{
    float padding      = 2.0f;
    float linePadding  = 5.0f;
    float viewWidthMin = std::min(endView->getRect().GetSize().x, startView->getRect().GetSize().x);
    float lineWidth    = std::min(Settings::GetCurrent()->ui.codeFlow.lineWidthMax, viewWidthMin / float(_slotCount) - (padding * 2.0f));

    ImVec2 start     = startView->getScreenPos();
    start.x          -= std::max(startView->getSize().x * 0.5f, lineWidth * float(_slotCount) * 0.5f);
    start.x          += lineWidth * 0.5f + float(_slotPosition) * lineWidth;

    ImVec2 end   = endView->getScreenPos();
    end.x -= endView->getSize().x * 0.5f;
    end.x += lineWidth * 0.5f;

    ImColor color(200,255,200,50);
    ImColor shadowColor(0,0,0,64);
    ImGuiEx::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor, lineWidth - linePadding*2.0f, 0.0f);
}

void GraphNodeView::updateViewConstraints()
{
    LOG_VERBOSE("GraphNodeView", "updateViewConstraints()\n");

    auto nodeRegistry = getGraphNode()->getNodeRegistry();

    for(Node* _eachNode: nodeRegistry)
    {
        if (auto eachView = _eachNode->getComponent<NodeView>())
        {
            eachView->clearConstraints();
        }
    }

    for(Node* _eachNode: nodeRegistry)
    {
        if ( auto eachView = _eachNode->getComponent<NodeView>() )
        {
            auto clss = _eachNode->getClass();

            // Follow previous Node(s), except if previous is a Conditional Structure Node.
            //-----------------------------------------------------------------------------

            auto previousNodes = _eachNode->getPrev();
            std::vector<NodeView*> previousNodesView;
            Node::GetComponents<NodeView>( previousNodes, previousNodesView);
            if ( !previousNodes.empty() && !previousNodes[0]->as<ConditionalStructNode>())
            {
                ViewConstraint constraint(ViewConstraint::Type::FollowWithChildren);
                constraint.addMasters(previousNodesView);
                constraint.addSlave(eachView);
                eachView->addConstraint(constraint);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            auto children = eachView->getChildren();
            if( !children.empty() && clss == mirror::GetClass<ConditionalStructNode>())
            {
                ViewConstraint constraint(ViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                constraint.addMaster(eachView);
                constraint.addSlaves(children);
                eachView->addConstraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !eachView->getInputs().empty() )
            {
                ViewConstraint constraint(ViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.addMaster(eachView);
                constraint.addSlaves(eachView->getInputs());
                eachView->addConstraint(constraint);
            }
        }
    }
}

bool GraphNodeView::update()
{
    GraphNode* graph                 = getGraphNode();
    std::vector<Node*>& nodeRegistry = graph->getNodeRegistry();

    // Constraints
    if (auto program = graph->getProgram() )
    {
        // Make sure result node is always visible
        auto view = program->getComponent<NodeView>();
        auto rect = ImRect(ImVec2(0,0), ImGui::GetWindowSize());
        rect.Expand(-20.f);
        rect.Max.y = std::numeric_limits<float>::max();
        NodeView::ConstraintToRect(view, rect );
    }

    // Find NodeView components
    auto deltaTime = ImGui::GetIO().DeltaTime;
    std::vector<NodeView*> views;
    Node::GetComponents(nodeRegistry, views);

    // Apply constraints
    for (auto eachView : views)
        if( eachView->isVisible() )
            eachView->applyConstraints(deltaTime);

    // Update
    for (auto eachView : views)
        eachView->update();

    return true;
}

void GraphNodeView::setOwner(Node* _owner)
{
    NodeView::setOwner( _owner );

    // create contextual menu items (not sure this is relevant, but it is better than in File class ^^)
    auto graphNode = _owner->as<GraphNode>();
    auto language  = graphNode->getLanguage();
    auto api       = language->getAllFunctions();

    for ( auto it = api.cbegin(); it != api.cend(); it++)
    {
        const auto function = &*it;
        auto op = language->findOperator(function->signature);

        if (op != nullptr )
        {
            auto lambda = [graphNode, op]()->Node*
            {
                return graphNode->newOperator(op);
            };

            auto label = op->signature.getLabel();
            addContextualMenuItem("Operators", label, lambda, op->signature);
        }
        else
        {
            auto lambda = [graphNode, function]()->Node*
            {
                return graphNode->newFunction(function);
            };

            std::string label;
            language->getSerializer()->serialize(label, (*it).signature);
            addContextualMenuItem("Functions", label, lambda, (*it).signature);
        }

    }
}
