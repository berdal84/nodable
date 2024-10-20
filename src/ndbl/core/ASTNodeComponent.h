#pragma once

#include "tools/core/reflection/reflection"
#include "tools/core/memory/memory.h"
#include "TComponent.h"

namespace ndbl
{
    // forward declaration
    class ASTNode;

    /**
     * @class Base abstract class for any Node Component
     */
    class ASTNodeComponent : public TComponent<ASTNode*>
	{
    public:
        POOL_REGISTRABLE(ASTNodeComponent)
        REFLECT_DERIVED_CLASS()
    };

    template<class T>
    struct IsASTNodeComponent
    {
        using type = std::is_base_of<ASTNodeComponent, T>;
        static constexpr bool value = IsASTNodeComponent<T>::type::value;
    };
}