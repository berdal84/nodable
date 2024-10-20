#pragma once

#include <string>
#include "Interpreter.h"
#include "ASTCompiler.h"

namespace tools
{
    struct PoolManager;
    struct TaskManager;
}

namespace ndbl
{
    class ASTGraph;

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
        virtual ASTGraph*      parse(const std::string& in );
        virtual const Code* compile(ASTGraph*);
        const Code*         compile();
        bool                load_program(const Code*);
        bool                run_program() const;
        bool                release_program();
        Nodlang*            get_language() const;
        ASTGraph*              get_graph() const;
        tools::qword        get_last_result() const;
        const std::string&  get_source_code() const;

        template<typename ResultT>
        ResultT get_last_result_as()
        { return ResultT(m_interpreter->get_last_result()); }

    protected:
        tools::TaskManager* m_task_manager{};
        Nodlang*            m_language{};
        ASTNodeFactory*        m_node_factory{};
        Interpreter*        m_interpreter{};
        bool                m_should_stop{false};
        ASTGraph*              m_graph{};
        std::string         m_source_code;
        const Code*         m_asm_code{};
        bool                m_auto_completion{false};
        ASTCompiler            m_compiler{}; // TODO: move this to a global (like VirtualMachine.h)
    };
}

