#include "Action_Manager_View.h"

#include <imgui.h>

#include "Event_Manager.h"
#include "Action_Manager.h"

namespace ndbl
{

void action_manager_view_draw(Action_Manager* manager)
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
            ImGui::Text(STRING_FMT, STRING_FMT_ARGS(action.label));
            ImGui::TableNextColumn();
            ImGui::Text(STRING_FMT, STRING_FMT_ARGS(shortcut_to_tstring(action.shortcut))); // Note: handle shortcut edition?
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
}

} // namespace ndbl
