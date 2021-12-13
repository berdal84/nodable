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
        void load(ScopedCodeBlockNode*);
        void unload();
        void run();
        bool isProgramOver();
        void stop();
        void debug();
        [[nodiscard]] inline bool isRunning() const{ return m_isRunning; }
        [[nodiscard]] inline bool isDebugging() const{ return m_isDebugging; }
        [[nodiscard]] inline bool isStopped() const{ return !m_isDebugging && !m_isRunning; }
//        void pause();
//        void stop();
        bool stepOver();
//        void stepInto();
//        void stepOut();
        [[nodiscard]] inline const Node* getCurrentNode() const {return m_currentNode; }

        inline InstructionNode *getLastEvaluatedInstruction() { return m_lastInstructionNode; }

    private:
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program;
        Node*                 m_currentNode;
        InstructionNode*      m_lastInstructionNode;
        bool                  m_isRunning;
        bool                  m_isDebugging;

        void eval(Node *_node);
    };
}


