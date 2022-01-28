#pragma once

#include <string>
#include <vector>
#include <mpark/variant.hpp>

namespace Nodable
{
    // forward declarations
    class Node;
    class Member;

    /**
     * Enum to identify each register, we try here to follow the x86_64 DASM reference from
     * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
     */
    enum Register {
        rax = 0, // accumulator
        rdx,      // storage
        COUNT
    };
    static std::string to_string(Register);

    /**
     * Enum to identify each function identifier.
     * A function is specified when using "call" instruction.
     */
    enum FctId
    {
        eval_member = 0
    };
    static std::string to_string(FctId);

    /**
     * Enumerate each possible instruction.
     */
    enum Instr
    {
        Instr_call,
        Instr_mov,
        Instr_jmp,
        Instr_jne,
        Instr_ret,
    };

    /**
     * Store a single assembly instruction ( line type larg rarg comment )
     */
    struct AssemblyInstr
    {
        // possible types for an argument. // TODO: use a single type, like char[4] for example.
        typedef mpark::variant<
                void*,
                Node*,
                Member*,
                long,
                Register,
                FctId
        > AsmInstrArg;

        AssemblyInstr(Instr _type, long _line): m_type(_type), m_line(_line) {}

        long        m_line;
        Instr       m_type;
        AsmInstrArg m_left_h_arg;
        AsmInstrArg m_right_h_arg;
        std::string m_comment;
        static std::string to_string(const AssemblyInstr&);
    };


    /**
     * Wraps an AssemblyInstr vector and add a shortcut to insert item easily.
     */
    class AssemblyCode
    {
    public:
        AssemblyCode() = default;
        ~AssemblyCode();

        AssemblyInstr*        push_instr(Instr _type);
        inline size_t         size() const { return  m_instructions.size(); }
        inline AssemblyInstr* operator[](size_t _index) const { return  m_instructions[_index]; }
        long                  get_next_pushed_instr_index() const { return m_instructions.size(); }
    private:
        std::vector<AssemblyInstr*> m_instructions;
    };
}
