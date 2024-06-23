#pragma once
#include <memory>     // std::shared_ptr
#include <functional> // std::function

#include "tools/core/reflection/reflection"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "WhileLoopNode.h"
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
     * @brief The NodeFactory instantiate Nodes. Class take a function to update after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory
    {
        using PostProcessFct = std::function<void(Node*)>;
    public:
        NodeFactory();
        ~NodeFactory(){}

        Node*                  create_program()const;
        VariableNode*          create_variable(const tools::type *_type, const std::string &_name, Scope* _scope)const;
        LiteralNode*           create_literal(const tools::type *_type)const;
        Node*                  create_abstract_func(const tools::func_type *_signature, bool _is_operator)const;
        Node*                  create_func(const tools::IInvokable*_function, bool _is_operator)const;
        Node*                  create_scope()const;
        IfNode*                create_cond_struct()const;
        ForLoopNode*           create_for_loop()const;
        WhileLoopNode*         create_while_loop()const;
        Node*                  create_node()const;
        void                   destroy_node(Node* node)const;
        void                   override_post_process_fct( NodeFactory::PostProcessFct f );
    private:
        Node*                  create_abstract_func_no_postprocess(const tools::func_type *_func_type, bool _is_operator) const; // this do not invoke post_process
        void                   add_invokable_component(Node* _node, const tools::func_type *_func_type, const tools::IInvokable*_invokable, bool _is_operator) const;

        bool m_post_process_is_overrided;
        std::function<void(Node*)>  m_post_process; // invoked after each node creation, just before to return.
    };

    NodeFactory* get_node_factory();
    NodeFactory* init_node_factory();
    void         shutdown_node_factory(); // Undo init_node_factory()
}