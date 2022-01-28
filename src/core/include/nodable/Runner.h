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
    struct SimpleInstrList;

    /**
     * Enum to identify each register, we try here to follow the x86_64 DASM reference from
     * @see https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html
     */
    enum Register {
        rax = 0, // accumulator
        rdx      // storage
    };
    static std::string to_string(Register);

    enum FctId
    {
        eval_member = 0
    };
    static std::string to_string(FctId);

    /**
     * Class to execute a Nodable program.
     */
    class Runner {
    public:
        Runner();
        [[nodiscard]] bool load_program(ScopedCodeBlockNode*);
        void unload_program();
        void run_program();
        bool is_program_over();
        void stop_program();
        void debug_program();
        [[nodiscard]] inline bool             is_program_running() const{ return m_is_program_running; }
        [[nodiscard]] inline bool             is_debugging() const{ return m_is_debugging; }
        [[nodiscard]] inline bool             is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
                             bool             step_over();
        [[nodiscard]] inline const Node*      get_current_node() const {return m_current_node; }
                      inline Variant*         get_last_eval() { return &m_register[0]; }

    private:
        SimpleInstrList*      compile_program(const ScopedCodeBlockNode* _program);
        void                  compile_node_and_append_to_program(const Node* _node);
        bool                  is_program_valid(const ScopedCodeBlockNode* _program);
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program_tree;
        SimpleInstrList*      m_program_compiled;
        Node*                 m_current_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Variant               m_register[2]; // variants to store temp values
    };


    enum Instr
    {
        Instr_udef,
        Instr_call,
        Instr_mov,
        Instr_jmp,
        Instr_jne,
        Instr_ret,
    };

    class SimpleInstr
    {
    public:

        SimpleInstr(Instr _type, long _line): m_type(_type), m_line(_line) {}
        SimpleInstr(const SimpleInstr& _other) = default;

        std::string to_string();

        Instr m_type;
        long   m_line;
        mpark::variant<void* , Node*, Member*, long, Register, FctId> m_left_h_arg;
        mpark::variant<void* , Node*, Member*, long, Register, FctId> m_right_h_arg;
        std::string m_comment;
     };

    /**
     * Class to store a simple instruction list and navigate forward through it
     */
     class SimpleInstrList
     {
     public:
         SimpleInstrList(){}
         ~SimpleInstrList()
         {
             for( auto each : m_instructions )
                 delete each;
             m_instructions.clear();
         }
         void         advance(long _amount = 1) { m_cursor_position += _amount; }
         SimpleInstr* push_instr(Instr _type)
         {
             SimpleInstr* instr = new SimpleInstr(_type, m_instructions.size());
             m_instructions.emplace_back(instr);
             return instr;
         };
         SimpleInstr* get_curr(){ return m_cursor_position < m_instructions.size() ? m_instructions[m_cursor_position] : nullptr; };
         void         reset_cursor(){ m_cursor_position = 0; };
         long         get_next_line_nb(){ return m_instructions.size(); }
         bool         is_over() { assert(get_curr()); return get_curr()->m_type == Instr_ret; }
     private:
         size_t m_cursor_position = 0;
         std::vector<SimpleInstr*> m_instructions;
     };
}


