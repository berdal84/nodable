#pragma once

#include <memory>     // std::shared_ptr
#include <functional> // std::function
#include <fw/core/reflection/reflection>

#include <ndbl/core/INodeFactory.h>
#include <ndbl/core/IScope.h>

namespace ndbl
{
    // forward declarations
    class IScope;
    class Nodlang;

    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to apply after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory: public INodeFactory
    {
    public:
        NodeFactory(const Nodlang* _language, std::function<void(Node *)> _post_process_fct = [](Node* _node){} )
        : m_language(_language)
        , m_post_process(_post_process_fct) {}
        ~NodeFactory() {}

        Node*                       new_program()const override ;
        InstructionNode*            new_instr()const override ;
        VariableNode*				new_variable(fw::type, const std::string&, IScope *)const override ;
        LiteralNode*                new_literal(fw::type)const override ;
        Node*                       new_abstract_function(const fw::func_type*, bool _is_operator)const override;
        Node*                       new_function(const fw::iinvokable*, bool _is_operator)const override ;
        Node*                       new_scope()const override ;
        ConditionalStructNode*      new_cond_struct()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       new_node()const override ;
    private:
        Node*                       _new_abstract_function(const fw::func_type*, bool _is_operator) const; // this do not invoke post_process
        void                        add_invokable_component(Node *_node, const fw::func_type*, const fw::iinvokable *_invokable, bool _is_operator) const;

        std::function<void(Node*)>  m_post_process; // invoked after each node creation, just before to return.
        const Nodlang*              m_language;
    };
}
