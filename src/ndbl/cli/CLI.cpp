#include "CLI.h"

#include <iostream>

#include "ndbl/core/language/Nodlang.h"
#include "tools/core/TaskManager.h"
#include "tools/core/reflection/reflection"

using namespace ndbl;
using namespace tools;

void CLI::init()
{
    NodableHeadless::init();
    std::cout << R"(== Nodable command line interface ==)" << std::endl <<
            R"(Nodable Copyright (C) 2023-2024 Bérenger DALLE-CORT. This program comes with ABSOLUTELY NO WARRANTY. )"
            R"(This is free software, and you are welcome to redistribute it under certain conditions.)"
              << std::endl << R"(Feel lost? type "help".)" << std::endl;

    // Declare CLI::PublicApi's methods into the reflection system
    using API = CLI::PublicApi;
    tools::StaticInitializer<API>("PublicAPI")
        //                 vvv---- method     vvvvv--- alias
        .add_method(&API::clear            , "clear")
        .add_method(&API::help             , "help")
        .add_method(&API::quit             , "exit")
        .add_method(&API::quit             , "quit")
        .add_method(&API::parse            , "parse")
        .add_method(&API::serialize        , "serialize")
        .add_method(&API::compile          , "compile")
        .add_method(&API::set_verbose      , "set_verbose")
        .add_method(&API::print_program    , "print program" )
        .add_method(&API::run              , "run");
}

void CLI::shutdown()
{
    // TODO: implement tools::registration::pop_class<CLI::PublicApi>
    NodableHeadless::shutdown();
    std::cout << "Good bye!" << std::endl;
}

void CLI::update()
{
    // command prompt
    std::cout << ">>> ";

    // get user input
    std::string user_input;
    while ( user_input.empty())
    {
        user_input = get_line();
    }

    // Priority 1: call a static function immediately
    const ClassDesc* api_class = type::get_class<PublicApi>();
    if( auto static_fct = api_class->get_static(user_input.c_str()) )
    {
        try
        {
            variant result = static_fct->invoke({});
        }
        catch (std::runtime_error& e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // Priority 2: try to call a CLI method immediately
    if( auto method = api_class->get_method(user_input.c_str()) )
    {
        try
        {
            // then we invoke it
            variant result = method->invoke(&api, {});
        }
        catch (std::runtime_error& e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // Priority 3: append to source code, parse, compile, and run the code;
    get_language()->serialize_token_t(user_input, Token_t::end_of_instruction);
    m_source_code.append(user_input);
    parse(m_source_code) &&
    compile() &&
    run();
}

void CLI::clear()
{
    System::clear_console();
    NodableHeadless::clear();
}

void CLI::log_function_call(const variant &result, const FuncType *type)
{
    LOG_MESSAGE("CLI", "CLI::%s() done (result: %s)\n",type->get_identifier(), result.to<std::string>().c_str())
}

std::string CLI::get_line()
{
    char input_buffer[256];
    std::cin.getline (input_buffer,256);
    std::string input = input_buffer;
    return input;
}

void CLI::PublicApi::quit()
{
    m_cli->m_should_stop = true;
}

bool CLI::PublicApi::serialize()
{
    if( m_cli->get_graph()->get_root() )
    {
        std::string result;
        m_cli->serialize( result );
        std::cout << result << std::endl;
        return true;
    }

    LOG_WARNING("CLI", "unable to serialize! Are you sure you entered an expression earlier?\n")
    return false;
}

bool CLI::PublicApi::compile()
{
    if( m_cli->compile() == nullptr)
    {
        LOG_ERROR("CLI", "unable to compile!\n")
        return false;
    }
    return true;
}

void CLI::PublicApi::set_verbose()
{
    printf("Verbose mode ON\n");
    log::set_verbosity(log::Verbosity_Verbose);
}

int CLI::PublicApi::print_program()
{
    std::string source_code = m_cli->get_source_code(); // TODO: crash here because the reflection system does not handle virtuals?
    if( source_code.empty() )
    {
        return printf("The current program is empty.\n");
    }
    return printf("Program:\n%s\n", source_code.c_str());
}

bool CLI::PublicApi::parse()
{
    // ask for user input
    std::cout << ">>> ";
    std::string parse_in = get_line();
    Graph* graph = m_cli->parse(parse_in);
    return graph;
}

void CLI::PublicApi::help()
{
    std::vector<std::string> command_names;

    const ClassDesc* api_class = type::get_class<PublicApi>();

    for ( const IInvokable* invokable : api_class->get_statics() )
    {
        std::string identifier = invokable->get_sig()->get_identifier();
        command_names.emplace_back( identifier + " (static)");
    }

    for ( const IInvokableMethod* invokable : api_class->get_methods() )
    {
        std::string identifier = invokable->get_sig()->get_identifier();
        command_names.emplace_back( identifier );
    }

    std::sort(command_names.begin(), command_names.end());

    std::cout << "Command list:" << std::endl;
    for ( const auto& each : command_names )
    {
        std::cout << "  o " << each << std::endl;
    }
}

void CLI::PublicApi::clear()
{

}

bool CLI::run()
{
    if( m_asm_code == nullptr )
    {
        return false;
    }

    if( !load_program(m_asm_code) )
    {
        LOG_ERROR("CLI", "Unable to run program!\n")
        return false;
    }

    if( !run_program() )
    {
        LOG_ERROR("CLI", "Unable to run program!\n")
        return false;
    }

    return release_program();
}

bool CLI::PublicApi::run()
{
    return m_cli->run();
}

variant CLI::invoke_static(const FuncType* _func_type, std::vector<variant>&& _args) const
{
    variant result;

    VERIFY(false, "not implemented yet")

    log_function_call(result, _func_type);
    return result;
}

variant CLI::invoke_method(const FuncType* _func_type, std::vector<variant>&& _args) const
{
    variant result;

    VERIFY(false, "not implemented yet")

    log_function_call(result, _func_type);
    return result;
}
