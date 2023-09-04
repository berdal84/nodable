#pragma once

#include "fw/core/reflection/reflection"

namespace ndbl
{
    // forward declarations
    class Scope;
    class Property;
    class InstructionNode;

    /**
     * @interface Interface for any conditional structure node (ex: if/else, for, while, do/while )
     */
    class IConditionalStruct
    {
    public:
        virtual const Property*  condition_property()const = 0;
        virtual ID<Scope>  get_condition_true_scope()const = 0;      // Get the "true" (if) branch's scope
        virtual ID<Scope>  get_condition_false_scope()const = 0;     // Get the "false" (else) branch's scope

        REFLECT_BASE_CLASS()
    };
}