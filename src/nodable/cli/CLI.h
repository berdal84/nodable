#pragma once

#include <memory>

#include "fw/core/reflection/reflection"

#include "nodable/core/language/Nodlang.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/core/Graph.h"
#include "nodable/core/VirtualMachine.h"
#include "nodable/core/assembly/Compiler.h"

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
        int          main(int argc, char* argv[]);
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
        void log_function_call(const fw::variant &result, const fw::func_type *type) const;

    };
}
