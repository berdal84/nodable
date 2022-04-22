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
        void         help();
        void         parse();
        void         serialize();
        void         compile();
        void         run();
        void         exit_();

    private:
        std::string get_line() const;

        std::unique_ptr<ILanguage> m_language;
        bool                       m_should_stop;
        NodeFactory                m_factory;
        GraphNode                  m_graph;
        assembly::Compiler         m_compiler;
        std::unique_ptr<const assembly::Code> m_asm_code;
        VirtualMachine             m_virtual_machine;
        bool                       m_auto_completion = false;
    };
}
