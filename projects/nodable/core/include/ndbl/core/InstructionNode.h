#pragma once

#include <string>
#include <memory>
#include "fw/core/types.h"

#include <ndbl/core/Node.h>// base class
#include <ndbl/core/Property.h>


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

        [[nodiscard]] inline Property * get_root_node_property()const { return m_props.get(k_value_property_name); }
        [[nodiscard]] inline std::shared_ptr<Token> end_of_instr_token()const { return m_end_of_instr_token; }
                      inline void    end_of_instr_token(std::shared_ptr<Token> token) { m_end_of_instr_token = token; }

    private:
        std::shared_ptr<Token> m_end_of_instr_token = nullptr;
        REFLECT_DERIVED_CLASS(Node)
    };
}
