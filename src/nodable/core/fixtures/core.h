#pragma once

#include <exception>
#include <gtest/gtest.h>
#include <string>

#include "fw/core/string.h"
#include "fw/core/types.h"
#include "nodable/core/Graph.h"
#include "nodable/core/Node.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/core/Property.h"
#include "nodable/core/Scope.h"
#include "nodable/core/VariableNode.h"
#include "nodable/core/VirtualMachine.h"
#include "nodable/core/language/Nodlang.h"

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
    {}

    void SetUp()
    {
        fw::pool::Pool::init();
    }

    void TearDown()
    {
        graph.clear();
        fw::pool::Pool::shutdown();
    }

    ~Core()
    {
        // cleanup any pending stuff, but no exceptions allowed
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
        fw::qword mem_space = virtual_machine.get_last_result();
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
        nodlang.serialize_node(result, graph.get_root() );
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
        nodlang.serialize_node(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "parse_and_serialize serialize_node() output is: \"%s\"\n", result.c_str());

        return result;
    }

    void log_ribbon() const
    {
        LOG_MESSAGE("fixture::core", "%s\n", nodlang.parser_state.ribbon.to_string().c_str());
    }
};
}
