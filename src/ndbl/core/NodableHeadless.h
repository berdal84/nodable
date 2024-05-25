#pragma once

#include <string>
#include "VirtualMachine.h"
#include "ndbl/core/assembly/Compiler.h"

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
        ResultT get_result_as()
        {
            tools::qword mem_space = get_virtual_machine()->get_last_result();
            return ResultT(mem_space);
        }

        bool                       m_should_stop{false};

    protected:
        Graph*                     m_graph{nullptr};
        std::string                m_source_code;
        const assembly::Code*      m_asm_code{nullptr};
        bool                       m_auto_completion{false};
        assembly::Compiler         m_compiler; // TODO: move this to a global (like VirtualMachine.h)
    };
}

