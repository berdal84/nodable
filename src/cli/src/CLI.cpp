//
// Created by Bérenger Dalle-Cort on 4/21/22.
//

#include <iostream>

#include <nodable/cli/CLI.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/reflection/reflection>
#include <iomanip>

using namespace Nodable;

REGISTER
{
    registration::push_class<CLI>("CLI")
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
                 R"(Nodable Copyright (C) 2022 Bérenger DALLE-CORT. This program comes with ABSOLUTELY NO WARRANTY. )"
                 R"(This is free software, and you are welcome to redistribute it under certain conditions.)"
            << std::endl << R"(Feel lost? type "help".)" << std::endl;
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

    // ask for user input
    std::string input;

    while ( input.empty())
    {
        input = get_line();
    }

    type api = type::get<CLI>();
    if( auto static_method = api.get_static(input) )
    {
        variant ok;
        static_method->invoke(&ok); // TODO:: we should not be forced to pass a result reference, what about "void" cases?
        return;
    }

    if( auto method = api.get_method(input) )
    {
        method->invoke((void*)this);
        return;
    }

    std::cout << "Command not found: \"" << input << '"' << std::endl;
    help();

    // <----- TODO
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
    if(Node* root = m_graph.get_root())
    {
        std::string result;
        m_language->get_serializer().serialize(result, root);
        std::cout << result << std::endl;
        return true;
    }

    LOG_WARNING("cli", "unable to serialize! Are you sure you entered an expression earlier?\n")
    return false;
}

void CLI::compile()
{
    if( auto asm_code = m_compiler.compile_syntax_tree(&m_graph))
    {
        m_asm_code = std::move(asm_code);
    }
    else
    {
        LOG_ERROR("cli", "unable to compile! Are you sure you entered an expression earlier?\n")
    }
}

void CLI::parse()
{
    // ask for user input
    std::cout << "Type an expression:" << std::endl;
    std::cout << ">>> ";
    std::string parse_in = get_line();
    m_language->get_parser().parse(parse_in, &m_graph);
}

void CLI::run()
{
    if( m_asm_code)
    {
        if( m_virtual_machine.load_program(std::move(m_asm_code)) )
        {
            m_virtual_machine.run_program();
            qword last_result = m_virtual_machine.get_last_result();

            std::cout << "Result in various types:";
            std::cout << std::endl;
            std::cout << " bool:    " << std::setw(12) << (bool)last_result;
            std::cout << ", double: " << std::setw(12) << (double)last_result;
            std::cout << ", i16_t:  " << std::setw(12) << (i16_t)last_result;
            std::cout << ", hexa:   " << std::setw(12) << last_result.to_string();
            std::cout << std::endl;

            m_virtual_machine.release_program();
        }
        else
        {
            LOG_ERROR("cli", "unable to run program!\n")
        }
    }
    else
    {
        LOG_ERROR("cli", "compile program first!\n")
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
