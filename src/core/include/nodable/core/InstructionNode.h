#pragma once

#include <string>
#include <nodable/core/memory.h>

#include <nodable/core/types.h> // forward declarations and common stuff
#include <nodable/core/Node.h> // base class
#include <nodable/core/Member.h>

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

        [[nodiscard]] inline Member* get_root_node_member()const { return m_props.get(k_value_member_name); }
        [[nodiscard]] inline s_ptr<Token> end_of_instr_token()const { return m_end_of_instr_token; }
                      inline void    end_of_instr_token(s_ptr<Token> token) { m_end_of_instr_token = token; }

    private:
        s_ptr<Token> m_end_of_instr_token = nullptr;
        REFLECT_DERIVED_CLASS(Node)
    };
}
