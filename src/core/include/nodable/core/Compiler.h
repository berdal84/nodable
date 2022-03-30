#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include <nodable/core/types.h>
#include <nodable/core/reflection/R.h>
#include <nodable/core/Variant.h>
#include "GraphNode.h"

namespace Nodable
{
    // forward declarations
    class Node;
    class Scope;
    class Member;
    class VariableNode;
    class InstructionNode;
    class ConditionalStructNode;
    class ForLoopNode;

    namespace Asm // Assembly namespace
    {
        /**
         * Enum to identify each register, we try here to follow the x86_64 DASM reference from
         * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
         */
        enum Register : size_t {
            undefined,
            rax,     // accumulator
            rdx,     // storage
            eip,     // The instruction pointer.
            COUNT
        };

        R_ENUM(Register)
        R_ENUM_VALUE(undefined)
        R_ENUM_VALUE(rax)
        R_ENUM_VALUE(rdx)
        R_ENUM_VALUE(eip)
        R_ENUM_END

        /**
         * Enumerate each possible instruction.
         */
        enum class Instr_t: u8_t
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

        R_ENUM(Instr_t)
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

        /**
         * Simple structure to store (data, type) couple.
         */
        struct MemSpace
        {
            union {
                bool      b;
                double    d;
                u64_t     u64;
                void*     ptr;
                Register  r;
            };

            MemSpace()
            {
                reset();
            }

            MemSpace(bool _value)
            {
                b = _value;
            }

            MemSpace(double _value)
            {
                d = _value;
            }

            MemSpace(Register _value)
            {
                r = _value;
            }

            MemSpace(u64_t _value)
            {
                u64 = _value;
            }

            template<typename T>
            MemSpace(const T* _value)
            {
                ptr = (void*)_value;
            }

            explicit operator bool() { return b; }
            explicit operator int() { return (int)d; }
            explicit operator double() { return d; }
            explicit operator u64_t() { return u64; }
            explicit operator char*() { return (char*)ptr; }
            explicit operator void*() { return ptr; }
            explicit operator std::string() { return std::string((char*)ptr); }

            void reset()
            {
                memset(this, 0, sizeof(*this));
            }

            std::string to_string()const { return MemSpace::to_string(*this); }
            static std::string to_string(const MemSpace&);
        };
        static_assert(sizeof(MemSpace) == 8);



        struct Instr_Jmp // Jump relative to current line (+/- offset)
        {
            Instr_t type;
            i64_t   offset;
        };

        struct Instr_Mov
        {
            Instr_t  type;
            MemSpace dst;
            MemSpace src;
        };

        struct Instr_Deref
        {
            Instr_t      type;
            VariantData* ptr;
            R::Type      ptr_t;
        };

        struct Instr_Cmp
        {
            Instr_t  type;
            MemSpace left;
            MemSpace right;
        };

        struct Instr_PushPop
        {
            Instr_t type;
            union {
                const VariableNode* var;
                const Scope*        scope;
            };
        };

        struct Instr_Eval
        {
            Instr_t     type;
            const Node* node;
        };

        /**
         * Store a single assembly instruction ( line type larg rarg comment )
         */
        struct Instr
        {
            Instr(Instr_t _type, u64_t _line)
                : type(_type)
                , line(_line)
            {}

            u64_t       line;

            union {
                Instr_t           type;
                Instr_Mov         mov;
                Instr_Deref       deref;
                Instr_Jmp         jmp;
                Instr_Cmp         cmp;
                Instr_PushPop     push;
                Instr_PushPop     pop;
                Instr_Eval        eval;
            };
            std::string m_comment;
            static std::string to_string(const Instr&);
        };

        /**
         * Wraps an Instruction vector, plus some shortcuts.
         */
        class Code
        {
        public:
            struct MetaData {
                Node* root_node;
            };

            Code(Node* _root): m_meta_data({_root}) {};
            ~Code();

            Instr*                     push_instr(Instr_t _type);
            inline size_t              size() const { return  m_instructions.size(); }
            inline Instr*              get_instruction_at(size_t _index) const { return  m_instructions.at(_index); }
            size_t                     get_next_index() const { return m_instructions.size(); }
            const std::vector<Instr*>& get_instructions()const { return m_instructions; }
            const MetaData&            get_meta_data()const { return m_meta_data; }
            static std::string         to_string(const Code*);
        private:
            MetaData            m_meta_data;
            std::vector<Instr*> m_instructions;
        };

        /**
         * @brief Class to convert a program graph to Assembly::Code
         */
        class Compiler
        {
        public:
            Compiler()= default;
            std::unique_ptr<const Code> compile_syntax_tree(const GraphNode*);
        private:
            bool is_syntax_tree_valid(const GraphNode*);
            void compile(const Node*);
            void compile(const Member*);
            void compile(const Scope*, bool _insert_fake_return = false);
            void compile(const InstructionNode*);
            void compile(const ForLoopNode*);
            void compile(const ConditionalStructNode*);
            void compile_as_condition(const InstructionNode *_instr_node);

            std::unique_ptr<Code> m_temp_code;
        };
    }
}
