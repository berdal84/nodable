#pragma once

#include <NodeTraversal.h>

namespace Nodable
{
    // forward declarations
    class ProgramNode;

    /**
     * Class to execute a Nodable program.
     */
    class VirtualMachine {
    public:
        VirtualMachine();
        void load(ProgramNode*);
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
    private:
        NodeTraversal m_traversal;
        ProgramNode* m_program;
        Node* m_currentNode{};
        bool m_isRunning;
        bool m_isDebugging;
    };
}


