#pragma once

#include <nodable/Nodable.h>
#include <nodable/GraphTraversal.h>
#include <nodable/InstructionNode.h>
#include <nodable/Assembly.h>

namespace Nodable
{
    /**
     * In this classes/struct/enum we try to follow as much as possible the x86_64 DASM reference
     * (https://www.cs.uaf.edu/2017/fall/cs301/reference/x86_64.html)
     * We do it in order to be maybe one day compatible, for now it is just to be inspired by something solid.
     */

    // forward declarations
    class ScopeNode;

    namespace Asm
    {
        /**
         * Class to execute a Nodable program (an Asm::Code compiled by an Asm::Compiler).
         */
        class VM {
        public:
            VM();
            [[nodiscard]] bool    load_program(ScopeNode*);
            void                  unload_program();
            void                  run_program();
            void                  stop_program();
            void                  debug_program();
            inline bool           is_program_running() const{ return m_is_program_running; }
            inline bool           is_debugging() const{ return m_is_debugging; }
            inline bool           is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
                   bool           step_over();
            inline const Node*    get_next_node() const {return m_next_node; }
            inline const Variant* get_last_result() { return (Variant*)m_register[Register::rax]; }
            bool                  is_program_over() { assert(get_next_instr()); return get_next_instr()->m_type == Instr_t::ret; }
            const Code*           get_program_asm_code()const { return m_program_asm_code; }
            Instr*                get_next_instr(){ return m_register[Register::esp] < m_program_asm_code->size() ? (*m_program_asm_code)[m_register[Register::esp]] : nullptr; };
        private:
            void                  advance_cursor(i64_t _amount = 1) { m_register[Register::esp] += _amount; }
            void                  reset_cursor(){ m_register[Asm::Register::esp] = 0; };
            bool                  _stepOver();
            GraphTraversal        m_traversal;
            ScopeNode*            m_program_graph;
            Asm::Code*            m_program_asm_code;
            Node*                 m_next_node;
            bool                  m_is_program_running;
            bool                  m_is_debugging;
            Instr*                m_last_step_next_instr;
            i64_t                 m_register[Register::COUNT];
        };
    }
}


