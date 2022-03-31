#pragma once

#include <nodable/core/types.h>
#include <nodable/core/assembly/QWord.h>

namespace Nodable
{
    // forward declarations
    class Scope;

namespace assembly
{
    enum class opcode: u8_t // list possible operation codes
    {
        cmp, /* compare */
        eval_node,
        jmp,
        jne,
        mov,
        deref_ptr,
        pop_stack_frame,
        pop_var,
        push_stack_frame,
        push_var,
        ret
    };

    R_ENUM(opcode)
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

    struct Instruction_jmp // Jump relative to current line (+/- offset)
    {
        opcode type;
        i64_t  offset;
    };

    struct Instruction_mov
    {
        opcode type;
        QWord  dst;
        QWord  src;
    };

    struct Instruction_uref
    {
        opcode  type;
        QWord*  qword_ptr;
        R::Type qword_type;
    };

    struct Instruction_cmp
    {
        opcode type;
        QWord  left;
        QWord  right;
    };

    struct Instruction_push_or_pop
    {
        opcode type;
        union {
            const VariableNode* var;
            const Scope*        scope;
        };
    };

    struct Instruction_eval
    {
        opcode      type;
        const Node* node;
    };

    /**
     * Store a single assembly instruction ( line type larg rarg comment )
     */
    struct Instruction
    {
        Instruction(opcode _type, u8_t _line)
                : type(_type)
                , line(_line)
        {}

        u8_t line;

        union {
            opcode                  type;
            Instruction_mov         mov;
            Instruction_uref        uref;
            Instruction_jmp         jmp;
            Instruction_cmp         cmp;
            Instruction_push_or_pop push;
            Instruction_push_or_pop pop;
            Instruction_eval        eval;
        };
        std::string m_comment;
        static std::string to_string(const Instruction&);
    };
} // namespace Asm
} // namespace Nodable
