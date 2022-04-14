#pragma once
#include <gtest/gtest.h>
#include <exception>
#include <string>

#include <nodable/core/types.h>
#include <nodable/core/Node.h>
#include <nodable/core/Member.h>
#include <nodable/core/VirtualMachine.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/Parser.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/NodeFactory.h>
#include <nodable/core/String.h>
#include <nodable/core/Scope.h>
#include <nodable/core/languages/Nodable.h>

namespace Nodable
{
    /*
     * TODO: instead of using those helper functions I must use gtest fixtures.
     *       this class will be constructed/destructed automatically before/after each test
     *       and the test scope will have access to the fixture class scope.
     *
     *       read: fixtures https://developer.ibm.com/articles/au-googletestingframework/
     */

    template<typename return_t>
    static return_t ParseAndEvalExpression(const std::string &expression)
    {
        static_assert( !std::is_pointer<return_t>::value ); // returning a pointer from VM will fail when accessing data
                                                            // since VM will be destroyed leaving this scope.

        // prepare
        assembly::Compiler    compiler;
        VirtualMachine        vm;
        LanguageNodable       lang;
        NodeFactory           factory(&lang);
        bool                  autocompletion = false;
        GraphNode             graph(&lang, &factory, &autocompletion);
        std::string           asm_code_string;

        // create program graph
        lang.get_parser().parse_graph(expression, &graph);

        // compile
        auto asm_code = compiler.compile_syntax_tree(&graph);
        if (!asm_code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(asm_code.get()) << std::flush;
        // load
        if (!vm.load_program( std::move(asm_code) ))
        {
            throw std::runtime_error("VM was not able to load the compiled program.");
        }

        // run
        vm.run_program();

        // ret result
        assembly::QWord mem_space = vm.get_last_result();

        auto result = return_t(mem_space);

        return result;
    }


    static std::string &ParseUpdateSerialize(std::string &result, const std::string &expression)
    {
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize parsing \"%s\"\n", expression.c_str());

        // prepare
        LanguageNodable       lang;
        NodeFactory           factory(&lang);
        bool                  autocompletion = false;
        GraphNode             graph(&lang, &factory, &autocompletion);
        assembly::Compiler    compiler;
        VirtualMachine        vm;

        // act
        lang.get_parser().parse_graph(expression, &graph);

        // compile
        auto code = compiler.compile_syntax_tree(&graph);
        if (!code)
        {
            throw std::runtime_error("Compiler was not able to compile program's graph.");
        }
        std::cout << assembly::Code::to_string(code.get()) << std::flush;
        // load
        if (!vm.load_program( std::move(code) ))
        {
            throw std::runtime_error("VM was not able to load the compiled program.");
        }

        // run
        vm.run_program();

        lang.get_serializer().serialize(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "ParseUpdateSerialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }

    static std::string ParseAndSerialize(const std::string &expression)
    {
        LOG_VERBOSE("tools.h", "ParseAndSerialize parsing \"%s\"\n", expression.c_str());
        // prepare
        bool             autocompletion  = false;
        LanguageNodable  lang;
        NodeFactory      factory(&lang);
        GraphNode        graph(&lang, &factory, &autocompletion);

        // act
        lang.get_parser().parse_graph(expression, &graph);
        if ( !graph.get_root())
        {
            throw std::runtime_error("ParseAndSerialize: Unable to generate program.");
        }

        std::string result;
        lang.get_serializer().serialize(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "ParseUpdateSerialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }

    static void ParseEvalSerializeExpressions(const std::vector<std::string> &expressions)
    {
        for (const auto &original_expr : expressions)
        {
            std::string result_expr;
            ParseUpdateSerialize(result_expr, original_expr);
            EXPECT_EQ(result_expr, original_expr);
        }
    }
}