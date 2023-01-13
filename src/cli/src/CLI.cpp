//
// Created by Bérenger Dalle-Cort on 4/21/22.
//

#include <iostream>

#include <nodable/cli/CLI.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/reflection/reflection>
#include <iomanip>

using namespace ndbl;

REGISTER
{
    registration::push_class<CLI>("CLI")
        .add_method(&CLI::test_concat_str  , "concat_str")
        .add_method(&CLI::test_return_str  , "return_str")
        .add_method(&CLI::clear            , "clear")
        .add_method(&CLI::help             , "help")
        .add_method(&CLI::exit_            , "exit")
        .add_method(&CLI::parse            , "parse")
        .add_method(&CLI::serialize        , "serialize")
        .add_method(&CLI::compile          , "compile")
        .add_method(&CLI::run              , "run");
};

CLI::CLI()
    : m_should_stop(false)
    , m_language(std::make_unique<NodableLanguage>() )
    , m_factory(m_language.get())
    , m_graph(m_language.get(), &m_factory, &m_auto_completion)
{
    std::cout << R"(== Nodable command line interface ==)" << std::endl <<
                 R"(Nodable Copyright (C) 2023 Bérenger DALLE-CORT. This program comes with ABSOLUTELY NO WARRANTY. )"
                 R"(This is free software, and you are welcome to redistribute it under certain conditions.)"
            << std::endl << R"(Feel lost? type "help".)" << std::endl;
    Log::set_verbosity(ndbl::Log::Verbosity_Warning);
}

CLI::~CLI()
{
    std::cout << "Good bye!" << std::endl;
}

bool CLI::should_stop() const
{
    return m_should_stop;
}

void CLI::update()
/*
 * TODO:
 * - we must differenciate current graph and program state (should be in VirtualMachine)
 * - user input should be parsed, compiled and executed in the existing program state.
 * - VirtualMachine should return a result after each execution.
 */
{
    // command prompt
    std::cout << ">>> ";

    // get user input
    std::string input;
    while ( input.empty())
    {
        input = get_line();
    }

    // get static function from user input
    type api = type::get<CLI>();
    if( auto static_fct = api.get_static(input) )
    {
        try
        {
            // invoke the function
            variant result = (*static_fct)();
            log_function_call(result, static_fct->get_type());
        }
        catch (std::runtime_error e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // no static found earlier, we try to get a method from user input
    if( auto method = api.get_method(input) )
    {
        try
        {
            // then we invoke it
            variant result = (*method)((void *) this);
            log_function_call(result, method->get_type());
        }
        catch (std::runtime_error e )
        {
            LOG_ERROR("CLI", "Error: %s\n", e.what() );
        }
        return;
    }

    // try to eval (parse, compile and run).
    m_language->get_parser().parse(input, &m_graph) && compile() && run();
}
void CLI::log_function_call(const variant &result, const func_type &type) const {LOG_MESSAGE("CLI", "CLI::%s() done (result: %s)\n", type.get_identifier().c_str(), result.is_defined() ? result.convert_to<std::string>().c_str() : "void")}

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
    if(Node* root = m_graph.get_root())
    {
        std::string result;
        m_language->get_serializer().serialize(result, root);
        std::cout << result << std::endl;
        return true;
    }

    LOG_WARNING("CLI", "unable to serialize! Are you sure you entered an expression earlier?\n")
    return false;
}

bool CLI::compile()
{
    if( auto asm_code = m_compiler.compile_syntax_tree(&m_graph))
    {
        m_asm_code = std::move(asm_code);
        return m_asm_code.get();
    }
    else
    {
        LOG_ERROR("CLI", "unable to compile!\n")
        return false;
    }
}

bool CLI::parse()
{
    // ask for user input
    std::cout << ">>> ";
    std::string parse_in = get_line();
    return m_language->get_parser().parse(parse_in, &m_graph);
}

bool CLI::run()
{
    if(!m_asm_code)
    {
        return false;
    }

    if( m_virtual_machine.load_program(std::move(m_asm_code)) )
    {
        m_virtual_machine.run_program();
        qword last_result = m_virtual_machine.get_last_result();

        std::cout << "Result in various types:";
        std::cout << std::endl;
        std::cout << " bool:   " << std::setw(12) << std::boolalpha << (bool)last_result;
        std::cout << " | double: " << std::setw(12) << (double)last_result;
        std::cout << " | i16_t:  " << std::setw(12) << (i16_t)last_result;
        std::cout << " | hex:    " << std::setw(12) << last_result.to_string() << std::endl;

        auto program = m_virtual_machine.release_program();
        return program.get();
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

    type api = type::get<CLI>();

    for(auto each : api.get_static_methods() )
    {
        command_names.push_back( each->get_type().get_identifier() + " (static)" );
    }

    for(auto each : api.get_methods() )
    {
        command_names.push_back( each->get_type().get_identifier());
    }

    std::sort(command_names.begin(), command_names.end());

    std::cout << "Command list:" << std::endl;
    for(auto each : command_names )
    {
        std::cout << "  o " << each << std::endl;
    }
}

void CLI::clear()
{
    System::console::clear();
}
