#pragma once

#include "Graph.h"
#include "NodeFactory.h"
#include "VirtualMachine.h"
#include "ndbl/core/assembly/Compiler.h"
#include "ndbl/core/language/Nodlang.h"

namespace ndbl
{
    class NodableHeadless
    {
    public:
        Nodlang             language;
        const NodeFactory   factory;
        Graph               graph;
        assembly::Compiler  compiler;
        VirtualMachine      vm;

        NodableHeadless();

        virtual void        init();
        virtual void        shutdown();
    };
}

