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
        inline bool isRunning(){ return m_isRunning; }
        inline bool isDebugging(){ return m_isDebugging; }
        inline bool isStopped(){ return !m_isDebugging && !m_isRunning; }
//        void pause();
//        void stop();
        bool stepOver();
//        void stepInto();
//        void stepOut();
    private:
        NodeTraversal m_traversal;
        ProgramNode* m_program;
        Node* m_currentNode;
        bool m_isRunning;
        bool m_isDebugging;
    };
}


