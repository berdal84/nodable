#include "NodableHeadless.h"
#include "VirtualMachine.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/async.h"

using namespace ndbl;

void NodableHeadless::init()
{
    tools::init_pool_manager();
    tools::init_task_manager();
    ndbl::init_language();
    ndbl::init_node_factory();
    ndbl::init_virtual_machine();
    m_graph = new Graph(get_node_factory());
}

void NodableHeadless::shutdown()
{
    delete m_graph;
    tools::shutdown_pool_manager();
    tools::shutdown_task_manager();
    ndbl::shutdown_language();
    ndbl::shutdown_node_factory();
    ndbl::shutdown_virtual_machine();
}

std::string& NodableHeadless::serialize( std::string& out ) const
{
    return get_language()->serialize_node( out, m_graph->get_root() );
}

Graph* NodableHeadless::parse( const std::string& code )
{
    get_language()->parse(code, m_graph );
    return m_graph;
}

bool NodableHeadless::run_program() const
{
    VirtualMachine* vm = get_virtual_machine();
    EXPECT(vm != nullptr, "please init a virtual machine")

    try {
        vm->run_program();
    }
    catch ( std::runtime_error& error)
    {
        LOG_ERROR("NodableHeadless", "Unable to run the program! %s\n", error.what())
        return false;
    }

    tools::qword last_result = get_last_result();

    printf( "bool: %s | int: %12f | double: %12d | hex: %12s\n"
        , (bool)last_result ? "true" : "false"
        , (double)last_result
        , (i16_t)last_result
        , last_result.to_string().c_str()
    );
    return true;
}

const Code* NodableHeadless::compile()
{
    m_asm_code = m_compiler.compile_syntax_tree(m_graph);
    return m_asm_code;
}

const Code* NodableHeadless::compile(Graph* _graph)
{
    ASSERT(_graph != nullptr)
    return m_compiler.compile_syntax_tree(_graph);
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
    return m_graph;
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

void NodableHeadless::clear()
{
    delete m_asm_code;
    m_graph->clear();
    release_program();
    m_source_code.clear();
}

bool NodableHeadless::should_stop() const
{
    return m_should_stop;
}

const std::string& NodableHeadless::get_source_code() const
{
    return m_source_code;
}
