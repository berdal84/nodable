#pragma once

#include <memory>     // std::shared_ptr
#include <functional> // std::function

#include "core/ConditionalStructNode.h"
#include "core/ForLoopNode.h"
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
    class ConditionalStructNode;

    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to apply after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory
    {
        using PostProcessFct = std::function<void(ID<Node>)>;
    public:
        NodeFactory();
        ~NodeFactory(){}

        ID<Node>                  create_program()const;
        ID<InstructionNode>       create_instr()const;
        ID<VariableNode>          create_variable(const fw::type *_type, const std::string &_name, ID<Scope> _scope)const;
        ID<LiteralNode>           create_literal(const fw::type *_type)const;
        ID<Node>                  create_abstract_func(const fw::func_type *_signature, bool _is_operator)const;
        ID<Node>                  create_func(const fw::iinvokable *_function, bool _is_operator)const;
        ID<Node>                  create_scope()const;
        ID<ConditionalStructNode> create_cond_struct()const;
        ID<ForLoopNode>           create_for_loop()const;
        ID<WhileLoopNode>         create_while_loop()const;
        ID<Node>                  create_node()const;
        void                      destroy_node(ID<Node> node)const;
        void                      set_post_process_fct( PostProcessFct );
    private:
        ID<Node>                  _create_abstract_func(const fw::func_type *_func_type, bool _is_operator) const; // this do not invoke post_process
        void                      add_invokable_component(ID<Node> _node, const fw::func_type *_func_type, const fw::iinvokable *_invokable, bool _is_operator) const;

        bool                           m_post_process_set;
        std::function<void(ID<Node>)>  m_post_process; // invoked after each node creation, just before to return.
    };
}
