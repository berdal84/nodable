#pragma once

#include <memory>
#include <nodable/core/types.h>
#include "nodable/core/assembly/Compiler.h"

namespace Nodable
{
    /**
     * In this classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it in order to be maybe one day compatible, for now it is just to be inspired by something solid.
     */

    using Code        = assembly::Code;
    using QWord       = assembly::QWord;
    using Instruction = assembly::Instruction;
    using Register    = assembly::Register;

    class CPU
    {
    public:
        CPU();
        ~CPU() = default;
        QWord         read(Register)const;
        void          write(Register, QWord);
        void          clear_registers();

    private:
        QWord&        _read(Register);
        void          init_eip(); // instruction pointer
        QWord         m_register[Register::COUNT];
    };

    /**
     * Class to execute a compiled
     */
    class VirtualMachine
    {
        using code_uptr = std::unique_ptr<const Code>;
    public:
        VirtualMachine();
        ~VirtualMachine() = default;
        [[nodiscard]] bool    load_program(code_uptr _code);
        code_uptr             release_program();
        void                  run_program();
        void                  stop_program();
        void                  debug_program();
        inline bool           is_program_running() const{ return m_is_program_running; }
        inline bool           is_debugging() const{ return m_is_debugging; }
        inline bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
               bool           step_over();
        inline const Node*    get_next_node() const {return m_next_node; }
        QWord                 get_last_result() const;
        bool                  is_there_a_next_instr() const;
        Instruction*          get_next_instr() const;
        QWord                 read_cpu_register(Register _register) const;
        const Code *          get_program_asm_code();
        bool                  is_next_node(Node *_node)const { return m_next_node == _node; }

    private:
        void                  advance_cursor(i64_t _amount = 1);
        bool                  _stepOver();

        const Node*           m_next_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Instruction*          m_last_step_next_instr;
        CPU                   m_cpu;
        code_uptr             m_program_asm_code;

    };
}


