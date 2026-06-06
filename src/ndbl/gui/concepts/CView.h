#pragma once
#include <concepts>
#include "tools/gui/geometry/Spatial_Node.h"

namespace ndbl
{
    template<typename T>
    concept CView = requires(T *t) {
        { t->spatial_node() } -> std::same_as<tools::Spatial_Node *>;
    };
}
