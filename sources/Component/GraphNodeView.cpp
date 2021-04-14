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
#include "Component/WireView.h"
#include "Component/NodeView.h"

using namespace Nodable;

bool GraphNodeView::draw()
{
    bool edited = false;

    Settings* settings = Settings::GetCurrent();
    GraphNode* graph = getGraphNode();
    auto entities    = graph->getNodeRegistry();

	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImVec2(0,0));

    /*
       CodeBlock
     */
    ProgramNode* scope = graph->getProgram();
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
			auto view = graph->getProgram()->getFirstInstruction()->getComponent<NodeView>();
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
			auto members = eachNode->getProps()->getMembers();

			for (auto pair : members)
			{
			    auto end = pair.second;

			    if ( auto start = end->getInputMember() )
                {
			        auto endNodeView   = eachNode->getComponent<NodeView>();
			        auto startNodeView = start->getOwner()->getComponent<NodeView>();
                    auto endPos   = endNodeView->getConnectorPosition(end, Way_In);
                    auto startPos = startNodeView->getConnectorPosition(start, Way_Out);
                    WireView::Draw(ImGui::GetWindowDrawList(), startPos, endPos , startNodeView, endNodeView );
                }
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
                    auto view     = member->getOwner()->getComponent<NodeView>();
                    lineScreenPosStart = view->getConnectorPosition(member, draggedConnector->way);
                }

                auto lineScreenPosEnd = ImGui::GetMousePos();

                // Snap lineEndPosition to hoveredByMouse member's currentPosition
                if (hoveredConnector != nullptr)
                {
                    auto member     = hoveredConnector->member;
                    auto view       = member->getOwner()->getComponent<NodeView>();
                    lineScreenPosEnd = view->getConnectorPosition(member, hoveredConnector->way);
                }

                ImGui::GetOverlayDrawList()->AddLine( lineScreenPosStart,
                                                      lineScreenPosEnd,
                                                      getColor(ColorType_BorderHighlights),
                                                      settings->ui.wire.bezier.thickness * float(0.9));

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
			newNode = graph->newVariable("Variable", graph->getProgram()); // new variable in global scope

		if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Output"))
        {
            newNode = graph->appendInstruction();
        }


		/*
			Connect the New Node with the current dragged a member
		*/

		if (draggedConnector != nullptr && newNode != nullptr)
		{
		    auto props = newNode->getProps();
			// if dragged member is an inputMember
			if (draggedConnector->member->allowsConnection(Way_In))
            {
				graph->connect(props->getFirstWithConn(Way_Out), draggedConnector->member);
            }
			// if dragged member is an output
			else if (draggedConnector->member->allowsConnection(Way_Out))
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

	return edited;
}

void Nodable::GraphNodeView::addContextualMenuItem(std::string _category, std::string _label, std::function<Node*(void)> _function)
{
	contextualMenus.insert( {_category, {_label, _function }} );
}

GraphNode* GraphNodeView::getGraphNode() const
{
    return getOwner()->as<GraphNode>();
}

GraphNodeView::GraphNodeView(): NodeView() {}

void GraphNodeView::drawCodeFlow(AbstractCodeBlockNode* _node)
{
    auto view = _node->getComponent<NodeView>();
    if ( view->isVisible() )
    {

        for (auto each : _node->getChildren())
        {
            if ( each->getClass()->isChildOf(mirror::GetClass<AbstractCodeBlockNode>(), false))
                drawCodeFlow(each->as<AbstractCodeBlockNode>());
            else if ( each->getClass() == mirror::GetClass<AbstractCodeBlockNode>())
                drawCodeFlow(reinterpret_cast<AbstractCodeBlockNode*>(each));
        }

        auto children = view->getChildren();

        // Draw a line between node and its first child OR between all children if it is a conditional structure.
        if ( !children.empty())
        {
            if ( _node->getClass() == mirror::GetClass<ConditionalStructNode>())
            {
                short position(0);
                for(auto eachChild : children)
                {
                    if ( eachChild->isVisible() ) {
                        DrawCodeFlowLine(view, eachChild, (short)children.size(), position);
                        position++;
                    }
                }
            }
            else
            {
                auto endView = children[0];
                if (endView->isVisible())
                    DrawCodeFlowLine(view, endView);
            }
        }

        // Draw a wire between each child except for condition struct
        if (children.size() >= 2 && _node->getClass() != mirror::GetClass<ConditionalStructNode>())
        {
            for(auto it = children.begin(); it < children.end() - 1; it++ )
            {
                // Draw a line
                auto startView = (*it);
                auto endView = (*(it+1));

                if ( startView->isVisible() && endView->isVisible() )
                    DrawCodeFlowLine(startView, endView);
            }
        }
    }
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
            auto children = eachView->getChildren();

            if ( clss->isChildOf(mirror::GetClass<AbstractCodeBlockNode>()))
            {
                if ( children.size() > 1 && clss != mirror::GetClass<ConditionalStructNode>())
                {
                    for (size_t i = 1; i < children.size(); i++)
                    {
                        auto eachChildView = children.at(i);
                        auto previousView = children.at(i - 1);

                        ViewConstraint followConstr(ViewConstraint::Type::FollowWithChildren);
                        followConstr.addMaster(previousView);
                        followConstr.addSlave(eachChildView);
                        eachChildView->addConstraint(followConstr);
                    }
                }

                if( !children.empty() )
                {
                    if ( clss == mirror::GetClass<ConditionalStructNode>())
                    {
                        ViewConstraint followConstr(ViewConstraint::Type::MakeRowAndAlignOnBBoxBottom);
                        followConstr.addMaster(eachView);
                        followConstr.addSlaves(children);
                        eachView->addConstraint(followConstr);
                    }
                    else
                    {
                        auto childView = children[0];
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
                ViewConstraint constraint(ViewConstraint::Type::AlignOnBBoxTR);
                constraint.addSlave(eachView);
                constraint.addMasters(eachView->getOutputs());
                eachView->addConstraint(constraint);
            }

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
