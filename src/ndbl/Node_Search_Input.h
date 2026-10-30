#pragma once
#include "bdc/Array.hpp"
#include "Action.h"

namespace ndbl
{
    // forward declarations
    struct Node_Slot_View;

    struct Node_Search_Input
    {
        bool                            must_be_reset_flag              = true;
        char                            search_input_value[255]         = "\0"; // The search input entered by the user.
        bdc::Resizable_Array<Action>    items;                                  // All the available items
        bdc::Resizable_Array<u32_t>     items_with_compatible_signature;        // Only the indices of the items having a compatible signature (with the slot dragged)
        bdc::Resizable_Array<u32_t>     items_matching_search;                  // Only the indices of the items having a compatible signature AND matching the search_input.
    };

    void    nodesearchinput_init(Node_Search_Input&);
    void    nodesearchinput_release(Node_Search_Input&);
    Action* nodesearchinput_draw(Node_Search_Input&, Node_Slot_View* dragged_slotview, size_t result_max_count ); // Return the triggered action, user has to deal with the Action.
}