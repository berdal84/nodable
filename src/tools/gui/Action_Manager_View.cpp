#include "Action_Manager_View.h"
#include "core/Event_Manager.h"
#include "tools/gui/Action_Manager.h"
#include "imgui.h"

void tools::action_manager_view_draw(Action_Manager* manager)
{
    if ( ImGui::BeginTable("Actions", 2) )
    {
        ImGui::TableSetupColumn("Action");
        ImGui::TableSetupColumn("Shortcut");
        ImGui::TableHeadersRow();

        for( auto& action : manager->actions)
        {
            ImGui::PushID(&action);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if( ImGui::SmallButton("trigger") )
            {
                event_manager_push_event( action.event);
            }
            ImGui::SameLine();
            ImGui::Text("%s", action.label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", action.shortcut.to_string().c_str()); // TODO: handle shortcut edition
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

