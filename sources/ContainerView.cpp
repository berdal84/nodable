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

	// Update NodeViews
	for (auto eachNode : entities)
	{
		if (eachNode->hasComponent("view"))
			eachNode->getComponent("view")->update();
	}

	//  Draw NodeViews
	bool isAnyNodeDragged = false;
	bool isAnyNodeHovered = false;

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

	// Draw input wires
	for (auto eachNode : entities)
	{
		auto wires = eachNode->getWires();

		for (auto eachWire : wires)
		{
			if (eachWire->getTarget()->getOwner() == eachNode)
				eachWire->getView()->draw();
		}
	}

	auto selectedView = NodeView::GetSelected();
	// Deselection
	if (!isAnyNodeHovered && ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
		NodeView::SetSelected(nullptr);


	if (selectedView != nullptr)
	{
		// Deletion
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			selectedView->setVisible(false);
		// Arrange 
		else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
			selectedView->arrangeRecursively();
	}

	// Draft Mouse PAN
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

	if (ImGui::IsMouseClicked(1))
		ImGui::OpenPopup("ContainerViewContextualMenu");

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
	{
		bool updateExpressionButtonClicked = ImGui::MenuItem("Update expression");

		ImGui::Separator();
		
		auto container = getOwner()->getAs<Container*>();

		Entity* newEntity = nullptr;

		if ( ImGui::BeginMenu("New operation")){
			
			if (ImGui::MenuItem("Add"))
				newEntity = container->createNodeAdd();

			if (ImGui::MenuItem("Divide"))
				newEntity = container->createNodeDivide();

			if (ImGui::MenuItem("Multiply"))
				newEntity = container->createNodeMultiply();

			if (ImGui::MenuItem("Substract"))
				newEntity = container->createNodeSubstract();

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("New variable"))
			newEntity = container->createNodeVariable("Variable");

		if (ImGui::MenuItem("New result"))
			newEntity = container->createNodeResult();

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

		if (updateExpressionButtonClicked){
			auto result = container->find("");
			auto app   = container->getOwner()->getAs<Application*>();
			if(result && app)
			{
				app->updateCurrentLineText(result->getValueMember()->getSourceExpression());
			}
		}

	}

	return true;
}