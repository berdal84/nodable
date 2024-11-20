#pragma once

#include "tools/core/memory/memory.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

namespace ndbl
{
    // forward declarations
    class Scope;
    class VariableNode;
    class Node;

    // list possible instruction's operation types
    typedef u8_t OpCode;
    enum OpCode_ : u8_t
    {
        OpCode_cmp,              // comparison.
        OpCode_call,        // node evaluation.
        OpCode_jmp,              // unconditional jump.
        OpCode_jne,              // conditional jump.
        OpCode_mov,              // move or copy memory.
        OpCode_deref_qword,      // qword ptr de-referencing.
        OpCode_pop_stack_frame,  // ends the current stack frame.
        OpCode_pop_var,          // pop a variable from the stack.
        OpCode_push_stack_frame, // starts a new stack frame within the current.
        OpCode_push_var,         // push a variable to the stack.
        OpCode_ret               // return value.
    };

    REFLECT_ENUM(OpCode)
    (
        REFLECT_ENUM_V(OpCode_mov)
        REFLECT_ENUM_V(OpCode_deref_qword )
        REFLECT_ENUM_V(OpCode_call)
        REFLECT_ENUM_V(OpCode_push_var)
        REFLECT_ENUM_V(OpCode_pop_var)
        REFLECT_ENUM_V(OpCode_push_stack_frame)
        REFLECT_ENUM_V(OpCode_pop_stack_frame)
        REFLECT_ENUM_V(OpCode_jmp)
        REFLECT_ENUM_V(OpCode_jne)
        REFLECT_ENUM_V(OpCode_ret)
        REFLECT_ENUM_V(OpCode_cmp)
    )

    // Unconditional jump
    struct Instruction_jmp
    {
        OpCode opcode;
        i64_t         offset;    // offset relative to the current instruction pointer.
    };

    struct Instruction_mov
    {
        OpCode opcode;
        tools::qword  dst;       // source memory
        tools::qword  src;       // destination memory
    };

    // Un-reference a pointer to a given type
    struct Instruction_uref
    {
        OpCode                 opcode;
        const tools::qword*    ptr;
        const tools::TypeDescriptor* type; // pointed data's type.
    };

    // Compare two operands (test if equals)
    struct Instruction_cmp
    {
        using qword = tools::qword;

        OpCode opcode;
        qword  left;     // the left operand.
        qword  right;    // the right operand.
    };

    // Push or pop to/from the stack.
    struct Instruction_push_or_pop
    {
        OpCode opcode;
        union {
            VariableNode* var;     // a variable to push/pop.
            const Scope*  scope;   // a scope to push/pop.
        };
    };

    // Evaluates a given node
    struct Instruction_eval
    {
        OpCode                   opcode;
        const tools::IInvokable* invokable;
    };

    /**
     * Store a single assembly instruction.
     * Each instruction looks like:
     * @code
     * line_nb type left-arg_at right-arg_at comment
     */
    struct Instruction
    {
        Instruction(OpCode _opcode, size_t _line)
            : opcode(_opcode)
            , line(_line)
        {}

        size_t      line;                                   // line index (zero-based position in the code)

        // all the possible instructions
        union {
            OpCode                  opcode;                 // simple operation
            Instruction_mov         mov;                    // copy data
            Instruction_uref        uref;                   // un-reference
            Instruction_jmp         jmp;                    // jump to
            Instruction_cmp         cmp;                    // compare
            Instruction_push_or_pop push;                   // push to stack
            Instruction_push_or_pop pop;                    // pop from stack
            Instruction_eval        call;                   // evaluates
        };
        std::string m_comment;                              // optional comment.
        static std::string to_string(const Instruction&);   // Convert the instruction to a nice looking string.
    };

} // namespace ndbl
