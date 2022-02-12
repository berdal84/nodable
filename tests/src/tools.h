#pragma once
#include <gtest/gtest.h>
#include <exception>

#include <nodable/Member.h>
#include <nodable/VM.h>
#include <nodable/GraphNode.h>
#include <nodable/Parser.h>
#include <nodable/VariableNode.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/String.h>
#include <nodable/Scope.h>
#include <nodable/LanguageNodable.h>

namespace Nodable
{
    template<typename return_t>
    static return_t ParseAndEvalExpression(const std::string &expression)
    {
        // prepare
        return_t result{};
        const LanguageNodable lang;
        HeadlessNodeFactory factory(&lang);
        GraphNode graph(&lang, &factory);

        // create program
        lang.getParser()->parse_graph(expression, &graph);

        auto program = graph.get_root();
        if (program)
        {
            // run
            Asm::VM runner;

            if (!runner.load_program(graph.get_root()))
            {
                throw std::runtime_error("Unable to load program.");
            }
            runner.run_program();

            const Variant* last_result = runner.get_last_result();
            if ( last_result == nullptr )
            {
                throw std::runtime_error("Unable to get program's last result.");
            }
            result = last_result->convert_to<return_t>();
        }
        else
        {
            throw std::runtime_error("Unable to convert expression to graph, program is nullptr.");
        }

        return result;
    }


    static std::string &ParseUpdateSerialize(std::string &result, const std::string &expression) {
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize parsing %s\n", expression.c_str());
        // prepare
        const LanguageNodable lang;
        HeadlessNodeFactory factory(&lang);
        GraphNode graph(&lang, &factory);

        // act
        lang.getParser()->parse_graph(expression, &graph);
        if (Node* program = graph.get_root()) {
            Asm::VM runner;

            if (runner.load_program(program)) {
                runner.run_program();

                if (const Variant* last_eval = runner.get_last_result())
                {
                    std::string result_str = last_eval->convert_to<std::string>();
                    LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize result is: %s\n", result_str.c_str());
                }
                else
                {
                    throw std::runtime_error("ParseUpdateSerialize: Unable to get last evaluated member.");
                }
            }
            else
            {
                throw std::runtime_error("ParseUpdateSerialize: Unable to load program.");
            }

        }
        else
        {
            throw std::runtime_error("ParseUpdateSerialize: Unable to generate program.");
        }

        Serializer *serializer = lang.getSerializer();
        serializer->serialize(result, graph.get_root() );
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize serialize output is: %s\n", result.c_str());

        return result;
    }

    static std::string ParseAndSerialize(const std::string &expression) {
        LOG_MESSAGE("Specs", "ParseAndSerialize parsing %s\n", expression.c_str());
        // prepare
        const LanguageNodable lang;
        HeadlessNodeFactory factory(&lang);
        GraphNode graph(&lang, &factory);

        // act
        lang.getParser()->parse_graph(expression, &graph);
        if ( !graph.get_root())
        {
            throw std::runtime_error("ParseAndSerialize: Unable to generate program.");
        }

        Serializer *serializer = lang.getSerializer();
        std::string result;
        serializer->serialize(result, graph.get_root() );
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize serialize output is: %s\n", result.c_str());

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