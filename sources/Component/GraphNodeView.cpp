#include "GraphNodeView.h"
#include "Log.h"
#include "Node.h"
#include "GraphNode.h"
#include "VariableNode.h"
#include "Wire.h"
#include "WireView.h"
#include <algorithm>    // for std::find_if
#include "NodeView.h"
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "InstructionNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"

using namespace Nodable;

bool GraphNodeView::draw()
{
    GraphNode* graph = getGraphNode();
    auto entities    = graph->getNodeRegistry();

	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImVec2(0,0));

    /*
       CodeBlock
     */
    ScopedCodeBlockNode* scope = graph->getScope();
    if ( scope && !scope->isEmpty() )
    {
        drawCodeFlow(scope);
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
			auto view = graph->getScope()->getFirstInstruction()->getComponent<NodeView>();
			auto rect = ImRect(ImVec2(0,0), ImGui::GetWindowSize());
			rect.Max.y = 1000000000000.0f;
			NodeView::ConstraintToRect(view, rect );
		}

		// Update
		for (auto eachNode : entities)
		{
			if (auto view = eachNode->getComponent<View>() )
				view->update();
		}

		//  Draw

		for (auto eachNode : entities)
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

	const auto draggedConnector = NodeView::GetDraggedConnector();
	const auto hoveredConnector = NodeView::GetHoveredConnector();

	/*
		Wires
	*/
	{
		

		// Draw existing wires
		for (auto eachNode : entities)
		{
			auto wires = eachNode->getWires();

			for (auto eachWire : wires)
			{
				if (eachWire->getTarget()->getOwner() == eachNode)
					eachWire->getView()->draw();
			}
		}

		if ( ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) )
        {
            // Draw temporary wire on top (overlay draw list)
            if (draggedConnector != nullptr)
            {
                ImVec2 lineScreenPosStart;
                {
                    auto member   = draggedConnector->member;
                    NODABLE_ASSERT(member);
                    NODABLE_ASSERT(member->getOwner() != nullptr);
                    auto node     = member->getOwner()->as<Node>();
                    auto view     = node->getComponent<NodeView>();
                    lineScreenPosStart = view->getConnectorPosition(member, draggedConnector->way);
                }

                auto lineScreenPosEnd = ImGui::GetMousePos();

                // Snap lineEndPosition to hoveredByMouse member's currentPosition
                if (hoveredConnector != nullptr)
                {
                    auto member     = hoveredConnector->member;
                    auto node       = member->getOwner()->as<Node>();
                    auto view       = node->getComponent<NodeView>();
                    lineScreenPosEnd = view->getConnectorPosition(member, hoveredConnector->way);
                }

                ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                      lineScreenPosEnd,
                                                      getColor(ColorType_BorderHighlights),
                                                      connectorRadius * float(0.9));

            }

            // If user release mouse button
            if (ImGui::IsMouseReleased(0))
            {
                // Add a new wire if needed (mouse drag'n drop)
                if (draggedConnector != nullptr &&
                    hoveredConnector != nullptr)
                {
                    if (draggedConnector->member != hoveredConnector->member)
                    {
                        graph->connect(draggedConnector->member, hoveredConnector->member);
                    }

                    NodeView::ResetDraggedConnector();


                }// If user release mouse without hovering a member, we display a menu to create a linked node
                else if (draggedConnector != nullptr)
                {
                    if ( !ImGui::IsPopupOpen("ContainerViewContextualMenu"))
                        ImGui::OpenPopup("ContainerViewContextualMenu");
                }
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
			for (auto eachNode : entities)
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
		if (draggedConnector == nullptr)
        {
            ImGui::OpenPopup("ContainerViewContextualMenu");
        }
		else if (ImGui::IsPopupOpen("ContainerViewContextualMenu"))
		{
			ImGui::CloseCurrentPopup();
			NodeView::ResetDraggedConnector();
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
		
		if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
			newNode = graph->newVariable("Variable", graph->getScope()); // new variable in global scope

		if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Output"))
        {
            newNode = graph->appendInstruction();
        }


		/*
			Connect the New Node with the current dragged a member
		*/

		if (draggedConnector != nullptr && newNode != nullptr)
		{
			// if dragged member is an inputMember
			if (draggedConnector->member->allowsConnection(Way_In))
            {
				graph->connect(newNode->getFirstWithConn(Way_Out), draggedConnector->member);
            }
			// if dragged member is an output
			else if (draggedConnector->member->allowsConnection(Way_Out))
			{
				// try to get the first Input only member
				auto targetMember = newNode->getFirstWithConn(Way_In);
				
				// If failed, try to get the first input/output member
				if (targetMember == nullptr)
                {
                    targetMember = newNode->getFirstWithConn(Way_InOut);
                }
				else
                {
                    graph->connect(draggedConnector->member, targetMember);
                }
			}
			NodeView::ResetDraggedConnector();
		}

		/*
			Set New Node's currentPosition were mouse cursor is 
		*/

		if (newNode != nullptr)
		{
			if (auto view = newNode->getComponent<NodeView>())
			{
				auto pos = ImGui::GetMousePos();
				pos.x -= origin.x;
				pos.y -= origin.y;
				view->setPosition(pos);
			}
			
		}

		ImGui::EndPopup();

	}

	return true;
}

