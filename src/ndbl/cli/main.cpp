#include "CLI.h"

using tools::log;
using namespace ndbl;

int main(int argc, char *argv[])
{
    // 1) configure tools related stuff
    //---------------------------------

    // Adjust lo levels
    tools::log::set_verbosity(log::Verbosity_Warning );

    // Declare CLI's methods into the reflection system
    tools::registration::push_class<CLI>("CLI")
        //                     vvv--- method     vvv--- keyword for the console
        .add_method(&CLI::test_concat_str  , "concat_str")
        .add_method(&CLI::test_return_str  , "return_str")
        .add_method(&CLI::clear            , "clear")
        .add_method(&CLI::help             , "help")
        .add_method(&CLI::exit_            , "exit")
        .add_method(&CLI::exit_            , "quit")
        .add_method(&CLI::parse            , "parse")
        .add_method(&CLI::serialize        , "serialize")
        .add_method(&CLI::compile          , "compile")
        .add_method(&CLI::set_verbose      , "set_verbose")
        .add_method(&CLI::print_program    , "print program" )
        .add_method(&CLI::run              , "run");

    // 2) Instantiate and run the app
    //-------------------------------
    ndbl::CLI cli;
    cli.init();
    while ( !cli.should_stop() )
    {
        cli.update();
    }
    cli.shutdown();
}
