#pragma once
#include <gtest/gtest.h>
#include <exception>

#include <nodable/core/Node.h>
#include <nodable/core/Member.h>
#include <nodable/core/VM.h>
#include <nodable/core/GraphNode.h>
#include <nodable/core/Parser.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/HeadlessNodeFactory.h>
#include <nodable/core/Format.h>
#include <nodable/core/Scope.h>
#include <nodable/core/LanguageNodable.h>

namespace Nodable
{
    template<typename return_t>
    static return_t ParseAndEvalExpression(const std::string &expression)
    {
        // prepare
        return_t result{};
        const LanguageNodable lang;
        Asm::Compiler compiler;
        Asm::VM vm;
        HeadlessNodeFactory factory(&lang);
        bool autocompletion = false;
        GraphNode graph(&lang, &factory, &autocompletion);
        std::string asm_code_string;

        // create program
        lang.getParser()->parse_graph(expression, &graph);

        auto program = graph.get_root();
        if (program)
        {
            // compile
            auto asm_code = compiler.compile_syntax_tree(graph.get_root());
            if (!asm_code)
            {
                throw std::runtime_error("Compiler was not able to compile program's graph.");
            }
            std::cout << Asm::Code::to_string(asm_code.get()) << std::flush;
            // load
            if (!vm.load_program( std::move(asm_code) ))
            {
                throw std::runtime_error("VM was not able to load the compiled program.");
            }

            // run
            vm.run_program();

            // ret result
            const Asm::MemSpace* mem_space = vm.get_last_result();
            if (mem_space == nullptr )
            {
                throw std::runtime_error("Unable to get program's last result.");
            }

            NODABLE_ASSERT(mem_space->type == Asm::MemSpace::Type::VariantPtr)
            const Variant* variant = mem_space->data.m_variant;
            NODABLE_ASSERT(!variant->get_meta_type()->is(R::get_meta_type<Node*>()) ) // we do not accept a result as Node*
            result = variant->convert_to<return_t>();
        }
        else
        {
            throw std::runtime_error("Unable to convert expression to graph, program is nullptr.");
        }

        return result;
    }


    static std::string &ParseUpdateSerialize(std::string &result, const std::string &expression) {
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize parsing \"%s\"\n", expression.c_str());
        // prepare
        const LanguageNodable lang;
        HeadlessNodeFactory factory(&lang);
        bool autocompletion = false;
        GraphNode graph(&lang, &factory, &autocompletion);
        Asm::Compiler compiler;
        Asm::VM vm;

        // act
        lang.getParser()->parse_graph(expression, &graph);
        if (Node* root = graph.get_root())
        {
            // compile
            auto code = compiler.compile_syntax_tree(root);
            if (!code)
            {
                throw std::runtime_error("Compiler was not able to compile program's graph.");
            }
            std::cout << Asm::Code::to_string(code.get()) << std::flush;
            // load
            if (!vm.load_program( std::move(code) ))
            {
                throw std::runtime_error("VM was not able to load the compiled program.");
            }

            // run
            vm.run_program();

            // serialize result
            if ( vm.get_last_result() )
            {
                LOG_VERBOSE("tools.h", "ParseUpdateSerialize has result\n");
            }
            else
            {
                throw std::runtime_error("ParseUpdateSerialize: Unable to get last evaluated member.");
            }
        }
        else
        {
            throw std::runtime_error("ParseUpdateSerialize: Unable to generate program.");
        }

        Serializer *serializer = lang.getSerializer();
        serializer->serialize(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "ParseUpdateSerialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }

    static std::string ParseAndSerialize(const std::string &expression) {
        LOG_VERBOSE("tools.h", "ParseAndSerialize parsing \"%s\"\n", expression.c_str());
        // prepare
        const LanguageNodable lang;
        HeadlessNodeFactory factory(&lang);
        bool autocompletion  = false;
        GraphNode graph(&lang, &factory, &autocompletion);

        // act
        lang.getParser()->parse_graph(expression, &graph);
        if ( !graph.get_root())
        {
            throw std::runtime_error("ParseAndSerialize: Unable to generate program.");
        }

        Serializer *serializer = lang.getSerializer();
        std::string result;
        serializer->serialize(result, graph.get_root() );
        LOG_VERBOSE("tools.h", "ParseUpdateSerialize serialize output is: \"%s\"\n", result.c_str());

        return result;
    }

    static void ParseEvalSerializeExpressions(const std::vector<std::string> &expressions) {
        for (const auto &original_expr : expressions) {
            std::string result_expr;
            ParseUpdateSerialize(result_expr, original_expr);
            EXPECT_EQ(result_expr, original_expr);
        }
    }
}