#pragma once

#include <nodable/GraphTraversal.h>
#include <nodable/InstructionNode.h>

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;

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
                      inline InstructionNode* get_last_evaluated_instr() { return m_last_evaluated_instr; }

    private:
        bool                  is_program_valid(const ScopedCodeBlockNode* _program);
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program;
        Node*                 m_current_node;
        InstructionNode*      m_last_evaluated_instr;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
    };
}


