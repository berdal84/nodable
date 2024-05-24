#pragma once

#include "ndbl/core/assembly/Compiler.h"

namespace ndbl
{
    class Graph;

    class NodableHeadless
    {
    public:
        Graph*              graph;
        assembly::Compiler  compiler;

        NodableHeadless();

        virtual void        init();
        virtual void        shutdown();
    };
}

