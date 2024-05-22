#pragma once

#include <memory>

#include "tools/core/reflection/reflection"

#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/VirtualMachine.h"
#include "ndbl/core/assembly/Compiler.h"

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
        void         set_verbose();

        int          print_program();
        std::string test_return_str() { return (std::string)m_virtual_machine.get_last_result(); }

        std::string test_concat_str(std::string left, std::string right) { return left + right; }

    private:
        std::string get_line() const;
        std::string get_word() const;

        Nodlang                    m_language;
        bool                       m_should_stop;
        NodeFactory                m_factory;
        Graph                      m_graph;
        assembly::Compiler         m_compiler;
        std::string                m_source_code;
        const assembly::Code*      m_asm_code;
        VirtualMachine             m_virtual_machine;
        bool                       m_auto_completion = false;
        void log_function_call(const tools::variant &result, const tools::func_type *type) const;

    };
}
