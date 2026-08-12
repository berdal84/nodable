#pragma once

#include "tools/core/reflection/Type_Descriptor.h"
#include "ndbl/core/Token.h"
#include <string>

namespace ndbl
{
    // forward declarations
    struct Node;

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

        std::string                     name;
        Token                           token;
        Node*                           node    = nullptr; // owner
        const tools::Type_Descriptor*   type    = nullptr; // n.b. use setter
        Flags                           flags   = Flag_NONE;
    };

    void property_set_type  (Node_Property*, const tools::Type_Descriptor* /* new_type */ );
    void property_init      (Node_Property*, Node* /* owner */, const tools::Type_Descriptor*, Node_Property::Flags, const char* _name); // must be called once before use
    void property_digest    (Node_Property*, Node_Property* /* other */);
}