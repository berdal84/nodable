#pragma once
#include <gtest/gtest.h>
#include <exception>
#include <string>

#include <nodable/core/GraphNode.h>
#include <nodable/core/Node.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/Property.h>
#include <nodable/core/Scope.h>
#include <nodable/core/String.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/languages/NodableParser.h>
#include <nodable/core/types.h>

using namespace ndbl;

class nodable_fixture: public ::testing::Test {
public:
    NodableLanguage       language;
    const NodeFactory     factory;
    bool                  autocompletion = false;
    GraphNode             graph;
    assembly::Compiler    compiler;
    VirtualMachine        virtual_machine;

    nodable_fixture( )
    : factory(&language)
    , graph(&language, &factory, &autocompletion) {}

    void SetUp( ) {
    }

    void TearDown( ) {
    }

    ~nodable_fixture( )  {
        // cleanup any pending stuff, but no exceptions allowed
    }

    template<typename return_t>
    return_t eval(const std::string& _source_code)
    {
        static_assert( !std::is_pointer<return_t>::value ); // returning a pointer from VM will fail when accessing data
                                                            // since VM will be destroyed leaving this scope.
        // parse
        language.get_parser().parse(_source_code, &graph);

        // compile
        auto asm_code = compiler.compile_syntax_tree(&graph);
        if (!asm_code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(asm_code.get()) << std::flush;

        // load
        if (!virtual_machine.load_program(std::move(asm_code) ))
        {
            throw std::runtime_error("VM was not able to load the compiled program.");
        }

        // run
        virtual_machine.run_program();

        // get result
        qword mem_space = virtual_machine.get_last_result();
        auto result = return_t(mem_space);

        virtual_machine.release_program();

        return result;
    }

    std::string parse_eval_and_serialize(const std::string& _source_code)
    {
        LOG_MESSAGE("nodable_fixture", "parse_compile_run_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        language.get_parser().parse(_source_code, &graph);

        // compile
        auto code = compiler.compile_syntax_tree(&graph);
        if (!code)
        {
            throw std::runtime_error("nodable_fixture: Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(code.get()) << std::flush;

        // load
        if (!virtual_machine.load_program(std::move(code) ))
        {
            throw std::runtime_error("nodable_fixture: VM was not able to load the compiled program.");
        }

        // run
        virtual_machine.run_program();

        // serialize
        std::string result;
        language.get_serializer().serialize(result, graph.get_root() );
        LOG_VERBOSE("nodable_fixture", "parse_compile_run_serialize serialize output is: \"%s\"\n", result.c_str());

        virtual_machine.release_program();
        return result;
    }

    std::string parse_and_serialize(const std::string& _source_code)
    {
        LOG_VERBOSE("nodable_fixture", "parse_and_serialize parsing \"%s\"\n", _source_code.c_str());

        // parse
        language.get_parser().parse(_source_code, &graph);
        if ( !graph.get_root())
        {
            throw std::runtime_error("parse_and_serialize: Unable to generate program.");
        }

        // serialize
        std::string result;
        language.get_serializer().serialize(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "parse_and_serialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }
};
