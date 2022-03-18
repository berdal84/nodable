#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <mpark/variant.hpp>

#include <nodable/core/types.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class Member;
    class VariableNode;
    class InstructionNode;

    namespace Asm // Assembly namespace
    {
        /**
         * Enum to identify each register, we try here to follow the x86_64 DASM reference from
         * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
         */
        enum Register {
            rax = 0, // accumulator
            rdx,     // storage
            eip,     // The instruction pointer.
            COUNT
        };

        R_ENUM(Register)
        R_ENUM_VALUE(rax)
        R_ENUM_VALUE(rdx)
        R_ENUM_VALUE(eip)
        R_ENUM_END

        /**
         * Enum to identify each function identifier.
         * A function is specified when using "call" instruction.
         */
        enum class FctId: i64_t
        {
            eval_member,
            eval_node,
            push_stack_frame,
            pop_stack_frame
        };

        R_ENUM(FctId)
        R_ENUM_VALUE(eval_member)
        R_ENUM_VALUE(eval_node)
        R_ENUM_VALUE(push_stack_frame)
        R_ENUM_VALUE(pop_stack_frame)
        R_ENUM_END

        /**
         * Enumerate each possible instruction.
         */
        enum class Instr_t: i8_t
        {
            call,
            mov,
            jmp,
            jne,
            ret,
            cmp /* compare */
        };

        R_ENUM(Instr_t)
        R_ENUM_VALUE(call)
        R_ENUM_VALUE(mov)
        R_ENUM_VALUE(jmp)
        R_ENUM_VALUE(jne)
        R_ENUM_VALUE(ret)
        R_ENUM_VALUE(cmp)
        R_ENUM_END

        /**
         * Store a single assembly instruction ( line type larg rarg comment )
         */
        struct Instr
        {
            Instr(Instr_t _type, long _line)
                : m_type(_type)
                , m_line(_line)
                , m_left_h_arg(0)
                , m_right_h_arg(0)
                , m_comment()
            {}

            i64_t   m_line;
            Instr_t m_type;
            i64_t   m_left_h_arg;
            i64_t   m_right_h_arg;
            std::string m_comment;
            static std::string to_string(const Instr&);
        };

        /**
         * Wraps an Instruction vector, plus some shortcuts.
         */
        class Code
        {
        public:
            Code() = default;
            ~Code();

            Instr*        push_instr(Instr_t _type);
            inline size_t size() const { return  m_instructions.size(); }
            inline Instr* at(size_t _index) const { return  m_instructions.at(_index); }
            long          get_next_pushed_instr_index() const { return m_instructions.size(); }
            const std::vector<Instr*>& get_instructions()const { return m_instructions; }
            void          reset();
        private:
            std::vector<Instr*> m_instructions;
        };

        /**
         * @brief Class to convert a program graph to Assembly::Code
         */
        class Compiler
        {
        public:
            Compiler()= default;
            /** user is owner for Code*, delete if you don't want to use it anymore */
            Code*         compile_program(const Node* _program_graph_root);
            bool          is_program_valid(const Node* _program_graph_root);
        private:
            void          compile(const Node* _node);
            void          compile(const Member *_member);
            Code*         m_output;
        };
    }
}
