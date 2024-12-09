#pragma once

#include <vector>
#include "tools/core/types.h"
#include "Instruction.h"

namespace ndbl
{
    // forward declarations
    class Graph;

    /**
     * @class Instructions container with some extra meta data
     */
    class Code
    {
        typedef std::vector<Instruction*> Instructions; // TODO: switch to a std::vector<Instruction> to iterate faster on this data.
        struct MetaData
        {
            const Graph* graph;
        };
    public:
        Code(const Graph* _root);
        ~Code();

        Instruction*               push_instr(OpCode);                                                            // Push back a new instruction to the code
        size_t              size() const { return  m_instructions.size(); }                                // Get instruction count.
        Instruction*        get_instruction_at(size_t _index) const { return  m_instructions.at(_index); } // Get the instruction at a given zero-based index (be careful, a push_instr() call might invalidate this ptr).
        size_t                     get_next_index() const { return m_instructions.size(); }                       // Get the next index available.
        const Instructions&        get_instructions()const { return m_instructions; }                             // Get the instructions.
        const MetaData&            get_meta_data()const { return m_meta_data; }                                   // Get the code metadata (cf. MetaData).
        static std::string         to_string(const Code*);                                                        // Convert all the instructions to a string.
    private:
        MetaData     m_meta_data;
        Instructions m_instructions;
    };
} // namespace ndbl
