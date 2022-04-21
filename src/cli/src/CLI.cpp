//
// Created by Bérenger Dalle-Cort on 4/21/22.
//

#include <iostream>

#include <nodable/cli/CLI.h>
#include <nodable/core/languages/NodableLanguage.h>
#include <nodable/core/reflection/reflection>

using namespace Nodable;

REGISTER
{
    registration::push_class<CLI>("CLI")
            .add_static(&CLI::log_stats   , "log_stats")
            .add_static(&CLI::help        , "help");
};

CLI::CLI()
    : m_should_stop(false)
    , m_language(std::make_unique<NodableLanguage>() )
    , m_factory(m_language.get())
    , m_graph(m_language.get(), &m_factory, &m_auto_completion)
{
    std::cout << "o-------------------------------------------o" << std::endl;
    std::cout << "| Welcome to Nodable command line interface |" << std::endl;
    std::cout << "o-------------------------------------------o" << std::endl;
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
    std::cin >> input;

    if( !input.empty() )
    {
        type api = type::get<CLI>();
        if( auto invokable_ = api.get_static(input) )
        {
            variant ok;
            invokable_->invoke(&ok);
        }
        else if ( input == "exit")
        {
            m_should_stop = true;
        }
        else if ( m_language->get_parser().parse(input, &m_graph))
        {
            std::cout << "Graph parsed: \"" << input << '"' << std::endl;
        }
        else
        {
            std::cout << "Command not found: \"" << input << '"' << std::endl;
            help();
        }
    }

    // <----- TODO
}

bool CLI::help()
{
    std::cout << "Command list:" << std::endl;
    type api = type::get<CLI>();
    for(auto static_ : api.get_static_methods() )
    {
        std::cout << "  - " << static_->get_type().get_identifier() << std::endl;
    }
    return true;
}

bool CLI::log_stats()
{
    Nodable::type_register::log_statistics();
    return true;
}
