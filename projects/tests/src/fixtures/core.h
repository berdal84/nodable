#pragma once

#include <exception>
#include <gtest/gtest.h>
#include <string>

#include <fw/core/string.h>
#include <fw/core/types.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/NodeFactory.h>
#include <ndbl/core/Property.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/VirtualMachine.h>
#include <ndbl/core/language/Nodlang.h>

using namespace ndbl;

namespace testing
{
class Core : public Test
{
public:
    Nodlang             language;
    const NodeFactory   factory;
    bool                autocompletion = false;
    GraphNode           graph;
    assembly::Compiler  compiler;
    VirtualMachine      virtual_machine;

    Core()
        : factory(&language), graph(&language, &factory, &autocompletion) {}

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    ~Core()
    {
        // cleanup any pending stuff, but no exceptions allowed
    }

    template<typename return_t>
    return_t eval(const std::string &_source_code)
    {
        static_assert(!std::is_pointer<return_t>::value);// returning a pointer from VM will fail when accessing data
                                                         // since VM will be destroyed leaving this scope.
        // parse
        language.parse(_source_code, &graph);

        // compile
        auto asm_code = compiler.compile_syntax_tree(&graph);
        if (!asm_code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(asm_code.get()) << std::flush;

        // load
        if (!virtual_machine.load_program(std::move(asm_code)))
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
        language.parse(_source_code, &graph);

        // compile
        auto code = compiler.compile_syntax_tree(&graph);
        if (!code)
        {
            throw std::runtime_error("core: Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(code.get()) << std::flush;

        // load
        if (!virtual_machine.load_program(std::move(code)))
        {
            throw std::runtime_error("core: VM was not able to load the compiled program.");
        }

        // run
        virtual_machine.run_program();

        // serialize
        std::string result;
        language.serialize(result, graph.get_root());
        LOG_VERBOSE("core", "parse_compile_run_serialize serialize output is: \"%s\"\n", result.c_str());

        virtual_machine.release_program();
        return result;
    }

    std::string parse_and_serialize(const std::string &_source_code)
    {
        LOG_VERBOSE("core", "parse_and_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        language.parse(_source_code, &graph);
        if (!graph.get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // serialize
        std::string result;
        language.serialize(result, graph.get_root());
        LOG_VERBOSE("tools.h", "parse_and_serialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }
};
}
