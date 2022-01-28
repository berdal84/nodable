#pragma once

#include <nodable/GraphTraversal.h>
#include <nodable/InstructionNode.h>

namespace Nodable
{
    /**
     * In this classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it in order to be maybe one day compatible, for now it is just to be inspired by something solid.
     */

    // forward declarations
    class ScopedCodeBlockNode;
    class AssemblyCode;
    struct AssemblyInstr;

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
    };

    static std::string to_string(const AssemblyInstr&);

    /**
     * Wraps an AssemblyInstr vector and add a shortcut to insert item easily.
     */
    class AssemblyCode
    {
    public:
        AssemblyCode() = default;

        ~AssemblyCode()
        {
            for( auto each : m_instructions )
                delete each;
            m_instructions.clear();
        }

        AssemblyInstr* push_instr(Instr _type)
        {
            AssemblyInstr* instr = new AssemblyInstr(_type, m_instructions.size());
            m_instructions.emplace_back(instr);
            return instr;
        };

        inline size_t size() const { return  m_instructions.size(); }
        inline AssemblyInstr* operator[](size_t _index) const { return  m_instructions[_index]; }
        long get_next_pushed_instr_index(){ return m_instructions.size(); }
    private:
        std::vector<AssemblyInstr*> m_instructions;
    };

    /**
     * Class to compile and execute a Nodable program.
     *
     * TODO: extract compilation related code to a dedicated class Compiler.
     */
    class Runner {
    public:
        Runner();
        [[nodiscard]] bool    load_program(ScopedCodeBlockNode*);
        void                  unload_program();
        void                  run_program();
        void                  stop_program();
        void                  debug_program();
        inline bool           is_program_running() const{ return m_is_program_running; }
        inline bool           is_debugging() const{ return m_is_debugging; }
        inline bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
               bool           step_over();
        inline const Node*    get_current_node() const {return m_current_node; }
        inline Variant*       get_last_eval() { return &m_register[0]; }
        bool                  is_program_over() { assert(get_current_instruction()); return get_current_instruction()->m_type == Instr_ret; }
    private:
        void                  advance_cursor(long _amount = 1) { m_cursor_position += _amount; }
        void                  reset_cursor(){ m_cursor_position = 0; };
        AssemblyInstr*        get_current_instruction(){ return m_cursor_position < m_program_assembly->size() ? (*m_program_assembly)[m_cursor_position] : nullptr; };
        AssemblyCode*         create_assembly_code(const ScopedCodeBlockNode* _program);
        void                  append_to_assembly_code(const Node* _node);
        bool                  is_program_valid(const ScopedCodeBlockNode* _program);
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program_tree;
        AssemblyCode*         m_program_assembly;
        Node*                 m_current_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Variant               m_register[Register::COUNT]; // variants to store temp values
        size_t                m_cursor_position = 0;
    };
}


