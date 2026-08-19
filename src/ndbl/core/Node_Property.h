#pragma once

#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "tools/core/reflection/Type_Descriptor.h"
#include "ndbl/core/Token.h"

namespace ndbl
{
    // forward declarations
    struct Node;
    struct Node_Slot;

    // Property wraps a Token including extra inFormation such as: name, owner (Node), and some flags.
	struct Node_Property
    {
        typedef int Flags;
        enum Flag_
        {
            Flag_NONE            = 0,
            Flag_IS_REF          = 1 << 0,
            Flag_IS_NODE_VALUE   = 1 << 1,
            Flag_ALL             = ~Flag_NONE,
        };

        bdc::String                      name;
        Token                            token;
        Node*                            node;
        const tools::Type_Descriptor*    type;
        Flags                            flags;
        bdc::Resizable_Array<Node_Slot*> slots;
    };

    void property_init      (Node_Property*, Node* /* owner */, const tools::Type_Descriptor*, Node_Property::Flags, const bdc::String _name); // must be called once before use
    void property_release   (Node_Property*);
    void property_set_type  (Node_Property*, const tools::Type_Descriptor* /* new_type */ );
    void property_digest    (Node_Property*, Node_Property* /* other */);
}