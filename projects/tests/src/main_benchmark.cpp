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

class Fixture : public benchmark::Fixture {
public:
    Nodlang* language;
    bool autocompletion;
    NodeFactory* factory;
    GraphNode* graph;
    std::random_device random_device;  // Will be used to obtain a seed for the random number engine
    std::mt19937 generator; // Standard mersenne_twister_engine
    std::uniform_real_distribution<double> distribution;

    Fixture()
        : generator(random_device())
        , distribution(
            -100000.0,
            100000.0
        )
    {
    }

    void SetUp(const ::benchmark::State& state) {
        language = &ndbl::Nodlang::get_instance();
        autocompletion = false;
        factory = new NodeFactory(language);
        graph = new GraphNode(language, factory, &autocompletion);
        fw::log::set_verbosity(fw::log::Verbosity_Warning);
    }

    void TearDown(const ::benchmark::State& state) {
        delete factory;
        delete graph;
    }

    inline std::string get_random_double_as_string() {
        return std::to_string( distribution(generator) );
    }
};

BENCHMARK_DEFINE_F(Fixture, BM_ParserMethod_REGEX)(benchmark::State& state) {
    language->set_parser_method(ParserMethod::REGEX);

    for (auto _ : state) {
        language->parse(get_random_double_as_string(), graph);
    }
}

BENCHMARK_DEFINE_F(Fixture, BM_ParserMethod_NOREGEX_IF_POSSIBLE)(benchmark::State& state) {
    language->set_parser_method(ParserMethod::NOREGEX_IF_POSSIBLE);
    for (auto _ : state) {
        language->parse(get_random_double_as_string(), graph);
    }
}

BENCHMARK_DEFINE_F(Fixture, BM_ParserMethod_NOREGEX)(benchmark::State& state) {
    language->set_parser_method(ParserMethod::NOREGEX);

    for (auto _ : state) {
        language->parse(get_random_double_as_string(), graph);
    }
}

BENCHMARK_REGISTER_F(Fixture, BM_ParserMethod_NOREGEX);
BENCHMARK_REGISTER_F(Fixture, BM_ParserMethod_REGEX);

BENCHMARK_MAIN();