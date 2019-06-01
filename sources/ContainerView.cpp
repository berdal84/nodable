#include "ContainerView.h"
#include "Log.h"
#include "Lexer.h"
#include "Entity.h"
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

using namespace Nodable;

bool ContainerView::draw()
{
	
	auto origin = ImGui::GetCursorScreenPos();
	auto entities = this->getOwner()->getAs<Container*>()->getEntities();

	/*
		NodeViews
	*/
	NodeView::memberHoveredByMouse = nullptr; // reset this var befor drawing
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
		// Update
		for (auto eachNode : entities)
		{
			if (eachNode->hasComponent("view"))
				eachNode->getComponent("view")->update();
		}

		//  Draw

		for (auto eachNode : entities)
		{
			if (eachNode->hasComponent("view"))
			{
				auto view = eachNode->getComponent("view")->getAs<View*>();

				if (view != nullptr && view->isVisible())
				{
					view->draw();
					isAnyNodeDragged |= NodeView::GetDragged() == view;
					isAnyNodeHovered |= view->isHovered();
				}
			}
		}
	}
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
		if (NodeView::memberDraggedByMouse != nullptr)
		{
			auto lineStartPosition = NodeView::memberDraggedByMouse->getOwner()->getAs<Entity*>()->getComponent("view")->getAs<NodeView*>()->getInputPosition(NodeView::memberDraggedByMouse->getName()) + ImGui::GetWindowPos();
			auto lineEndPosition = ImGui::GetMousePos();

			// Snap lineEndPosition to hoveredByMouse member's position
			if (NodeView::memberHoveredByMouse != nullptr)
				lineEndPosition = NodeView::memberHoveredByMouse->getOwner()->getAs<Entity*>()->getComponent("view")->getAs<NodeView*>()->getInputPosition(NodeView::memberHoveredByMouse->getName()) + ImGui::GetWindowPos();
			
			ImGui::GetOverlayDrawList()->AddLine(lineStartPosition, lineEndPosition, getColor(ColorType_BorderHighlights), connectorRadius * float(0.9));
		}

		// If user release mouse button
		if (!ImGui::IsMouseDown(0))
		{
			// Add a new wire if needed (mouse drag'n drop)
			if (NodeView::memberDraggedByMouse != nullptr &&
				NodeView::memberHoveredByMouse != nullptr)
			{
				if (NodeView::memberDraggedByMouse != NodeView::memberHoveredByMouse)
				{
					auto wire = NodeView::memberDraggedByMouse->getOwner()->getAs<Entity*>()->getParent()->createWire();
					Entity::Connect(wire, NodeView::memberDraggedByMouse, NodeView::memberHoveredByMouse);
				}

				NodeView::memberDraggedByMouse = nullptr;
				NodeView::memberHoveredByMouse = nullptr;

			}// If user release mouse without hovering a member, we display a menu to create a linked node
			else if (NodeView::memberDraggedByMouse != nullptr)
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
				((NodeView*)eachNode->getComponent("view"))->translate(drag);
			}
			ImGui::ResetMouseDragDelta();
		}
	}

	/*
		Mouse right-click popup menu
	*/

	if (ImGui::IsMouseClicked(1))
		ImGui::OpenPopup("ContainerViewContextualMenu");

	

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
	{
		auto container = getOwner()->getAs<Container*>();
		Entity* newEntity = nullptr;

		// Title :
		View::ColoredShadowedText( ImVec2(1,1), ImColor(0.00f, 0.00f, 0.00f, 1.00f), ImColor(1.00f, 1.00f, 1.00f, 0.50f), "Create new node :");
		ImGui::Separator();

		/*
			Menu Items...
		*/

		if (ImGui::MenuItem(ICON_FA_PLUS " Add"))
			newEntity = container->createNodeAdd();

		if (ImGui::MenuItem(ICON_FA_DIVIDE " Divide"))
			newEntity = container->createNodeDivide();

		if (ImGui::MenuItem(ICON_FA_TIMES " Multiply"))
			newEntity = container->createNodeMultiply();

		if (ImGui::MenuItem(ICON_FA_MINUS " Substract"))
			newEntity = container->createNodeSubstract();

		if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
			newEntity = container->createNodeVariable("Variable");

		if (ImGui::MenuItem(ICON_FA_EQUALS " Result"))
			newEntity = container->createNodeResult();

		/*
			Connect the New Entity with the current dragged a member
		*/

		if (NodeView::memberDraggedByMouse != nullptr && newEntity != nullptr)
		{
			// if dragged member is an input
			if (NodeView::memberDraggedByMouse->allows(Connection_In) && newEntity->getMember("result") != nullptr)
				Entity::Connect(container->createWire(), newEntity->getMember("result"), NodeView::memberDraggedByMouse);

			// if dragged member is an output
			else if (NodeView::memberDraggedByMouse->allows(Connection_Out)) {

				// try to get the first Input only member
				auto targetMember = newEntity->getFirstMemberWithConnection(Connection_In);
				
				// If failed, try to get the first input/output member
				if (targetMember == nullptr)
					targetMember = newEntity->getFirstMemberWithConnection(Connection_InOut);

				if ( targetMember != nullptr)
					Entity::Connect(container->createWire(), NodeView::memberDraggedByMouse, targetMember);
			}
			NodeView::memberDraggedByMouse = nullptr;
		}

		/*
			Set New Entity's position were mouse cursor is 
		*/

		if (newEntity != nullptr && newEntity->hasComponent("view"))
		{
			auto view = static_cast<NodeView*>(newEntity->getComponent("view"));

			if (view != nullptr)
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