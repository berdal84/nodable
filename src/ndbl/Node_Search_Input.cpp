#include "Node_Search_Input.h"

#include <cstddef>
#include <imgui/imgui.h>

#include "Node.h"
#include "Event.h"
#include "Node_Slot_View.h"
#include "Action.h"

namespace ndbl
{
// private
void _nodesearchinput_update_cache_based_on_signature(Node_Search_Input&, Node_Slot_View* dragged_slot);;
void _nodesearchinput_update_cache_based_on_user_input(Node_Search_Input&, Node_Slot_View* dragged_slot, size_t limit );

void nodesearchinput_init(Node_Search_Input& input)
{
    array_init(input.items);
    array_init(input.items_matching_search);
    array_init(input.items_with_compatible_signature);
}

void nodesearchinput_release(Node_Search_Input& input)
{
    array_release(input.items);
    array_release(input.items_matching_search);
    array_release(input.items_with_compatible_signature);
}

void _nodesearchinput_update_cache_based_on_signature(Node_Search_Input& input, Node_Slot_View* dragged_slot)
{
    array_clear(input.items_with_compatible_signature);

    // 1) When NO slot is dragged
    //---------------------------

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        array_resize( input.items_with_compatible_signature, input.items.size );
        for(size_t i = 0; i < input.items_with_compatible_signature.size; i++)
            input.items_with_compatible_signature[i] = i;

        return;
    }

    // 2) When a slot is dragged
    //--------------------------

    for (size_t i = 0; i < input.items.size; ++i )
    {   
        Action& action = input.items[i];
        Event_Data__Create_Node* event_data = static_cast<Event_Data__Create_Node*>(action.event.data1);

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

        array_append(input.items_with_compatible_signature, i);
    }
}

void _nodesearchinput_update_cache_based_on_user_input(Node_Search_Input& input, Node_Slot_View* _dragged_slot, size_t _limit )
{
    bdc::String search{input.search_input_value}; // FindCaseInsensitive takes a bdc::String
    array_clear(input.items_matching_search);
    for ( size_t i : input.items_with_compatible_signature )
    {
        bdc::String found = bdc::string_case_insensitive_find( input.items[i].label, search);
        if( !found.empty() )
        {
            continue;
        }

        array_append(input.items_matching_search, i);
        if ( input.items_matching_search.size == _limit )
        {
            break;
        }
    }
}

Action* nodesearchinput_draw(Node_Search_Input& input, Node_Slot_View* dragged_slot, size_t _result_max_count )
{
    if ( input.must_be_reset_flag )
    {
        input.search_input_value[0] = '\0';
        array_clear(input.items_matching_search);
        array_clear(input.items_with_compatible_signature);

        ImGui::SetKeyboardFocusHere();

        //
        _nodesearchinput_update_cache_based_on_signature(input, dragged_slot);

        // Initial search
        _nodesearchinput_update_cache_based_on_user_input(input, dragged_slot, 100 );

        // Ensure we reset once
        input.must_be_reset_flag = false;
    }

    // Draw search input and update_cache_based_on_user_input on input change
    ImGui::BeginGroup();
    ImGui::Text("Create Node:");
    ImGui::SameLine();
    if ( ImGui::InputText("###Search", input.search_input_value, 255, ImGuiInputTextFlags_EscapeClearsAll ))
    {
        _nodesearchinput_update_cache_based_on_user_input(input, dragged_slot, 100 );
    }
    ImGui::EndGroup();

    if ( input.items_matching_search.size )
    {
        // When a single item is filtered, pressing enter will press the item's button.
        if ( input.items_matching_search.size == 1)
        {
            auto action_index = input.items_matching_search[0];
            if ( ImGui::SmallButton( input.items[action_index].label.c_str()) || ImGui::IsKeyDown( ImGuiKey_Enter ) )
            {
                return &input.items[action_index];
            }
        }
        else
        {
            size_t more = input.items_matching_search.size > _result_max_count ? input.items_matching_search.size : 0;
            if ( more )
            {
                ImGui::Text("Found %u result(s)", input.items_matching_search.size );
            }
            // Otherwise, user has to move with arrow keys and press enter to trigger the highlighted button.
            auto it = input.items_matching_search.begin();
            while( it != input.items_matching_search.end() && std::distance(input.items_matching_search.begin(), it) != _result_max_count)
            {
                Action* action = &input.items[*it];

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

} // namespace ndbl