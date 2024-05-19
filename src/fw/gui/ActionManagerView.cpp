#include "ActionManagerView.h"
#include "ActionManager.h"
#include "ImGuiEx.h"

using namespace fw;

void ActionManagerView::draw(ActionManager* manager)
{
    if ( ImGui::BeginTable("Actions", 2) )
    {
        ImGui::TableSetupColumn("Action");
        ImGui::TableSetupColumn("Shortcut");
        ImGui::TableHeadersRow();

        for( auto& action : manager->get_actions())
        {
            ImGui::PushID(action);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if( ImGui::SmallButton("trigger") )
            {
                action->trigger();
            }
            ImGui::SameLine();
            ImGui::Text("%s", action->label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", action->shortcut.to_string().c_str()); // TODO: handle shortcut edition
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

