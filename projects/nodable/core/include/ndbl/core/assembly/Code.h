#pragma once

#include "fw/core/types.h"
#include <ndbl/core/assembly/Instruction.h>

namespace ndbl
{
namespace assembly
{
    /**
     * @class Instructions container with some extra meta data
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

        Instruction*               push_instr(Instruction_t _type);                                                    // Push back a new instruction to the code.
        inline size_t              size() const { return  m_instructions.size(); }                                // Get instruction count.
        inline Instruction*        get_instruction_at(size_t _index) const { return  m_instructions.at(_index); } // Get the instruction at a given zero-based index.
        size_t                     get_next_index() const { return m_instructions.size(); }                       // Get the next index available.
        const Instructions&        get_instructions()const { return m_instructions; }                             // Get the instructions.
        const MetaData&            get_meta_data()const { return m_meta_data; }                                   // Get the code metadata (cf. MetaData).
        static std::string         to_string(const Code*);                                                        // Convert all the instructions to a string.
    private:
        MetaData     m_meta_data;
        Instructions m_instructions;
    };
} // namespace assembly
} // namespace nodable
