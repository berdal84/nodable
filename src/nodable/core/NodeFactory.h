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
    public:
        NodeFactory(std::function<void(Node *)> _post_process_fct = [](Node* _node){} )
        : m_post_process(_post_process_fct) {}
        ~NodeFactory() {}

        Node*                       new_program()const;
        InstructionNode*            new_instr()const;
        VariableNode*				new_variable(const fw::type *, const std::string&, IScope *)const;
        LiteralNode*                new_literal(const fw::type *)const;
        Node*                       new_abstract_function(const fw::func_type*, bool _is_operator)const;
        Node*                       new_function(const fw::iinvokable*, bool _is_operator)const;
        Node*                       new_scope()const;
        ConditionalStructNode*      new_cond_struct()const;
        ForLoopNode*                new_for_loop_node()const;
        WhileLoopNode*              new_while_loop_node()const;
        Node*                       new_node()const;
        void                        delete_node(Node *pNode)const;

    private:
        Node*                       _new_abstract_function(const fw::func_type*, bool _is_operator) const; // this do not invoke post_process
        void                        add_invokable_component(Node *_node, const fw::func_type*, const fw::iinvokable *_invokable, bool _is_operator) const;

        std::function<void(Node*)>  m_post_process; // invoked after each node creation, just before to return.
    };
}
