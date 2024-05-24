#pragma once

#include <memory>

#include "tools/core/reflection/reflection"

#include "ndbl/core/Graph.h"
#include "ndbl/core/NodableHeadless.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/VirtualMachine.h"
#include "ndbl/core/assembly/Compiler.h"
#include "ndbl/core/language/Nodlang.h"

namespace ndbl
{
    /**
     * @brief Command Line Interface for Nodable
     */
    class CLI: public NodableHeadless
    {
    public:
        CLI();
        ~CLI();
        void         init() override;
        void         update();
        void         shutdown() override;
        bool         should_stop() const;

        // api
        void         clear();
        bool         compile();
        void         exit_();
        void         help();
        bool         parse();
        bool         run();
        bool         serialize();
        void         set_verbose();

        int          print_program();

        std::string test_return_str();
        std::string test_concat_str(std::string left, std::string right);

    private:
        std::string get_line() const;
        std::string get_word() const;

        std::string                m_source_code;
        bool                       m_should_stop{false};
        const assembly::Code*      m_asm_code;
        bool                       m_auto_completion{false};
        void log_function_call(const tools::variant &result, const tools::func_type *type) const;

    };
}
