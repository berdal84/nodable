#pragma once

#include "fw/core/types.h"
#include "fw/core/reflection/reflection"

namespace ndbl
{
    // forward declarations
    class Scope;
    class VariableNode;
    class Node;

namespace assembly
{
    // list possible instruction's operation types
    enum class Instruction_t : u8_t
    {
        cmp,                   // comparison.
        eval_node,             // node evaluation.
        jmp,                   // unconditional jump.
        jne,                   // conditional jump.
        mov,                   // move or copy memory.
        deref_ptr,             // pointer de-referencing.
        pop_stack_frame,       // ends the current stack frame.
        pop_var,               // pop a variable from the stack.
        push_stack_frame,      // starts a new stack frame within the current.
        push_var,              // push a variable to the stack.
        ret                    // return value.
    };

    R_ENUM(Instruction_t)
    R_ENUM_VALUE(mov)
    R_ENUM_VALUE(deref_ptr)
    R_ENUM_VALUE(eval_node)
    R_ENUM_VALUE(push_var)
    R_ENUM_VALUE(pop_var)
    R_ENUM_VALUE(push_stack_frame)
    R_ENUM_VALUE(pop_stack_frame)
    R_ENUM_VALUE(jmp)
    R_ENUM_VALUE(jne)
    R_ENUM_VALUE(ret)
    R_ENUM_VALUE(cmp)
    R_ENUM_END

    // Unconditional jump
    struct Instruction_jmp
    {
        Instruction_t opcode;
        i64_t         offset;    // offset relative to the current instruction pointer.
    };

    struct Instruction_mov
    {
        Instruction_t opcode;
        fw::qword     dst;       // source memory
        fw::qword     src;       // destination memory
    };

    // Un-reference a pointer to a given type
    struct Instruction_uref
    {
        Instruction_t   opcode;
        fw::qword*      qword_ptr;   // pointer to the data.
        const fw::type* qword_type;  // pointed data's type.
    };

    // Compare two operands (test if equals)
    struct Instruction_cmp
    {
        Instruction_t opcode;
        fw::qword     left;     // the left operand.
        fw::qword     right;    // the right operand.
    };

    // Push or pop to/from the stack.
    struct Instruction_push_or_pop
    {
        Instruction_t opcode;            // Instruction_t::push_xxx or Instruction_t::pop_xxx.
        union {
            const VariableNode* var;     // a variable to push/pop.
            const Scope*        scope;   // a scope to push/pop.
        };
    };

    // Evaluates a given node
    struct Instruction_eval
    {
        Instruction_t opcode;
        const Node*   node;             // The node to evaluate.
    };

    /**
     * Store a single assembly instruction.
     * Each instruction looks like:
     * @code
     * line_nb type left-arg right-arg comment
     */
    struct Instruction
    {
        Instruction(Instruction_t _opcode, u64_t _line)
                : opcode(_opcode)
                , line(_line)
        {}

        u64_t line;                                          // line index (zero-based position in the code)

        // all the possible instructions
        union {
            Instruction_t           opcode;                 // simple operation
            Instruction_mov         mov;                    // copy data
            Instruction_uref        uref;                   // un-reference
            Instruction_jmp         jmp;                    // jump to
            Instruction_cmp         cmp;                    // compare
            Instruction_push_or_pop push;                   // push to stack
            Instruction_push_or_pop pop;                    // pop from stack
            Instruction_eval        eval;                   // evaluates
        };
        std::string m_comment;                              // optional comment.
        static std::string to_string(const Instruction&);   // Convert the instruction to a nice looking string.
    };
} // namespace assembly
} // namespace nodable
