#include "Nodable_Headless.h"
#include "core/Graph.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/Task_Manager.h"

using namespace ndbl;

void Nodable_Headless::init()
{
    // init managers
    m_task_manager    = tools::init_task_manager();
    m_language        = init_language();

    // configure
    m_graph = new Graph();
    graph_init(m_graph);
    m_language->_state.reset( m_graph ); // in some cases (like during tests), we call parse_xxx methods that implicitly requires the state to be reset
}

void Nodable_Headless::shutdown()
{
    ASSERT(m_graph);
    clear();
    graph_shutdown(m_graph);
    delete m_graph;
    tools::shutdown_task_manager(m_task_manager);
    shutdown_language(m_language);
}

std::string& Nodable_Headless::serialize( std::string& out ) const
{
    return m_language->serialize_graph(out, m_graph);
}

Graph* Nodable_Headless::parse( const std::string& code )
{
    m_language->parse( m_graph, code );
    return m_graph;
}

Nodlang* Nodable_Headless::get_language() const
{
    return m_language;
}

Graph* Nodable_Headless::graph() const
{
    return m_graph;
}

void Nodable_Headless::update()
{
}

void Nodable_Headless::clear()
{
    graph_reset(m_graph);
    m_source_code.clear();
}

bool Nodable_Headless::should_stop() const
{
    return m_should_stop;
}