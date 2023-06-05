#include <benchmark/benchmark.h>
#include <ndbl/core/language/Nodlang.h>
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/NodeFactory.h>
#include <fw/core/reflection/reflection>
#include <random>

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
        fw::log::set_verbosity(fw::log::Verbosity_Error);
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
    for (auto _ : state)
    {
        language->parse_token(get_random_double_as_string());
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

BENCHMARK_DEFINE_F(NodlangFixture, parse__some_code_to_graph)(benchmark::State& state) {
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
        FW_EXPECT(language->parse(code, graph ), "parse failed");
    }
}

BENCHMARK_REGISTER_F(NodlangFixture, parse__some_code_to_graph)->Unit(benchmark::TimeUnit::kMicrosecond);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_operator)->Unit(benchmark::TimeUnit::kNanosecond);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_boolean)->Unit(benchmark::TimeUnit::kNanosecond);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_double)->Unit(benchmark::TimeUnit::kNanosecond);
BENCHMARK_REGISTER_F(NodlangFixture, parse_token__a_single_char)->Unit(benchmark::TimeUnit::kNanosecond);

BENCHMARK_MAIN();