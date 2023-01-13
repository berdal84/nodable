#pragma once

#include <memory>
#include <nodable/core/ILanguage.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/assembly/Compiler.h>

namespace ndbl
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

        // api
        void         clear();
        bool         compile();
        void         exit_();
        void         help();
        bool         parse();
        bool         run();
        bool         serialize();
        void         update();

        std::string test_return_str() { return (std::string)m_virtual_machine.get_last_result(); }
        std::string test_concat_str(std::string left, std::string right) { return left + right; }

    private:
        std::string get_line() const;
        std::string get_word() const;

        std::unique_ptr<ILanguage> m_language;
        bool                       m_should_stop;
        NodeFactory                m_factory;
        GraphNode                  m_graph;
        assembly::Compiler         m_compiler;
        std::unique_ptr<const assembly::Code> m_asm_code;
        VirtualMachine             m_virtual_machine;
        bool                       m_auto_completion = false;
        void log_function_call(const variant &result, const func_type &type) const;
    };
}
