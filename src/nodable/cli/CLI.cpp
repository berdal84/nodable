#include "CLI.h"

#include <iostream>
#include <iomanip>

#include "fw/core/reflection/reflection"

#include "core/language/Nodlang.h"

using namespace ndbl;
using namespace fw;

REGISTER
{
    registration::push_class<CLI>("CLI")
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
};

CLI::CLI()
    : m_should_stop(false)
    , m_asm_code(nullptr)
    , m_graph(&m_factory)
{
    std::cout << R"(== Nodable command line interface ==)" << std::endl <<
                 R"(Nodable Copyright (C) 2023-2024 Bérenger DALLE-CORT. This program comes with ABSOLUTELY NO WARRANTY. )"
                 R"(This is free software, and you are welcome to redistribute it under certain conditions.)"
            << std::endl << R"(Feel lost? type "help".)" << std::endl;
}

CLI::~CLI()
{
    std::cout << "Good bye!" << std::endl;
    delete m_asm_code;
 }



int CLI::main(int argc, char* argv[])
{
    log::set_verbosity(log::Verbosity_Warning);
    Pool::init();

    while (!should_stop())
    {
        update();
    }

    m_graph.clear();
    Pool::shutdown();
    LOG_FLUSH()
    return 0;
}

bool CLI::should_stop() const
{
    return m_should_stop;
}

void CLI::update()
/*
 * TODO:
 * - we must differentiate current graph and program state (should be in VirtualMachine)
 */
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
    const type* cli_type = type::get<CLI>();
    if( auto static_fct = cli_type->get_static(user_input) )
    {
        try
        {
            variant result = static_fct->invoke();
            log_function_call(result, static_fct->get_type());
        }
        catch (std::runtime_error e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // Priority 2: try to call a CLI method immediately
    if( auto method = cli_type->get_method(user_input) )
    {
        try
        {
            // then we invoke it
            variant result = method->invoke((void*)this);
            log_function_call(result, method->get_type());
        }
        catch (std::runtime_error e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // Priority 3: append to source code, parse, compile, and run the code;
    m_language.serialize_token_t(user_input, Token_t::end_of_instruction);
    m_source_code.append(user_input);
    m_language.parse(m_source_code, &m_graph) && compile() && run();
}

void CLI::log_function_call(const variant &result, const func_type *type) const
{
    LOG_MESSAGE("CLI",
                "CLI::%s() done (result: %s)\n",
                type->get_identifier().c_str(),
                result.is_defined() ? result.to<std::string>().c_str() : "void"
                )
}

std::string CLI::get_word() const
{
    std::string str;
    std::cin >> str;
    return str;
}

std::string CLI::get_line() const
{
    char input_buffer[256];
    std::cin.getline (input_buffer,256);
    std::string input = input_buffer;
    return input;
}

void CLI::exit_()
{
    m_should_stop = true;
}

bool CLI::serialize()
{
    if( PoolID<Node> root = m_graph.get_root())
    {
        std::string result;
        m_language.serialize_node( result, root );
        std::cout << result << std::endl;
        return true;
    }

    LOG_WARNING("CLI", "unable to serialize! Are you sure you entered an expression earlier?\n")
    return false;
}

bool CLI::compile()
{
    m_asm_code = m_compiler.compile_syntax_tree(&m_graph);
    if(!m_asm_code)
    {
        LOG_ERROR("CLI", "unable to compile!\n")
        return false;
    }
    return true;
}

void CLI::set_verbose()
{
    printf("Verbose mode ON\n");
    log::set_verbosity(log::Verbosity_Verbose);
}

int CLI::print_program()
{
    if( m_source_code.empty() )
    {
        return printf("The current program is empty.\n");
    }
    return printf("Program:\n%s\n", m_source_code.c_str());
}

bool CLI::parse()
{
    // ask for user input
    std::cout << ">>> ";
    std::string parse_in = get_line();
    return m_language.parse(parse_in, &m_graph);
}

bool CLI::run()
{
    if(!m_asm_code)
    {
        return false;
    }

    if( m_virtual_machine.load_program(m_asm_code) )
    {
        m_virtual_machine.run_program();
        qword last_result = m_virtual_machine.get_last_result();

        printf( "bool: %s | int: %12f | double: %12d | hex: %12s\n"
           , (bool)last_result ? "true" : "false"
           , (double)last_result
           , (i16_t)last_result
           , last_result.to_string().c_str()
        );

        return m_virtual_machine.release_program();
    }
    else
    {
        LOG_ERROR("CLI", "Unable to run program!\n")
        return false;
    }
}

void CLI::help()
{
    std::vector<std::string> command_names;

    const type* cli_type = type::get<CLI>();

    for ( auto static_method_type : cli_type->get_static_methods() )
    {
        command_names.push_back(static_method_type->get_type()->get_identifier() + " (static)" );
    }

    for ( auto method_type : cli_type->get_methods() )
    {
        command_names.push_back(method_type->get_type()->get_identifier());
    }

    std::sort(command_names.begin(), command_names.end());

    std::cout << "Command list:" << std::endl;
    for ( auto each : command_names )
    {
        std::cout << "  o " << each << std::endl;
    }
}

void CLI::clear()
{
    m_source_code.clear();
    system::clear_console();
    delete m_asm_code;
    m_graph.clear();
    m_virtual_machine.release_program();
}
