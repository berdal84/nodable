#include "GraphNodeView.h"

#include <algorithm>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "Core/Settings.h"
#include "Core/Log.h"
#include "Core/Wire.h"
#include "Core/Application.h"
#include "Node/ProgramNode.h"
#include "Node/GraphNode.h"
#include "Node/VariableNode.h"
#include "Node/InstructionNode.h"
#include "Node/LiteralNode.h"
#include "Component/WireView.h"
#include "Component/NodeView.h"

using namespace Nodable;

bool GraphNodeView::draw()
{
    bool edited = false;

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
        for (auto& each_next : each_node->getNext() )
        {
            NodeView *each_view      = each_node->getComponent<NodeView>();
            NodeView *each_next_view = each_next->getComponent<NodeView>();
            if (each_view && each_next_view && each_view->isVisible() && each_next_view->isVisible() )
            {
                DrawCodeFlowLine(each_view, each_next_view);
            }
        }
    }


	/*
		NodeViews
	*/
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
		// Constraints
		if (graph->hasInstructionNodes() )
		{
            // Make sure result node is always visible
			auto view = graph->getProgram()->getFirstInstruction()->getComponent<NodeView>();
			auto rect = ImRect(ImVec2(0,0), ImGui::GetWindowSize());
			rect.Max.y = std::numeric_limits<float>::max();
			NodeView::ConstraintToRect(view, rect );
		}

		// Apply Forces
        auto deltaTime = ImGui::GetIO().DeltaTime;
        for (auto eachNode : nodeRegistry)
        {
            if (auto view = eachNode->getComponent<NodeView>())
            {
                if( view->isVisible() )
                view->applyConstraints(deltaTime);
            }
        }

		// Update
		for (auto eachNode : nodeRegistry)
		{
			if (auto view = eachNode->getComponent<NodeView>() )
				view->update();
		}

		//  Draw (Wires first, Node after)
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

		for (auto eachNode : nodeRegistry)
		{
			if (auto view = eachNode->getComponent<View>())
			{
				if (view->isVisible())
				{
					view->draw();
					isAnyNodeDragged |= NodeView::GetDragged() == view;
					isAnyNodeHovered |= view->isHovered();
				}
			}
		}
	}

	const auto draggedMemberConnector = NodeView::GetDraggedMemberConnector();
	const auto hoveredMemberConnector = NodeView::GetHoveredMemberConnector();
    const auto draggedNodeConnector = NodeView::GetDraggedNodeConnector();
    const auto hoveredNodeConnector = NodeView::GetHoveredNodeConnector();
	{
		if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
        {
            // Draw temporary wire on top (overlay draw list)
            if (draggedMemberConnector)
            {
                ImVec2 lineScreenPosStart = draggedMemberConnector->getPos();
                ImVec2 lineScreenPosEnd   = hoveredMemberConnector ? hoveredMemberConnector->getPos() : ImGui::GetMousePos();

                ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                      lineScreenPosEnd,
                                                      getColor(ColorType_BorderHighlights),
                                                      settings->ui.wire.bezier.thickness * float(0.9));

            }

            // Draw temporary wire on top (overlay draw list)
            if (draggedNodeConnector)
            {
                ImVec2 lineScreenPosStart = draggedNodeConnector->getScreenPos();
                ImVec2 lineScreenPosEnd   = hoveredNodeConnector ? hoveredNodeConnector->getScreenPos() : ImGui::GetMousePos();

                ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                      lineScreenPosEnd,
                                                      getColor(ColorType_BorderHighlights),
                                                      settings->ui.codeFlow.lineWidthMax);
            }

            // If user release mouse button
            if (ImGui::IsMouseReleased(0))
            {
                bool openPopUp = false;

                // Add a new wire if needed (mouse drag'n drop)
                if (draggedMemberConnector && hoveredMemberConnector)
                {
                    if ( !MemberConnector::ShareSameMember(draggedMemberConnector, hoveredMemberConnector) )
                    {
                        graph->connect(draggedMemberConnector->getMember(), hoveredMemberConnector->getMember() );
                    }
                    NodeView::ResetDraggedMemberConnector();

                }// If user release mouse without hovering a member, we display a menu to create a linked node
                else if (draggedMemberConnector != nullptr)
                {
                    openPopUp = true;
                }

                // Add a new wire if needed (mouse drag'n drop)
                if (draggedNodeConnector && hoveredNodeConnector)
                {
                    if ( !NodeConnector::ShareSameNode(draggedNodeConnector, hoveredNodeConnector) )
                    {
                        graph->connect(draggedNodeConnector->getNode(), hoveredNodeConnector->getNode(), RelationType::IS_NEXT_OF );
                    }

                    NodeView::ResetDraggedNodeConnector();


                }// If user release mouse without hovering a member, we display a menu to create a linked node
                else if (draggedNodeConnector != nullptr)
                {
                    openPopUp = true;
                }

                if ( openPopUp && !ImGui::IsPopupOpen("ContainerViewContextualMenu"))
                    ImGui::OpenPopup("ContainerViewContextualMenu");
            }
	    }
	}

	// Virtual Machine cursor
	if( VirtualMachine* vm = &Application::s_instance->getVirtualMachine() )
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

	bool isMousePanEnable = false;
	if (isMousePanEnable)
	{	if (ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused() && !isAnyNodeDragged)
		{
			auto drag = ImGui::GetMouseDragDelta();
			for (auto eachNode : nodeRegistry)
			{
				if (auto view = eachNode->getComponent<NodeView>() ) 
					view->translate(drag);
			}
			ImGui::ResetMouseDragDelta();
		}
	}

	/*
		Mouse right-click popup menu
	*/

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
	{
		if (draggedMemberConnector == nullptr)
        {
            ImGui::OpenPopup("ContainerViewContextualMenu");
        }
		else if (ImGui::IsPopupOpen("ContainerViewContextualMenu"))
		{
			ImGui::CloseCurrentPopup();
            NodeView::ResetDraggedMemberConnector();
            NodeView::ResetDraggedNodeConnector();
		}
	}

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
	{
		Node* newNode = nullptr;

		// Title :
		View::ColoredShadowedText( ImVec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		/*
			Menu Items...
		*/


		auto drawMenu = [&](const std::string _key)-> void {
			char menuLabel[255];
			snprintf( menuLabel, 255, ICON_FA_CALCULATOR" %s", _key.c_str());

			if (ImGui::BeginMenu(menuLabel))
			{		
				auto range = contextualMenus.equal_range(_key);
				for (auto it = range.first; it != range.second; it++)
				{
					auto labelFunctionPair = it->second;
					auto itemLabel = labelFunctionPair.first.c_str();

					if (ImGui::MenuItem(itemLabel))
					{
						if ( labelFunctionPair.second != nullptr  )
							newNode = labelFunctionPair.second();
						else
							LOG_WARNING( "GraphNodeView", "The function associated to the key %s is nullptr", itemLabel );
					}
				}

				ImGui::EndMenu();
			}	
		};

		drawMenu("Operators");
		drawMenu("Functions");

		ImGui::Separator();

        if (ImGui::BeginMenu("Variable"))
        {
            if (ImGui::MenuItem(ICON_FA_DATABASE " Boolean"))
                newNode = graph->newVariable(Type_Boolean, "var", graph->getProgram());

            if (ImGui::MenuItem(ICON_FA_DATABASE " Double"))
                newNode = graph->newVariable(Type_Double, "var", graph->getProgram());

            if (ImGui::MenuItem(ICON_FA_DATABASE " String"))
                newNode = graph->newVariable(Type_String, "var", graph->getProgram());

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Literal"))
        {
            if (ImGui::MenuItem(ICON_FA_FILE " Boolean"))
                newNode = graph->newLiteral(Type_Boolean);

            if (ImGui::MenuItem(ICON_FA_FILE " Double"))
                newNode = graph->newLiteral(Type_Double);

            if (ImGui::MenuItem(ICON_FA_FILE " String"))
                newNode = graph->newLiteral(Type_String);

            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem(ICON_FA_CODE " Instruction"))
            newNode = graph->newInstruction_UserCreated();

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


        if (newNode)
        {
            if (draggedNodeConnector)
            {
                if ( draggedNodeConnector->m_way == Way_Out )
                {
                    graph->connect(newNode, draggedNodeConnector->getNode(), RelationType::IS_NEXT_OF);
                }
                else
                {
                    graph->connect(draggedNodeConnector->getNode(), newNode, RelationType::IS_NEXT_OF);
                }
                NodeView::ResetDraggedNodeConnector();
            }

            // Connect the New Node with the current dragged a member
            if (draggedMemberConnector)
            {
                auto props = newNode->getProps();
                // if dragged member is an m_inputMember
                if (draggedMemberConnector->m_memberView->m_in)
                {
                    graph->connect(props->getFirstWithConn(Way_Out), draggedMemberConnector->m_memberView->m_member);
                }
                // if dragged member is an output
                else if (draggedMemberConnector->m_memberView->m_out)
                {
                    // try to get the first Input only member
                    auto targetMember = props->getFirstWithConn(Way_In);

                    // If failed, try to get the first input/output member
                    if (targetMember == nullptr)
                    {
                        targetMember = props->getFirstWithConn(Way_InOut);
                    }
                    else
                    {
                        graph->connect(draggedMemberConnector->m_memberView->m_member, targetMember);
                    }
                }
                NodeView::ResetDraggedMemberConnector();
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

	return edited;
}

void Nodable::GraphNodeView::addContextualMenuItem(const std::string& _category, std::string _label, std::function<Node*(void)> _function)
{
	contextualMenus.insert( {_category, {_label, _function }} );
}

GraphNode* GraphNodeView::getGraphNode() const
{
    return getOwner()->as<GraphNode>();
}

GraphNodeView::GraphNodeView(): NodeView() {}

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
    WireView::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor, lineWidth - linePadding*2.0f, 0.0f);
}

void GraphNodeView::updateViewConstraints()
{
    LOG_VERBOSE("GraphNodeView", "updateViewConstraints()\n");

    for(Node* _eachNode: this->getGraphNode()->getNodeRegistry()) {
        if (auto eachView = _eachNode->getComponent<NodeView>()) {
            eachView->clearConstraints();
        }
    }

    for(Node* _eachNode: this->getGraphNode()->getNodeRegistry())
    {
        if ( auto eachView = _eachNode->getComponent<NodeView>() )
        {
            auto clss = _eachNode->getClass();

            // follow prev
            auto prev = _eachNode->getPrev();
            if ( !prev.empty())
            {
                ViewConstraint followConstr(ViewConstraint::Type::FollowWithChildren);
                for(auto eachPrev : prev )
                {
                    if (auto eachPrevView = eachPrev->getComponent<NodeView>())
                        followConstr.addMaster(eachPrevView);
                }
                followConstr.addSlave(eachView);

                // indent if previous is parent
//                if ( prev[0] == _eachNode->getParent() )
//                    followConstr.offset = ImVec2(30.0f, 0);

                eachView->addConstraint(followConstr);
            }


            auto children = eachView->getChildren();
            if( children.size() > 1 && clss == mirror::GetClass<ConditionalStructNode>())
            {
                ViewConstraint followConstr(ViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                followConstr.addMaster(eachView);
                followConstr.addSlaves(children);
                eachView->addConstraint(followConstr);
            }

            // Each Node with more than 1 output needs to be aligned with the bbox top of output nodes
//            if ( _eachNode->getOutputs().size() > 1 )
//            {
//                ViewConstraint constraint(ViewConstraint::Type::AlignOnBBoxTop);
//                constraint.addSlave(eachView);
//                constraint.addMasters(eachView->getOutputs());
//                eachView->addConstraint(constraint);
//            }

            // Input nodes must be aligned to their output
            if ( !_eachNode->getInputs().empty() )
            {
                ViewConstraint constraint(ViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.addMaster(eachView);
                constraint.addSlaves(eachView->getInputs());
                eachView->addConstraint(constraint);
            }
        }
    }
}
