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
	auto entities = this->getOwner()->getAs<Container*>()->getEntities();
	
	// Update NodeViews
	for(auto eachNode : entities)
	{
		eachNode->getComponent("view")->update();
	}

	//  Draw NodeViews
	bool isAnyItemDragged = false;
	bool isAnyItemHovered = false;	

	for(auto eachNode : entities)
	{
		auto view = eachNode->getComponent("view")->getAs<View*>();

		if (view != nullptr)
		{
			view->draw();
			isAnyItemDragged |= NodeView::GetDragged() == view;
			isAnyItemHovered |= view->isHovered();
		}
	}

	// Draw input wires
	for(auto eachNode : entities)
	{
		auto wires = eachNode->getWires();

		for(auto eachWire : wires)
		{
			if ( eachWire->getTarget()->getOwner() == eachNode)
				eachWire->getView()->draw();
		}
	}

	auto selectedView = NodeView::GetSelected();
	// Deselection
	if( !isAnyItemHovered && ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
		NodeView::SetSelected(nullptr);

	
	if (selectedView != nullptr)
	{
		// Deletion
		if ( ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
			selectedView->setVisible(false);
		// Arrange 
		else if ( ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
			selectedView->arrangeRecursively();
	}

	// Draft Mouse PAN
	if( ImGui::IsMouseDragging() && ImGui::IsWindowFocused() && !isAnyItemDragged )
	{
		auto drag = ImGui::GetMouseDragDelta();
		for(auto eachNode : entities)
		{
			((NodeView*)eachNode->getComponent("view"))->translate(drag);
		}
		ImGui::ResetMouseDragDelta();
	}

	if (ImGui::IsMouseClicked(1))
		ImGui::OpenPopup("ContainerViewContextualMenu");

	if (ImGui::BeginPopup("ContainerViewContextualMenu"))
	{
		bool updateExpression = ImGui::MenuItem("Update expression");

		ImGui::Separator();
		
		auto container = getOwner()->getAs<Container*>();

		if ( ImGui::BeginMenu("New operation")){
			
			if (ImGui::MenuItem("Add"))
				container->createNodeAdd();

			if (ImGui::MenuItem("Divide"))
				container->createNodeDivide();

			if (ImGui::MenuItem("Multiply"))
				container->createNodeMultiply();

			if (ImGui::MenuItem("Substract"))
				container->createNodeSubstract();

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("New variable"))
			container->createNodeVariable("Variable");

		if (ImGui::MenuItem("New result"))
			container->createNodeResult();


		ImGui::EndPopup();

		if (updateExpression){
			auto result = container->find("");
			auto app   = container->getOwner()->getAs<Application*>();
			if(result && app)
			{
				app->updateCurrentLineText(result->getValue()->getSourceExpression());
			}
		}

	}

	return true;
}