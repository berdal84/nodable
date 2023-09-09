#pragma once

#include "fw/core/types.h"

#include "assembly/Compiler.h"
#include "assembly/Register.h"

namespace ndbl
{
    /*
     * In this classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it in order to be maybe one day compatible, for now it is just to be inspired by something solid.
    */

    using Code        = assembly::Code;
    using Instruction = assembly::Instruction;
    using Register    = assembly::Register;

    class CPU
    {
    public:
        CPU();
        ~CPU() = default;
        // Read a given register
        fw::qword     read(Register)const;
        // Write a word into a given register
        void          write(Register, fw::qword);
        // Clear all registers
        void          clear_registers();

    private:
        // Read a given register by reference with write mode
        fw::qword&    read_write(Register);
        // Store all registers
        fw::qword     m_register[Register::COUNT];
    };

    /**
     * The VirtualMachine is able to run the assembly::Code produced by the assembly::Compiler
     * The term VirtualMachine is maybe not adequate, it is closer to an interpreter in fact,
     * but I found it more clear.
    */
    class VirtualMachine
    {
    public:
        VirtualMachine();
        VirtualMachine(const VirtualMachine&) = delete;  // disable copy
        ~VirtualMachine() = default;
        // Load program code
        [[nodiscard]] bool    load_program(const Code *_code);
        // Release any loaded program
        const Code*           release_program();
        // Run loaded program. Check load_program()'s return value before to run.*/
        void                  run_program();
        // Stop the execution
        void                  stop_program();
        // Run the program in debug mode. Then call step_over() to advance step by step.*/
        void                  debug_program();
        // Check if a program is running
        inline bool           is_program_running() const{ return m_is_program_running; }
        // Check if a program is running in debug mode
        inline bool           is_debugging() const{ return m_is_debugging; }
        // Check if a program is stopped
        inline bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
        // Execute the next instruction. Works only in debug mode, use debug_program() and is_debugging()
               bool           step_over();
        // Get the next node to be executed. Works in debug mode only.
        inline const Node*    get_next_node() const {return m_next_node.get(); }
        // Get the last instruction's result
        fw::qword             get_last_result() const;
        // Check if there is a next instruction (internally check instruction pointer's position)
        bool                  is_there_a_next_instr() const;
        // Get the next instruction to execute
        Instruction*          get_next_instr() const;
        // Read a given CPU register
        fw::qword             read_cpu_register(Register _register) const;
        // Get current program ptr*/
        const Code *          get_program_asm_code();
        // Check if a given Node is the next to be executed
        bool                  is_next_node(PoolID<const Node> _node)const { return m_next_node == _node; }
    private:
        // Advance the instruction pointer of a given amount
        void                  advance_cursor(i64_t _amount = 1);
        // Step over common code (for both "run" and "debug" modes)
        bool                  _stepOver();

        PoolID<const Node>    m_next_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Instruction*          m_last_step_next_instr;
        CPU                   m_cpu;
        const Code*           m_program_asm_code;
    };
}


