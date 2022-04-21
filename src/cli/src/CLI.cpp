//
// Created by Bérenger Dalle-Cort on 4/21/22.
//

#include <iostream>

#include <nodable/cli/CLI.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/reflection/reflection>
#include <iomanip>

using namespace Nodable;

static const std::string k_cmd_compile   = "compile";
static const std::string k_cmd_serialize = "serialize";
static const std::string k_cmd_exit      = "exit";
static const std::string k_cmd_run       = "run";
static const std::string k_cmd_parse     = "parse";

REGISTER
{
    registration::push_class<CLI>("CLI")
            .add_static(&CLI::reflection_stats , "reflection_stats")
            .add_static(&CLI::help             , "help");
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
    std::string input = get_line();

    if( !input.empty() )
    {
        type api = type::get<CLI>();
        if( auto invokable_ = api.get_static(input) )
        {
            variant ok;
            invokable_->invoke(&ok);
        }
        else if (input == k_cmd_exit)
        {
            m_should_stop = true;
        }
        else if (input == k_cmd_serialize)
        {
            if(Node* root = m_graph.get_root())
            {
                std::string result;
                m_language->get_serializer().serialize(result, root);
                std::cout << result << std::endl;
            }
            else
            {
                LOG_WARNING("cli", "unable to serialize! Are you sure you entered an expression earlier?\n")
            }
        }
        else if (input == k_cmd_compile)
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
        else if (input == k_cmd_run)
        {
            if( m_asm_code)
            {
                if( m_virtual_machine.load_program(std::move(m_asm_code)) )
                {
                    m_virtual_machine.run_program();
                    qword last_result = m_virtual_machine.get_last_result();

                    std::cout << "Result in various types:";
                    std::cout << std::endl;
                    std::cout << " bool:   " << std::setw(12) << (bool)last_result;
                    std::cout << ", double: " << std::setw(12) << (double)last_result;
                    std::cout << ", i16_t:  " << std::setw(12) << (i16_t)last_result;
                    std::cout << ", hexa:   " << std::setw(12) << last_result.to_string();
                    std::cout << std::endl;
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
        else if (input == k_cmd_parse)
        {
            // ask for user input
            std::cout << "Type an expression:" << std::endl;
            std::cout << ">>> ";
            std::string parse_in = get_line();
            m_language->get_parser().parse(parse_in, &m_graph);
        }
        else
        {
            std::cout << "Command not found: \"" << input << '"' << std::endl;
            help();
        }
    }

    // <----- TODO
}

std::string CLI::get_line() const {
    char input_buffer[256];
    std::cin.getline (input_buffer,256);
    std::string input = input_buffer;
    return input;
}

bool CLI::help()
{
    std::vector<std::string> command_names = {k_cmd_exit, k_cmd_compile, k_cmd_serialize, k_cmd_run, k_cmd_parse}; // TODO: reflect non static methods

    type api = type::get<CLI>();
    for(auto each : api.get_static_methods() )
    {
        command_names.push_back( each->get_type().get_identifier() );
    }

    std::sort(command_names.begin(), command_names.end());

    std::cout << "Command list:" << std::endl;
    for(auto each : command_names )
    {
        std::cout << "  o " << each << std::endl;
    }
    return true;
}

bool CLI::reflection_stats()
{
    Nodable::type_register::log_statistics();
    return true;
}
