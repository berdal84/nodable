#pragma once

#include <nodable/core/types.h>
#include <nodable/core/reflection/qword.h>
#include <nodable/core/reflection/type.h>
#include <nodable/core/reflection/enum.h>

namespace Nodable
{
    // forward declarations
    class Scope;

namespace assembly
{
    enum class opcode_t: u8_t // list possible operation codes
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

    R_ENUM(opcode_t)
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
        opcode_t opcode;
        i64_t  offset;
    };

    struct Instruction_mov
    {
        opcode_t opcode;
        qword  dst;
        qword  src;
    };

    struct Instruction_uref
    {
        opcode_t  opcode;
        qword*  qword_ptr;
        const type* qword_type;
    };

    struct Instruction_cmp
    {
        opcode_t opcode;
        qword  left;
        qword  right;
    };

    struct Instruction_push_or_pop
    {
        opcode_t opcode;
        union {
            const VariableNode* var;
            const Scope*        scope;
        };
    };

    struct Instruction_eval
    {
        opcode_t      opcode;
        const Node* node;
    };

    /**
     * Store a single assembly instruction ( line type larg rarg comment )
     */
    struct Instruction
    {
        Instruction(opcode_t _opcode, u8_t _line)
                : opcode(_opcode)
                , line(_line)
        {}

        u8_t line;

        union {
            opcode_t                opcode;
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
