#pragma once

#include <memory>
#include <nodable/core/types.h>
#include <nodable/core/Compiler.h>

namespace Nodable
{
    /**
     * In this classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it in order to be maybe one day compatible, for now it is just to be inspired by something solid.
     */

    namespace Asm
    {
        class CPU
        {
        public:
            CPU();
            ~CPU() = default;
            MemSpace&        read_register(Register _id);
            const MemSpace&  read_register(Register _id)const;
            void             write_register(Register _id, MemSpace _mem_src);
            void             clear_registers();

        private:
            void             init_eip_register(); // instruction pointer
            MemSpace         m_register[Register::COUNT];
        };

        /**
         * Class to execute a Nodable program (an Asm::Code compiled by an Asm::Compiler).
         */
        class VM
        {
        public:
            VM();
            ~VM() = default;
            [[nodiscard]] bool          load_program(std::unique_ptr<const Code> _code);
            std::unique_ptr<const Code> release_program();
            void                  run_program();
            void                  stop_program();
            void                  debug_program();
            inline bool           is_program_running() const{ return m_is_program_running; }
            inline bool           is_debugging() const{ return m_is_debugging; }
            inline bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
                   bool           step_over();
            inline const Node*    get_next_node() const {return m_next_node; }
            const MemSpace*       get_last_result() const;
            bool                  is_there_a_next_instr() const;
            const Asm::Code*      get_program_asm_code()const { return m_program_asm_code.get(); }
            Instr*                get_next_instr() const;
            const MemSpace&       read_cpu_register(Register _register)const;

        private:
            void                  advance_cursor(i64 _amount = 1);

            bool                  _stepOver();

            const Node*           m_next_node;
            bool                  m_is_program_running;
            bool                  m_is_debugging;
            Instr*                m_last_step_next_instr;
            CPU                   m_cpu;
            std::unique_ptr<const Asm::Code> m_program_asm_code;
        };
    }
}


