#pragma once

#include <memory>     // std::shared_ptr
#include <functional> // std::function

#include "core/ForLoopNode.h"
#include "core/IfNode.h"
#include "core/WhileLoopNode.h"
#include "fw/core/reflection/reflection"

#include "IScope.h"

namespace ndbl
{
    // forward declarations
    class IScope;
    class Nodlang;
    class LiteralNode;
    class ForLoopNode;
    class WhileLoopNode;
    class IfNode;

    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to apply after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory
    {
        using PostProcessFct = std::function<void(PoolID<Node>)>;
    public:
        NodeFactory();
        ~NodeFactory(){}

        PoolID<Node>                  create_program()const;
        PoolID<VariableNode>          create_variable(const fw::type *_type, const std::string &_name, PoolID<Scope> _scope)const;
        PoolID<LiteralNode>           create_literal(const fw::type *_type)const;
        PoolID<Node>                  create_abstract_func(const fw::func_type *_signature, bool _is_operator)const;
        PoolID<Node>                  create_func(const fw::iinvokable *_function, bool _is_operator)const;
        PoolID<Node>                  create_scope()const;
        PoolID<IfNode> create_cond_struct()const;
        PoolID<ForLoopNode>           create_for_loop()const;
        PoolID<WhileLoopNode>         create_while_loop()const;
        PoolID<Node>                  create_node()const;
        void                          destroy_node(PoolID<Node> node)const;
        void override_post_process_fct( NodeFactory::PostProcessFct f );
    private:
        PoolID<Node>                  _create_abstract_func(const fw::func_type *_func_type, bool _is_operator) const; // this do not invoke post_process
        void                          add_invokable_component(PoolID<Node> _node, const fw::func_type *_func_type, const fw::iinvokable *_invokable, bool _is_operator) const;

        bool m_post_process_is_overrided;
        std::function<void(PoolID<Node>)>  m_post_process; // invoked after each node creation, just before to return.
    };
}
