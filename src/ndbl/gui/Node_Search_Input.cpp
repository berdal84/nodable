#include "Node_Search_Input.h"
#include "core/Node.h"
#include "gui/Event.h"
#include "gui/Node_Slot_View.h"
#include "tools/gui/Action.h"
#include "imgui.h"
#include <cstddef>

namespace ndbl
{
    // private
    void _nodeview_contextmenu_update_cache_based_on_signature(Node_Search_Input*, Node_Slot_View* dragged_slot);;
    void _nodeview_contextmenu_update_cache_based_on_user_input(Node_Search_Input*, Node_Slot_View* dragged_slot, size_t limit );
}

void ndbl::_nodeview_contextmenu_update_cache_based_on_signature(Node_Search_Input* context_menu, Node_Slot_View* dragged_slot)
{
    using namespace tools;

    context_menu->items_with_compatible_signature.clear();

    // 1) When NO slot is dragged
    //---------------------------

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        context_menu->items_with_compatible_signature.resize( context_menu->items.size() );
        for(size_t i = 0; i < context_menu->items_with_compatible_signature.size(); i++)
            context_menu->items_with_compatible_signature[i] = i;

        return;
    }

    // 2) When a slot is dragged
    //--------------------------

    for (size_t i = 0; i < context_menu->items.size(); ++i )
    {   
        Action& action = context_menu->items[i];
        Event_Data__Create_Node* event_data = static_cast<Event_Data__Create_Node*>(action.event.user.data1);

        // Connect FLOW ?
        if ( dragged_slot->allows(Node_Slot::Flag_TYPE_FLOW) )
        {
            continue;
        }
        
        // Dragging a Node* slot?
        if ( dragged_slot->allows(Node_Slot::Flag_INPUT) && !dragged_slot->property_type()->is<Node*>() )
        {
            continue;
        }

        // Compatible with any argument type?
        if ( dragged_slot->allows(Node_Slot::Flag_ORDER_1ST ) &&
            !event_data->function_type->function_has_arg_with_type( dragged_slot->property_type() )
                )
            continue;

        // Compatible return type?
        if ( !type_equals( event_data->function_type->function.return_type, dragged_slot->property_type() ) )
            continue;

        context_menu->items_with_compatible_signature.push_back(i);
    }
}

void ndbl::_nodeview_contextmenu_update_cache_based_on_user_input(Node_Search_Input* context_menu, Node_Slot_View* _dragged_slot, size_t _limit )
{
    bdc::String search{context_menu->search_input_value}; // FindCaseInsensitive takes a bdc::String
    context_menu->items_matching_search.clear();
    for ( size_t i : context_menu->items_with_compatible_signature )
    {
        bdc::String found = bdc::string_case_insensitive_find( context_menu->items[i].label, search);
        if( !found.empty() )
        {
            continue;
        }

        context_menu->items_matching_search.push_back(i);
        if ( context_menu->items_matching_search.size() == _limit )
        {
            break;
        }
    }
}

tools::Action* ndbl::nodeview_contextmenu_draw_search_input(Node_Search_Input* context_menu, Node_Slot_View* dragged_slot, size_t _result_max_count )
{
    if ( context_menu->must_be_reset_flag )
    {
        context_menu->search_input_value[0] = '\0';
        context_menu->items_matching_search.clear();
        context_menu->items_with_compatible_signature.clear();

        ImGui::SetKeyboardFocusHere();

        //
        _nodeview_contextmenu_update_cache_based_on_signature(context_menu, dragged_slot);

        // Initial search
        _nodeview_contextmenu_update_cache_based_on_user_input(context_menu, dragged_slot, 100 );

        // Ensure we reset once
        context_menu->must_be_reset_flag = false;
    }

    // Draw search input and update_cache_based_on_user_input on input change
    ImGui::BeginGroup();
    ImGui::Text("Create Node:");
    ImGui::SameLine();
    if ( ImGui::InputText("###Search", context_menu->search_input_value, 255, ImGuiInputTextFlags_EscapeClearsAll ))
    {
        _nodeview_contextmenu_update_cache_based_on_user_input(context_menu, dragged_slot, 100 );
    }
    ImGui::EndGroup();

    if ( !context_menu->items_matching_search.empty() )
    {
        // When a single item is filtered, pressing enter will press the item's button.
        if ( context_menu->items_matching_search.size() == 1)
        {
            auto action_index = context_menu->items_matching_search.front();
            if ( ImGui::SmallButton( context_menu->items[action_index].label.c_str()) || ImGui::IsKeyDown( ImGuiKey_Enter ) )
            {
                return &context_menu->items[action_index];
            }
        }
        else
        {
            size_t more = context_menu->items_matching_search.size() > _result_max_count ? context_menu->items_matching_search.size() : 0;
            if ( more )
            {
                ImGui::Text("Found %zu result(s)", context_menu->items_matching_search.size() );
            }
            // Otherwise, user has to move with arrow keys and press enter to trigger the highlighted button.
            auto it = context_menu->items_matching_search.begin();
            while( it != context_menu->items_matching_search.end() && std::distance(context_menu->items_matching_search.begin(), it) != _result_max_count)
            {
                tools::Action* action = &context_menu->items[*it];

                // User can click on the button...
                ImGui::Button( action->label.c_str());
                if( ImGui::IsItemClicked(0) )
                    return action;

                // ...or press enter if this item is the first
                if ( ImGui::IsKeyDown( ImGuiKey_Enter ) && ImGui::IsItemFocused() )
                    return action;

                it++;
            }
            if ( more )
            {
                ImGui::Text(".. %zu more ..", more );
            }
        }
    }
    else
    {
        ImGui::Text("No matches...");
    }

    return nullptr;
}