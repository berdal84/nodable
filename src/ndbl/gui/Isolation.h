#pragma once

namespace ndbl
{
    typedef int Isolation;
    enum Isolation_
    {
        Isolation_OFF = 0,
        Isolation_ON = ~Isolation_OFF,
    };
}