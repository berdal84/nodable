#pragma once

#include <memory>

#include "tools/core/reflection/reflection"

#include "ndbl/core/Graph.h"
#include "ndbl/core/NodableHeadless.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/Interpreter.h"
#include "ndbl/core/Compiler.h"
#include "ndbl/core/language/Nodlang.h"

namespace ndbl
{
    /**
     * @brief Command Line Interface for Nodable
     */
    class CLI: public NodableHeadless
    {
    public:

        struct PublicApi
        {
            explicit      PublicApi(CLI* cli): m_cli(cli) {}

            void          clear();
            bool          compile();
            void          quit();
            void          help();
            bool          parse();
            bool          run();
            bool          serialize();
            void          set_verbose();
            int           print_program();
        private:
            CLI*          m_cli;
        };

        PublicApi  api;

        void       init() override;
        void       update() override;
        void       shutdown() override;
        void       clear() override;
        bool       run();

        CLI(): NodableHeadless(), api(this) {}
        ~CLI() override = default;

    private:
        static std::string get_line() ;
        static void log_function_call(const tools::variant &result, const tools::func_type *type) ;
    };
}
