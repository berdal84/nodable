#pragma once

#include <nodable/core/types.h>
#include "Instruction.h"

namespace ndbl
{
namespace assembly
{
    /**
     * Instruction container + some meta data.
     */
    class Code
    {
        using Instructions = std::vector<Instruction*>;
        struct MetaData
        {
            Node* root_node;
        };
    public:
        Code(Node* _root): m_meta_data({_root}) {};
        ~Code();

        Instruction*               push_instr(opcode_t _type);
        inline size_t              size() const { return  m_instructions.size(); }
        inline Instruction*        get_instruction_at(size_t _index) const { return  m_instructions.at(_index); }
        size_t                     get_next_index() const { return m_instructions.size(); }
        const Instructions&        get_instructions()const { return m_instructions; }
        const MetaData&            get_meta_data()const { return m_meta_data; }
        static std::string         to_string(const Code*);
    private:
        MetaData     m_meta_data;
        Instructions m_instructions;
    };
} // namespace Asm
} // namespace ndbl
