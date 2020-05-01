#include "ContainerView.h"
#include "Log.h"
#include "Parser.h"
#include "Node.h"
#include "Container.h"
#include "Variable.h"
#include "BinaryOperation.h"
#include "Wire.h"
#include "WireView.h"
#include "DataAccess.h"
#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include "NodeView.h"
#include "Application.h"
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <math.h>

using namespace Nodable;

bool ContainerView::draw()
{
	
	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImVec2(0,0));
	auto entities = this->getOwner()->as<Container>()->getEntities();

	/*
		NodeViews
	*/
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
		// Constraints
		auto container = getOwner()->as<Container>();
		auto result    = container->getResultVariable();

		if (result != nullptr) { // Make sure result node is always visible
			auto view = result->getComponent<NodeView>("view");
			auto rect = ImRect(ImVec2(0,0), ImGui::GetWindowSize());
			rect.Max.y = 1000000000000.0f;
			NodeView::ConstraintToRect(view, rect );
		}

		// Update
		for (auto eachNode : entities)
		{
			if (auto view = eachNode->getComponent<View>("view") )
				view->update();
		}

		//  Draw

		for (auto eachNode : entities)
		{
			if (auto view = eachNode->getComponent<View>("view"))
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

	auto draggedConnector = NodeView::GetDraggedConnectorState();
	auto hoveredConnector = NodeView::GetHoveredConnectorState();

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

		// Draw temporary wire on top (overlay draw list)
		if (draggedConnector->member != nullptr)
		{
			ImVec2 lineScreenPosStart;
			{
				auto member   = draggedConnector->member;
				auto node     = member->getOwner()->as<Node>();
				auto view     = node->getComponent<NodeView>("view");
				auto position = view->getConnectorPosition(member->getName(), draggedConnector->side);

				lineScreenPosStart = position + ImGui::GetWindowPos();
			}

			auto lineScreenPosEnd = ImGui::GetMousePos();

			// Snap lineEndPosition to hoveredByMouse member's currentPosition
			if (hoveredConnector->member != nullptr) {
				auto member     = hoveredConnector->member;
				auto node       = member->getOwner()->as<Node>();
				auto view       = node->getComponent<NodeView>("view");
				auto position   = view->getConnectorPosition(member->getName(), hoveredConnector->side);

				lineScreenPosEnd = position + ImGui::GetWindowPos();
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
			if (draggedConnector->member != nullptr &&
				hoveredConnector->member != nullptr)
			{
				if (draggedConnector->member != hoveredConnector->member)
				{
					auto container = getOwner()->as<Container>();
					auto wire      = container->newWire();
					Node::Connect(wire, draggedConnector->member, hoveredConnector->member);
				}

				NodeView::ResetDraggedByMouseMember();

			}// If user release mouse without hovering a member, we display a menu to create a linked node
			else if (draggedConnector->member != nullptr)
			{
				if ( !ImGui::IsPopupOpen("ContainerViewContextualMenu"))
					ImGui::OpenPopup("ContainerViewContextualMenu");	
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
	{	if (ImGui::IsMouseDragging() && ImGui::IsWindowFocused() && !isAnyNodeDragged)
		{
			auto drag = ImGui::GetMouseDragDelta();
			for (auto eachNode : entities)
			{
				if (auto view = eachNode->getComponent< NodeView>("view") ) 
					view->translate(drag);
			}
			ImGui::ResetMouseDragDelta();
		}
	}

	/*
		Mouse right-click popup menu
	*/

	if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
		ImGui::OpenPopup("ContainerViewContextualMenu");
	

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
	{
		auto    container = getOwner()->as<Container>();
		Node* newNode = nullptr;

		// Title :
		View::ColoredShadowedText( ImVec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		/*
			Menu Items...
		*/

		if (ImGui::MenuItem(ICON_FA_PLUS " Add"))
			newNode = container->newAdd();

		if (ImGui::MenuItem(ICON_FA_DIVIDE " Divide"))
			newNode = container->newDivide();

		if (ImGui::MenuItem(ICON_FA_TIMES " Multiply"))
			newNode = container->newMult();

		if (ImGui::MenuItem(ICON_FA_MINUS " Substract"))
			newNode = container->newSub();

		if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
			newNode = container->newVariable("Variable");

		if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Output"))
			newNode = container->newResult();

		/*
			Connect the New Node with the current dragged a member
		*/

		if (draggedConnector->member != nullptr && newNode != nullptr)
		{
			// if dragged member is an inputMember
			if (draggedConnector->member->allows(Connection_In))
				Node::Connect(container->newWire(), newNode->get("result"), draggedConnector->member);

			// if dragged member is an output
			else if (draggedConnector->member->allows(Connection_Out)) {

				// try to get the first Input only member
				auto targetMember = newNode->getFirstWithConn(Connection_In);
				
				// If failed, try to get the first input/output member
				if (targetMember == nullptr)
					targetMember = newNode->getFirstWithConn(Connection_InOut);
				else
					Node::Connect(container->newWire(), draggedConnector->member, targetMember);
			}
			NodeView::ResetDraggedByMouseMember();
		}

		/*
			Set New Node's currentPosition were mouse cursor is 
		*/

		if (newNode != nullptr && newNode->hasComponent("view"))
		{

			if (auto view = newNode->getComponent<NodeView>("view"))
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

