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
        fw::log::set_verbosity(fw::log::Verbosity_Error);
    }

    void TearDown(const ::benchmark::State& state) {
        delete factory;
        delete graph;
    }

    inline std::string get_random_double_as_string() {
        return std::to_string( distribution(generator) );
    }
};

BENCHMARK_DEFINE_F(NodlangFixture, parse_double)(benchmark::State& state) {
    auto method = (ParserMethod)state.range(0);
    language->set_parser_method(method);
    state.SetLabel(to_string(method));

    for (auto _ : state) {
        FW_EXPECT(language->parse(get_random_double_as_string(), graph), "parse failed");
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_boolean)(benchmark::State& state) {
    auto method = (ParserMethod)state.range(0);
    language->set_parser_method(method);
    state.SetLabel(to_string(method));

    bool b = true;
    for (auto _ : state) {
        FW_EXPECT(language->parse( b ? "true" : "false", graph), "parse failed");
        b = !b;
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_operators)(benchmark::State& state) {
    auto method = (ParserMethod)state.range(0);
    language->set_parser_method(method);
    state.SetLabel(to_string(method));

    std::array operations{
        "1+1",
        "1-1",
        "1!=1",
        "1==1",
        "1>1",
        "1<1",
        "1>=1",
        "1<=1",
        "1*=1",
        "1/=1",
        "1+=1",
        "1-=1",
        "1=>1",
        "1<=>1"
    };

    size_t id = 0;
    for (auto _ : state) {
        FW_EXPECT(language->parse(operations.at(id), graph ), "parse failed");
        id = (id+1) % operations.size();
    }
}

BENCHMARK_DEFINE_F(NodlangFixture, parse_code)(benchmark::State& state) {
    auto method = (ParserMethod)state.range(0);
    language->set_parser_method(method);
    state.SetLabel(to_string(method));

    std::string code = "double a = 10.400012;"
                       "double b = 5.564478;"
                       "if(a>b){"
                       " print(\"a>b\");"
                       "}else{"
                       " print(\"a<=b\");"
                       "}"
                       ;

    for (auto _ : state) {
        FW_EXPECT(language->parse(code, graph ), "parse failed");
    }
}

BENCHMARK_REGISTER_F(NodlangFixture, parse_code)
        ->Args({(int)ParserMethod::REGEX_ONLY})
        ->Args({(int)ParserMethod::NO_REGEX_ONLY});

BENCHMARK_REGISTER_F(NodlangFixture, parse_operators)
    ->Args({(int)ParserMethod::REGEX_ONLY})
    ->Args({(int)ParserMethod::NO_REGEX_ONLY});

BENCHMARK_REGISTER_F(NodlangFixture, parse_boolean)
    ->Args({(int)ParserMethod::REGEX_ONLY})
    ->Args({(int)ParserMethod::NO_REGEX_ONLY});

BENCHMARK_REGISTER_F(NodlangFixture, parse_double)
    ->Args({(int)ParserMethod::REGEX_ONLY})
    ->Args({(int)ParserMethod::NO_REGEX_ONLY});

BENCHMARK_MAIN();