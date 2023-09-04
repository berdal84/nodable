#pragma once
#include <vector>
#include <algorithm>

namespace fw
{
    template<typename Out, typename In, typename F = std::function<Out(In)>>
    inline std::vector<Out> map( std::vector<In>& in, F map_function )
    {
        std::vector<Out> out;
        out.reserve( in.size() );
        std::transform( in.begin(), in.end(), out.end(), map_function );
        return out;
    }
} // namespace fw