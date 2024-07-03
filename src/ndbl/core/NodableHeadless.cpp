#include "NodableHeadless.h"
#include "Interpreter.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/memory/PoolManager.h"
#include "tools/core/TaskManager.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;

void NodableHeadless::init()
{
    m_task_manager    = tools::init_task_manager();
    m_language        = init_language();
    m_node_factory    = init_node_factory();
    m_interpreter     = init_interpreter();
    m_graph = new Graph(m_node_factory);
}

void NodableHeadless::shutdown()
{
    clear();
    delete m_graph;
    tools::shutdown_task_manager(m_task_manager);
    shutdown_language(m_language);
    shutdown_node_factory(m_node_factory);
    shutdown_interpreter(m_interpreter);
}

std::string& NodableHeadless::serialize( std::string& out ) const
{
    return m_language->serialize_node( out, m_graph->get_root(), SerializeFlag_RECURSE );
}

Graph* NodableHeadless::parse( const std::string& code )
{
    m_language->parse(code, m_graph );
    return m_graph;
}

bool NodableHeadless::run_program() const
{
    VERIFY(m_interpreter != nullptr, "Did you call init() ?")

    try {
        m_interpreter->run_program();
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
    return m_interpreter->load_program(code);
}

Nodlang* NodableHeadless::get_language() const
{
    return m_language;
}

Graph* NodableHeadless::get_graph() const
{
    return m_graph;
}

bool NodableHeadless::release_program()
{
    return m_interpreter->release_program();
}

tools::qword NodableHeadless::get_last_result() const
{
    return m_interpreter->get_last_result();
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
