#pragma once

#include <exception>
#include <gtest/gtest.h>
#include <string>

#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/Property.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/core/VirtualMachine.h"
#include "ndbl/core/core.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/core.h"
#include "tools/core/string.h"
#include "tools/core/types.h"
#include "tools/gui/gui.h"

using namespace ndbl;

namespace testing
{
class Core : public Test
{
public:
    Nodlang             nodlang;
    const NodeFactory   factory;
    Graph               graph;
    assembly::Compiler  compiler;
    VirtualMachine      virtual_machine;

    Core()
    : graph(&factory)
    {
        //tools::log::set_verbosity( tools::log::Verbosity_Verbose );
    }

    void SetUp()
    {
        ndbl::core_init();
    }

    void TearDown()
    {
        try
        {
            // Must be cleared before Pool shutdown
            graph.clear();
        }
        catch (std::exception& error)
        {
            LOG_ERROR(__FILE__, "Exception during graph.clear(): %s\n", error.what() );
        }
        ndbl::core_shutdown();
    }

    ~Core()
    {
    }

    template<typename return_t>
    return_t eval(const std::string &_source_code)
    {
        static_assert(!std::is_pointer<return_t>::value, "returning a pointer from VM would fail (destroyed leaving this scope)");

        // parse
        nodlang.parse(_source_code, &graph);

        // compile
        auto asm_code = compiler.compile_syntax_tree(&graph);
        if (!asm_code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(asm_code) << std::flush;

        // load
        if (!virtual_machine.load_program(asm_code))
        {
            throw std::runtime_error("VM was not able to load the compiled program.");
        }

        // run
        virtual_machine.run_program();

        // get result
        tools::qword mem_space = virtual_machine.get_last_result();
        auto result = return_t(mem_space);

        virtual_machine.release_program();

        return result;
    }

    std::string parse_eval_and_serialize(const std::string &_source_code)
    {
        LOG_MESSAGE("core", "parse_compile_run_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        nodlang.parse(_source_code, &graph);

        // compile
        auto code = compiler.compile_syntax_tree(&graph);
        if (!code)
        {
            throw std::runtime_error("core: Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(code) << std::flush;

        // load
        if (!virtual_machine.load_program(code))
        {
            throw std::runtime_error("core: VM was not able to load the compiled program.");
        }

        // run
        virtual_machine.run_program();

        // serialize
        std::string result;
        nodlang.serialize_node( result, graph.get_root() );
        LOG_VERBOSE("core", "parse_compile_run_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        virtual_machine.release_program();
        return result;
    }

    std::string parse_and_serialize(const std::string &_source_code)
    {
        LOG_VERBOSE("core", "parse_and_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        nodlang.parse(_source_code, &graph);
        if (!graph.get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // serialize
        std::string result;
        nodlang.serialize_node( result, graph.get_root() );
        LOG_VERBOSE("tools.h", "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    void log_ribbon() const
    {
        LOG_MESSAGE("fixture::core", "%s\n", nodlang.parser_state.ribbon.to_string().c_str());
    }
};
}
