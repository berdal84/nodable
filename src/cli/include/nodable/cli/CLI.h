#pragma once

#include <memory>
#include <nodable/core/ILanguage.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/assembly/Compiler.h>

namespace Nodable
{
    /**
     * @brief Command Line Interface for Nodable
     */
    class CLI
    {
    public:
        CLI();
        ~CLI();
        bool         should_stop() const;
        void         update();
        static bool  help();
        static bool  log_stats();
        static bool  compile();
    private:
        std::unique_ptr<ILanguage> m_language;
        bool                       m_should_stop;
        NodeFactory                m_factory;
        GraphNode                  m_graph;
        static assembly::Compiler  m_compiler;
        static VirtualMachine      m_virtual_machine;
        bool                       m_auto_completion = false;
    };
}
