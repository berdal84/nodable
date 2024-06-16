#pragma once

#include "ndbl/core/NodableHeadless.h"
#include "ndbl/core/VirtualMachine.h"
#include "ndbl/core/language/Nodlang.h"
#include <exception>
#include <gtest/gtest.h>
#include <string>

using namespace ndbl;

namespace testing
{
class Core : public Test
{
public:
    NodableHeadless app;
    Nodlang*        language{ nullptr};

    Core()
    {
        //tools::log::set_verbosity( tools::log::Verbosity_Verbose );
    }

    void SetUp()
    {
        app.init();
        language = app.get_language();
    }

    void TearDown()
    {
        try
        {
            app.shutdown();
        }
        catch (std::exception& error)
        {
            LOG_ERROR(__FILE__, "Exception during app.shutdown(): %s\n", error.what() );
        }
    }

    ~Core() = default;

    template<typename return_t>
    return_t eval(const std::string &_source_code)
    {
        static_assert(!std::is_pointer<return_t>::value, "returning a pointer from VM would fail (destroyed leaving this scope)");

        // parse
        Graph* graph = app.parse(_source_code);
        if (!graph->get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // compile
        auto asm_code = app.compile(graph);
        if (!asm_code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(asm_code) << std::flush;

        // load
        if (!app.load_program(asm_code))
        {
            throw std::runtime_error("VM was not able to load the compiled program.");
        }

        // run
        app.run_program();

        // get result
        return_t result = app.get_last_result_as<return_t>();

        app.release_program();

        return result;
    }

    std::string parse_eval_and_serialize(const std::string &_source_code)
    {
        LOG_MESSAGE("core", "parse_compile_run_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        Graph* graph = app.parse(_source_code);
        if (!graph->get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // compile
        const Code* code = app.compile(graph);
        if (!code)
        {
            throw std::runtime_error("core: Compiler was not able to compile program's graph.");
        }
        std::cout << Code::to_string(code) << std::flush;

        // load
        if (!app.load_program(code))
        {
            throw std::runtime_error("core: VM was not able to load the compiled program.");
        }

        // run
        app.run_program();

        // serialize
        std::string result;
        app.serialize( result );
        LOG_VERBOSE("core", "parse_compile_run_serialize serialize_node() output is: \"%s\"\n", result.c_str());
        app.release_program();

        return result;
    }

    std::string parse_and_serialize(const std::string &_source_code)
    {
        LOG_VERBOSE("core", "parse_and_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        Graph* graph = app.parse(_source_code);
        if (!graph->get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // serialize
        std::string result;
        app.serialize( result );
        LOG_VERBOSE("tools.h", "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    void log_ribbon() const
    {
        LOG_MESSAGE("fixture::core", "%s\n\n", get_language()->parser_state.ribbon.to_string().c_str());
    }
};
}
