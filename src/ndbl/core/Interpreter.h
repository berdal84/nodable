#pragma once

#include "tools/core/types.h"

#include "Compiler.h"
#include "Register.h"

namespace ndbl
{
    /*
     * In these classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it just to be inspired by something solid, but it's NOT at all compatible.
    */
    class CPU
    {
    public:
        CPU();
        ~CPU() = default;
        tools::qword  read(Register)const;           // Read a given register
        void          write(Register, tools::qword); // Write a word into a given register
        void          clear_registers();             // Clear all registers

    private:
        tools::qword& read_write(Register); // Read a given register by reference with write mode
        tools::qword  m_register[Register_COUNT]; // Store all registers
    };

    /**
     * The Interpreter is able to run the Code produced by the Compiler
    */
    class Interpreter
    {
    public:
        [[nodiscard]] bool    load_program(const Code *_code);
        const Code*           release_program();  // Release any loaded program
        void                  run_program(); // Run loaded program. Check load_program()'s return value before to run.*/
        void                  stop_program();
        void                  debug_program(); // Run the program in debug mode. Then call step_over() to advance step by step.
        bool                  debug_step_over(); // Execute the next instruction. Works only in debug mode, use debug_program() and is_debugging()
        bool           is_program_running() const{ return m_is_program_running; }
        bool           is_debugging() const{ return m_is_debugging; }
        bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
        const ASTNode*    get_next_node() const {return m_next_node; } // Get the next node to be executed. Works in debug mode only.
        tools::qword          get_last_result() const; // Get the last instruction's result
        bool                  is_there_a_next_instr() const; // Check if there is a next instruction (internally check instruction pointer's position)
        Instruction*          get_next_instr() const; // Get the next instruction to execute
        tools::qword          read_cpu_register(Register _register) const; // Read a given CPU register
        const Code *          get_program_asm_code(); // Get current program ptr
        bool                  is_next_node(const ASTNode* _node)const { return m_next_node == _node; } // Check if a given Node is the next to be executed
        bool                  was_visited(const ASTNode *) const;

    private:
        void                  advance_cursor(i64_t _amount = 1);// Advance the instruction pointer of a given amount
        bool                  step_over(); // Step over common code (for both "run" and "debug" modes)
        const Graph*   graph() { ASSERT(m_code); return m_code->get_meta_data().graph; }
        CPU                   m_cpu;
        bool                  m_is_program_running   = false; // TODO: use StateMachine
        bool                  m_is_debugging         = false; // TODO: use StateMachine
        const Code*           m_code                 = nullptr;
        const ASTNode*           m_next_node            = nullptr;
        Instruction*          m_last_step_next_instr = nullptr;
        std::set<const ASTNode*> m_visited_nodes;
    };

    [[nodiscard]]
    Interpreter* init_interpreter(); // note: store ptr, you'll need it to shut it down.
    Interpreter* get_interpreter();
    void         shutdown_interpreter(Interpreter*); // Undo init_interpreter()
}


