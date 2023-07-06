#pragma once

#include <string>
#include <memory>

#include "fw/core/types.h"
#include "core/Node.h" // base class
#include "core/Property.h"


namespace ndbl
{
    // forward declarations
    struct Token;

    /*
        The role of this class is to symbolize an instruction.
        The result of the instruction is value()
    */
    class InstructionNode : public Node
    {
    public:
        explicit InstructionNode();
        ~InstructionNode()= default;
        Token token_end;
        inline Property * get_root_node_property()const { return m_props.get(k_value_property_name); }
    private:
        REFLECT_DERIVED_CLASS(Node)
    };
}
