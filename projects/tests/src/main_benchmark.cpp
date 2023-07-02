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

template<class StringT>
static void BM_empty_constructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT str;
        benchmark::DoNotOptimize(str);
    }
}

const char* SIXTY_THREE_CHARS = "|<------------------------ 63 chars ------------------------->|";

template<class StringT>
static void BM_63chars_constructor(benchmark::State& state)
{
    for (auto _ : state) StringT str{SIXTY_THREE_CHARS};
}

template<class StringT>
static void BM_63chars_constructor_then_append_char(benchmark::State& state)
{
    for (auto _ : state)
    {
        StringT str{SIXTY_THREE_CHARS}; // reach 50% capacity
        str.append("+");
    }
}

static void BM_strncpy(benchmark::State& state)
{
    char source[ 1 << 16];
    while (state.KeepRunning())
    {
        char destination[ 1 << 16];
        strncpy(destination, source, state.range(0));
        benchmark::DoNotOptimize(destination);
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

BENCHMARK(BM_empty_constructor<std::string>);
BENCHMARK(BM_empty_constructor<string>);
BENCHMARK(BM_empty_constructor<string64>);
BENCHMARK(BM_empty_constructor<string128>);

BENCHMARK(BM_63chars_constructor<std::string>);
BENCHMARK(BM_63chars_constructor<string>);
BENCHMARK(BM_63chars_constructor<string64>);
BENCHMARK(BM_63chars_constructor<string128>);

BENCHMARK(BM_63chars_constructor_then_append_char<std::string>);
BENCHMARK(BM_63chars_constructor_then_append_char<string>);
BENCHMARK(BM_63chars_constructor_then_append_char<string64>);
BENCHMARK(BM_63chars_constructor_then_append_char<string128>);

BENCHMARK(BM_strncpy)->Ranges({{1 << 2, 1 << 8}});

BENCHMARK_MAIN();