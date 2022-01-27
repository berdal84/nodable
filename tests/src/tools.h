#pragma once
#include <gtest/gtest.h>
#include <exception>

#include <nodable/Member.h>
#include <nodable/Runner.h>
#include <nodable/GraphNode.h>
#include <nodable/Parser.h>
#include <nodable/LanguageFactory.h>
#include <nodable/VariableNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/HeadlessNodeFactory.h>

namespace Nodable
{
    template<typename expected_result_t>
    static expected_result_t ParseAndEvalExpression(const std::string &expression) {
        // prepare
        expected_result_t result{};
        const Language *lang = LanguageFactory::GetNodable();
        HeadlessNodeFactory factory(lang);
        GraphNode graph(lang, &factory);

        // create program
        lang->getParser()->source_code_to_graph(expression, &graph);

        auto program = graph.getProgram();
        if (program) {
            // run
            Runner runner;

            if (runner.load_program(graph.getProgram())) {
                runner.run_program();
            } else {
                throw std::runtime_error("Unable to load program.");
            }


            // compare result
            Member* last_eval = runner.get_last_eval();
            EXPECT_TRUE(last_eval);
            if (last_eval) {
                result = last_eval->convert_to<expected_result_t>();
            } else {
                throw std::runtime_error("Unable to get last instruction.");
            }
        } else {
            throw std::runtime_error("Unable to convert expression to graph, program is nullptr.");
        }

        return result;
    }


    static std::string &ParseUpdateSerialize(std::string &result, const std::string &expression) {
        LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize parsing %s\n", expression.c_str());
        // prepare
        const Language *lang = LanguageFactory::GetNodable();
        HeadlessNodeFactory factory(lang);
        GraphNode graph(lang, &factory);

        // act
        lang->getParser()->source_code_to_graph(expression, &graph);
        if (ScopedCodeBlockNode *program = graph.getProgram()) {
            Runner runner;

            if (runner.load_program(program)) {
                runner.run_program();

                if (auto last_eval = runner.get_last_eval()) {
                    std::string result_str;
                    lang->getSerializer()->serialize(result_str, last_eval);
                    LOG_MESSAGE("Parser.specs", "ParseUpdateSerialize result is: %s\n", result_str.c_str());
                } else {
                    throw std::runtime_error("ParseUpdateSerialize: Unable to get last evaluated member.");
                }
            } else {
                throw std::runtime_error("ParseUpdateSerialize: Unable to load program.");
            }

        } else {
            throw std::runtime_error("ParseUpdateSerialize: Unable to generate program.");
        }

        Serializer *serializer = lang->getSerializer();
        serializer->serialize(result, graph.getProgram());
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