void Nodable::GraphNodeView::addContextualMenuItem(std::string _category, std::string _label, std::function<Node*(void)> _function)
{
	contextualMenus.insert( {_category, {_label, _function }} );
}

GraphNode *GraphNodeView::getGraphNode() const
{
    return getOwner()->as<GraphNode>();
}

GraphNodeView::GraphNodeView(): NodeView() {}

void GraphNodeView::drawCodeFlow(AbstractCodeBlockNode* _node)
{
    for (auto each : _node->getChildren())
    {
        if ( each->getClass()->isChildOf(mirror::GetClass<AbstractCodeBlockNode>()))
            drawCodeFlow(each->as<AbstractCodeBlockNode>());
    }

    auto children = _node->getChildren();

    // Draw a wire to link CodeBlock to each child
    if ( _node->hasComponent<NodeView>())
    {
//        for(auto& eachInstr: children )
//        {
//            // Draw a line
//            ImVec2 start = _node->getComponent<NodeView>()->getScreenPos();
//            ImVec2 end   = eachInstr->getComponent<NodeView>()->getScreenPos();
//            ImColor color(255,255,255,64);
//            ImColor shadowColor(0,0,0,64);
//            WireView::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor, 5.0f);
//        }

        if ( !children.empty())
        {
            auto startView = _node->getComponent<NodeView>();
            auto endView = children[0]->getComponent<NodeView>();
            DrawCodeFlowLine(startView, endView);
        }
    }

    // Draw a wire between each child
    if (children.size() >= 2 )
    {
        for(auto it = children.begin(); it < children.end() - 1; it++ )
        {
            // Draw a line
            auto startView = (*it)->getComponent<NodeView>();
            auto endView = (*(it+1))->getComponent<NodeView>();
            DrawCodeFlowLine(startView, endView);
        }
    }
}

void GraphNodeView::DrawCodeFlowLine(NodeView *startView, NodeView *endView) {
    ImVec2 start = startView->getScreenPos();
    ImVec2 end   = endView->getScreenPos();
    ImColor color(200,255,200,50);
    ImColor shadowColor(0,0,0,64);
    float width = std::min(endView->getRect().GetSize().x, startView->getRect().GetSize().x) - 5.0f;
    WireView::DrawVerticalWire(ImGui::GetWindowDrawList(), start, end, color, shadowColor, width);
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

            //     (1) CodeBlockNode must follow average position of each inner block
            // AND (2) Each child must follow its previous
            if ( clss->isChildOf(mirror::GetClass<AbstractCodeBlockNode>()))
            {
                // (1)
//                ViewConstraint constraint(ViewConstraint::Type::AlignOnBBoxLeft);
//                constraint.addSlave(eachView);
//                for (auto instr : _eachNode->getChildren())
//                {
//                    if (auto instrView = instr->getComponent<NodeView>())
//                    {
//                        constraint.addMaster(instrView);
//                    }
//                }
//                eachView->addConstraint(constraint);

                // (2)
                if ( _eachNode->getChildren().size() > 1 )
                {
                    for (size_t i = 1; i < _eachNode->getChildren().size(); i++)
                    {
                        auto eachChildView = _eachNode->getChildren().at(i)->getComponent<NodeView>();
                        auto previousView = _eachNode->getChildren().at(i - 1)->getComponent<NodeView>();

                        ViewConstraint followConstr(ViewConstraint::Type::FollowWithChildren);
                        followConstr.addMaster(previousView);
                        followConstr.addSlave(eachChildView);
//                        followConstr.offset = ImVec2(20.0f, 0);
                        eachChildView->addConstraint(followConstr);
                    }
                }

                if( !_eachNode->getChildren().empty() )
                {
                    if ( auto childView = _eachNode->getChildren()[0]->getComponent<NodeView>())
                    {
                        ViewConstraint followConstr(ViewConstraint::Type::FollowWithChildren);
                        followConstr.addMaster(eachView);
                        followConstr.addSlave(childView);
                        followConstr.offset = ImVec2(30.0f, 0); // indent
                        childView->addConstraint(followConstr);
                    }
                }
            }

            // Each Node with more than 1 output needs to be aligned with the bbox top of output nodes
            if ( _eachNode->getOutputs().size() > 1 )
            {
                ViewConstraint constraint(ViewConstraint::Type::AlignOnBBoxTop);
                constraint.addSlave(eachView);

                for (auto eachOutput : _eachNode->getOutputs())
                {
                    if (auto eachOutView = eachOutput->getComponent<NodeView>())
                    {
                        constraint.addMaster(eachOutView);
                    }
                }
                eachView->addConstraint(constraint);
            }

            // Input nodes must be aligned to their output
            if ( !_eachNode->getInputs().empty() )
            {
                ViewConstraint constraint(ViewConstraint::Type::MakeRowAndAlignOnBBoxTop);
                constraint.addMaster(eachView);
                for (auto eachInput : _eachNode->getInputs())
                {
                    if (auto eachInputView = eachInput->getComponent<NodeView>())
                    {
                        constraint.addSlave(eachInputView);
                    }
                }
                eachView->addConstraint(constraint);
            }
        }
    }
}
