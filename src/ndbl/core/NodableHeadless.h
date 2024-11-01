#pragma once

#include <string>
#include "Interpreter.h"
#include "Compiler.h"

namespace tools
{
    struct PoolManager;
    struct TaskManager;
}

namespace ndbl
{
    class Graph;

    class NodableHeadless
    {
    public:
        NodableHeadless() = default;
        virtual ~NodableHeadless() = default;
        virtual void        init();
        virtual void        update();
        virtual void        shutdown();
        virtual void        clear();
        bool                should_stop() const;
        virtual std::string& serialize( std::string& out ) const;
        virtual Graph*      parse( const std::string& in );
        virtual const Code* compile(Graph*);
        const Code*         compile();
        bool                load_program(const Code*);
        bool                run_program() const;
        bool                release_program();
        Nodlang*            get_language() const;
        Graph*              get_graph() const;
        tools::qword        get_last_result() const;
        const std::string&  get_source_code() const;

        template<typename ResultT>
        ResultT get_last_result_as()
        { return ResultT(m_interpreter->get_last_result()); }

    protected:
        tools::TaskManager* m_task_manager{};
        Nodlang*            m_language{};
        NodeFactory*        m_node_factory{};
        ComponentFactory*   m_component_factory{};
        Interpreter*        m_interpreter{};
        bool                m_should_stop{false};
        Graph*              m_graph{};
        std::string         m_source_code;
        const Code*         m_asm_code{};
        bool                m_auto_completion{false};
        Compiler            m_compiler{}; // TODO: move this to a global (like VirtualMachine.h)
    };
}

