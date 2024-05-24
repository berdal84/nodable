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
        NodableHeadless();
        virtual void        init();
        virtual void        update();
        virtual void        shutdown();
        std::string&        serialize( std::string& out ) const;
        Graph*              parse( const std::string& in );
        const Code*         compile(Graph*);
        bool                load_program(const Code*);
        void                run_program() const;
        bool                release_program();
        Nodlang*            get_language() const;
        Graph*              get_graph() const;
        tools::qword        get_last_result() const;

        template<typename ResultT>
        ResultT get_result_as()
        {
            tools::qword mem_space = get_virtual_machine()->get_last_result();
            return ResultT(mem_space);
        }

    protected:
        Graph*              graph;
        assembly::Compiler  compiler;
    };
}

