#include "Node_Search_Input.h"
#include "tools/gui/Action.h"
#include "imgui.h"

namespace ndbl
{
    // private
    void _nodeview_contextmenu_update_cache_based_on_signature(Node_Search_Input*, Node_Slot_View* dragged_slot);;
    void _nodeview_contextmenu_update_cache_based_on_user_input(Node_Search_Input*, Node_Slot_View* dragged_slot, size_t limit );
}

void ndbl::_nodeview_contextmenu_update_cache_based_on_signature(Node_Search_Input* context_menu, Node_Slot_View* dragged_slot)
{
    context_menu->items_with_compatible_signature.clear();

    // 1) When NO slot is dragged
    //---------------------------

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        context_menu->items_with_compatible_signature = context_menu->items;
        return;
    }

    // 2) When a slot is dragged
    //--------------------------

    for (Action_CreateNode* action : context_menu->items )
    {
        const Type_Descriptor* dragged_property_type = dragged_slot->property_type();

        switch ( action->event_data.node_type )
        {
            case Create_Node_Type_BLOCK_CONDITION:
            case Create_Node_Type_BLOCK_FOR_LOOP:
            case Create_Node_Type_BLOCK_WHILE_LOOP:
            case Create_Node_Type_BLOCK_SCOPE:
            case Create_Node_Type_ROOT:
                // Blocks are only for code flow slots
                if ( !dragged_slot->allows(Node_Slot::Flag_TYPE_FLOW) )
                    continue;
                break;

            default:

                if ( dragged_slot->allows(Node_Slot::Flag_TYPE_FLOW))
                {
                    // we can connect anything to a code flow slot
                }
                else if ( dragged_slot->allows(Node_Slot::Flag_INPUT) && dragged_slot->property_type()->is<Node*>() )
                {
                    // we can connect anything to a Node ref input
                }
                else if ( action->event_data.node_signature )
                {
                    // discard incompatible signatures

                    if ( dragged_slot->allows(Node_Slot::Flag_ORDER_1ST ) &&
                        !action->event_data.node_signature->has_arg_with_type(dragged_property_type)
                            )
                        continue;

                    if ( !action->event_data.node_signature->return_type()->equals(dragged_property_type) )
                        continue;

                }
        }
        context_menu->items_with_compatible_signature.push_back( action );
    }
}

template<typename charT>
struct CaseInsensitiveEqual
{
    const std::locale& locale;
    bool operator()(charT ch1, charT ch2)
    {
        return std::toupper(ch1, locale) == std::toupper(ch2, locale);
    }
};

template<typename T>
bool CaseInsensitiveFind(const T& str1, const T& str2, const std::locale& loc = std::locale())
{
    return std::search(str1.begin(), str1.end(),
                       str2.begin(), str2.end(),
                       CaseInsensitiveEqual<typename T::value_type>{loc}) != str1.end();
}

void ndbl::_nodeview_contextmenu_update_cache_based_on_user_input(Node_Search_Input* context_menu, Node_Slot_View* _dragged_slot, size_t _limit )
{
    std::string search{context_menu->search_input_value}; // FindCaseInsensitive takes a std::string
    context_menu->items_matching_search.clear();
    for ( auto& menu_item : context_menu->items_with_compatible_signature )
    {
        if( !CaseInsensitiveFind(menu_item->label, search) )
            continue;

        context_menu->items_matching_search.push_back(menu_item);
        if ( context_menu->items_matching_search.size() == _limit )
            break;
    }
}

ndbl::Action_CreateNode* ndbl::nodeview_contextmenu_draw_search_input(Node_Search_Input* context_menu, Node_Slot_View* dragged_slot, size_t _result_max_count )
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
            auto action = context_menu->items_matching_search.front();
            if ( ImGui::SmallButton( action->label.c_str()) || ImGui::IsKeyDown( ImGuiKey_Enter ) )
            {
                return action;
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
                auto* action = *it;

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