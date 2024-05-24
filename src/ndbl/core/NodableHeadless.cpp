#include "NodableHeadless.h"
#include "VirtualMachine.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/async.h"

using namespace ndbl;

NodableHeadless::NodableHeadless()
: graph( nullptr)
, compiler()
{
}

void NodableHeadless::init()
{
    tools::init_pool_manager();
    tools::init_task_manager();
    ndbl::init_language();
    ndbl::init_node_factory();
    ndbl::init_virtual_machine();
    graph = new Graph(get_node_factory());
}

void NodableHeadless::shutdown()
{
    delete graph;
    tools::shutdown_pool_manager();
    tools::shutdown_task_manager();
    ndbl::shutdown_language();
    ndbl::shutdown_node_factory();
    ndbl::shutdown_virtual_machine();
}

std::string& NodableHeadless::serialize( std::string& out ) const
{
    return get_language()->serialize_node( out, graph->get_root() );
}

Graph* NodableHeadless::parse( const std::string& code )
{
    get_language()->parse(code, graph);
    return graph;
}

void NodableHeadless::run_program() const
{
    get_virtual_machine()->run_program();
}

const Code* NodableHeadless::compile(Graph* _graph)
{
    return compiler.compile_syntax_tree(_graph);
}

bool NodableHeadless::load_program(const Code* code)
{
    return get_virtual_machine()->load_program(code);
}

Nodlang* NodableHeadless::get_language() const
{
    return ::get_language();
}

Graph* NodableHeadless::get_graph() const
{
    return graph;
}

bool NodableHeadless::release_program()
{
    return get_virtual_machine()->release_program();
}

tools::qword NodableHeadless::get_last_result() const
{
    return get_virtual_machine()->get_last_result();
}

void NodableHeadless::update()
{
}
