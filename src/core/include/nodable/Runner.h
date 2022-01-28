#pragma once

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
    class ScopedCodeBlockNode;

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
        bool                  is_program_over() { assert(get_current_instruction()); return get_current_instruction()->m_type == Asm::Instr::Type::ret; }
    private:
        void                  advance_cursor(long _amount = 1) { m_cursor_position += _amount; }
        void                  reset_cursor(){ m_cursor_position = 0; };
        Asm::Instr*           get_current_instruction(){ return m_cursor_position < m_program_asm_code->size() ? (*m_program_asm_code)[m_cursor_position] : nullptr; };
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program_graph;
        Asm::Code*            m_program_asm_code;
        Node*                 m_current_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Variant               m_register[Asm::Register::COUNT]; // variants to store temp values
        size_t                m_cursor_position = 0;
    };
}


