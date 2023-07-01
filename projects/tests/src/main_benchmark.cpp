#include <benchmark/benchmark.h>
#include <ndbl/core/language/Nodlang.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/NodeFactory.h>
#include <fw/core/reflection/reflection>
#include <random>
#include <fw/core/string.h>

/*
 *
 * EXAMPLE:
 * --------

    #include <benchmark/benchmark.h>

    static void BM_SomeFunction(benchmark::State& state) {
      // Perform setup here
      for (auto _ : state) {
        // This code gets timed
        SomeFunction();
      }
    }
    // Register the function as a benchmark
    BENCHMARK(BM_SomeFunction);
    // Run the benchmark
    BENCHMARK_MAIN();

 */

using namespace ndbl;
using namespace fw;

class NodlangFixture : public benchmark::Fixture {
public:
    Nodlang* language;
    bool autocompletion;
    NodeFactory* factory;
    GraphNode* graph;
    std::random_device random_device;  // Will be used to obtain a seed for the random number engine
    std::mt19937 generator; // Standard mersenne_twister_engine
    std::uniform_real_distribution<double> distribution;

    NodlangFixture()
        : generator(random_device())
        , distribution(-100000.0, 100000.0)
    {}

    void SetUp(const ::benchmark::State& state)
    {
        language = &ndbl::Nodlang::get_instance();
        autocompletion = false;
        factory = new NodeFactory(language);
        graph = new GraphNode(language, factory, &autocompletion);
        log::set_verbosity(log::Verbosity_Error);
    }

    void TearDown(const ::benchmark::State& state)
    {
        delete factory;
        delete graph;
    }

    inline std::string get_random_double_as_string()
    {
        return std::to_string( distribution(generator) );
    }
};

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_double)(benchmark::State& state) {
    std::array<std::string, 500> double_as_str;
    for(size_t i = 0; i < double_as_str.size(); ++i)
    {
        double_as_str[i] = get_random_double_as_string();
    }
    size_t j = 0;
    for (auto _ : state)
    {
        language->parse_token(double_as_str[j%double_as_str.size()]);
        ++j;
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_boolean)(benchmark::State& state) {
    bool b = true;
    for (auto _ : state)
    {
        language->parse_token(b ? "true" : "false");
        b = !b;
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, std_string_append)(benchmark::State& state) {

    for (auto _ : state)
    {
        std::string str;
        for(int i = 0; i < 8; i++ )
            str.append("12345678");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, string_append)(benchmark::State& state) {

    for (auto _ : state)
    {
        string str;
        for(int i = 0; i < 8; i++ )
            str.append("12345678");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, string128_append_no_dynamic_alloc)(benchmark::State& state) {

    for (auto _ : state)
    {
        string128 str;
        for(int i = 0; i < 8; i++ )
            str.append("12345678");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, string64_append_some_dynamic_alloc)(benchmark::State& state) {

    for (auto _ : state)
    {
        string64 str;
        for(int i = 0; i < 8; i++ )
            str.append("12345678");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_operator)(benchmark::State& state) {
    std::array operations{
        "+",
        "-",
        "!",
        "!=",
        "==",
        ">",
        "<",
        ">=",
        "<=",
        "*=",
        "/=",
        "+=",
        "-=",
        "=>",
        "<=>"
    };

    size_t id = 0;
    for (auto _ : state)
    {
        language->parse_token( operations.at(id++ % operations.size()) );
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_char)(benchmark::State& state) {
    std::array chars{
       ",",
       " ",
       "\n",
       "\t",
       ";",
       "{",
       "}",
       "(",
       ")"
    };

    size_t id = 0;
    for (auto _ : state)
    {
        language->parse_token( chars.at(id++ % chars.size() ));
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, tokenize__some_code_to_graph)(benchmark::State& state) {
    std::string code = "double a = 10.400012;"
                       "double b = 5.564478;"
                       "if(a>b){"
                       " print(\"a>b\");"
                       "}else{"
                       " print(\"a<=b\");"
                       "}"
                       ;

    for (auto _ : state)
    {
        FW_EXPECT(language->tokenize(code), "parse failed");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_keyword)(benchmark::State& state) {

    std::array chars{
        "if",
        "else",
        "for",
        "operator",
        "int",
        "bool",
        "double",
        "string"
    };

    size_t id = 0;
    for (auto _ : state)
    {
        language->parse_token( chars.at(id++ % chars.size() ));
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_token__a_single_identifier_starting_with_a_keyword)(benchmark::State& state) {

    std::array chars{
            "iff",
            "elsee",
            "forr",
            "operatorr",
            "intt",
            "booll",
            "doublee",
            "stringg"
    };

    size_t id = 0;
    for (auto _ : state)
    {
        language->parse_token( chars.at(id++ % chars.size() ));
    }
}

BENCHMARK_REGISTER_F(NodlangFixture, tokenize__some_code_to_graph);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_operator);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_boolean);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_double);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_char);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_keyword);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_identifier_starting_with_a_keyword);
BENCHMARK_REGISTER_F(NodlangFixture, std_string_append);
BENCHMARK_REGISTER_F(NodlangFixture, string_append);
BENCHMARK_REGISTER_F(NodlangFixture, string128_append_no_dynamic_alloc);
BENCHMARK_REGISTER_F(NodlangFixture, string64_append_some_dynamic_alloc);

BENCHMARK_MAIN();