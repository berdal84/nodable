#include "Action_Manager_View.h"
#include "ImGuiEx.h"

void tools::action_manager_view_draw(Action_Manager* manager)
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

