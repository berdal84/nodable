#include "ContainerView.h"
#include "Log.h"
#include "Parser.h"
#include "Node.h"
#include "Container.h"
#include "Variable.h"
#include "Wire.h"
#include "WireView.h"
#include <algorithm>    // for std::find_if
#include "NodeView.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

bool ContainerView::draw()
{
	
	auto origin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorPos(ImVec2(0,0));
	auto container = reinterpret_cast<Container*>(getOwner());
	auto entities  = container->getEntities();

	/*
		NodeViews
	*/
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;
	{
		// Constraints
		auto result    = container->getResultVariable();

		if (result != nullptr) { // Make sure result node is always visible
			auto view = result->getComponent<NodeView>();
			auto rect = ImRect(ImVec2(0,0), ImGui::GetWindowSize());
			rect.Max.y = 1000000000000.0f;
			NodeView::ConstraintToRect(view, rect );
		}

		// Update
		for (const auto eachNode : entities)
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
				if (eachWire->getTarget()->getOwner() == eachNode.get() )
					eachWire->getView()->draw();
			}
		}

		// Draw temporary wire on top (overlay draw list)
		if (draggedConnector != nullptr)
		{
			ImVec2 lineScreenPosStart;
			{
				auto position = draggedConnector->position();
				lineScreenPosStart = position + ImGui::GetWindowPos();
			}

			auto lineScreenPosEnd = ImGui::GetMousePos();

			// Snap lineEndPosition to hoveredByMouse member's currentPosition
			if (hoveredConnector != nullptr) {
				auto position = hoveredConnector->position();
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
			if (draggedConnector != nullptr &&
				hoveredConnector != nullptr)
			{
			    auto source = draggedConnector->member.lock();
			    auto target = hoveredConnector->member.lock();

				if ( source != target)
				{
					Node::Connect(source, target);
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
				if (auto view = eachNode->getComponent<NodeView>() ) 
					view->translate(drag);
			}
			ImGui::ResetMouseDragDelta();
		}
	}

	/*
		Mouse right-click popup menu
	*/

	if (ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1)) {
		if (draggedConnector == nullptr)
			ImGui::OpenPopup("ContainerViewContextualMenu");
		else if (ImGui::IsPopupOpen("ContainerViewContextualMenu")) {
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
							LOG_WARNING( 1u, "The function associated to the key %s is nullptr", itemLabel );					
					}
				}

				ImGui::EndMenu();
			}	
		};

		drawMenu("Operators");
		drawMenu("Functions");

		ImGui::Separator();
		
		if (ImGui::MenuItem(ICON_FA_DATABASE " Variable"))
			newNode = container->newVariable("Variable").get();

		if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Output"))
			newNode = container->newResult();


		/*
			Connect the New Node with the current dragged a member
		*/

		if (draggedConnector != nullptr && newNode != nullptr)
		{
			// if dragged member is an inputMember
			auto draggedMember = draggedConnector->member.lock();
			if ( draggedMember->allowsConnections(Way::In) )
            {
				Node::Connect( newNode->getFirstWithConn(Way::Out), draggedMember );

			// if dragged member is an output
			}
			else if ( draggedMember->allowsConnections(Way::Out) )
			{

				// try to get the first Input only member
				auto targetMember = newNode->getFirstWithConn(Way::In);
				
				// If failed, try to get the first input/output member
				if (targetMember == nullptr)
				{
                    targetMember = newNode->getFirstWithConn(Way::InOut);
                }

				Node::Connect(draggedMember, targetMember);
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


void Nodable::ContainerView::addContextualMenuItem(std::string _category, std::string _label, std::function<Node*(void)> _function)
{
	contextualMenus.insert( {_category, {_label, _function }} );
}

