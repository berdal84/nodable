#pragma once

#include <string>
#include <vector>
#include <mpark/variant.hpp>

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;
    class Node;
    class Member;

    namespace Asm // Assembly namespace
    {
        /**
         * Enum to identify each register, we try here to follow the x86_64 DASM reference from
         * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
         */
        enum Register {
            rax = 0, // accumulator
            rdx,      // storage
            COUNT
        };


        /**
         * Enum to identify each function identifier.
         * A function is specified when using "call" instruction.
         */
        enum FctId
        {
            eval_member = 0
        };

        /**
         * Store a single assembly instruction ( line type larg rarg comment )
         */
        struct Instr
        {
            /**
             * Enumerate each possible instruction.
             */
            enum Type
            {
                call,
                mov,
                jmp,
                jne,
                ret,
            };

            // possible types for an argument. // TODO: use a single type, like char[4] for example.
            typedef mpark::variant<
                    void*,
                    Node*,
                    Member*,
                    long,
                    Register,
                    FctId
            > AsmInstrArg;

            Instr(Type _type, long _line): m_type(_type), m_line(_line) {}

            long        m_line;
            Type        m_type;
            AsmInstrArg m_left_h_arg;
            AsmInstrArg m_right_h_arg;
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

            Instr*        push_instr(Instr::Type _type);
            inline size_t size() const { return  m_instructions.size(); }
            inline Instr* operator[](size_t _index) const { return  m_instructions[_index]; }
            long          get_next_pushed_instr_index() const { return m_instructions.size(); }
        private:
            std::vector<Instr*> m_instructions;
        };

        /**
         * Class to check a program node and convert it to Assembly::Code
         */
        class Compiler
        {
        public:
            Compiler():m_output(nullptr){}
            bool          create_assembly_code(const ScopedCodeBlockNode* _program);
            void          append_to_assembly_code(const Node* _node);
            Code*         get_output_assembly();
            static bool   is_program_valid(const ScopedCodeBlockNode* _program);
        private:
            Code* m_output;
        };
    }

    static std::string to_string(Asm::Register);
    static std::string to_string(Asm::FctId);
}
