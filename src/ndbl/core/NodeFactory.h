#pragma once
#include <memory>     // std::shared_ptr
#include <functional> // std::function

#include "tools/core/reflection/reflection"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "WhileLoopNode.h"
#include "IScope.h"
#include "FunctionNode.h"
#include "VariableRefNode.h"

namespace ndbl
{
    // forward declarations
    class IScope;
    class Nodlang;
    class LiteralNode;
    class ForLoopNode;
    class WhileLoopNode;
    class IfNode;
    class VariableRefNode;

    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to update_world_matrix after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory
    {
        typedef std::function<void(Node*)> PostProcessFct;
    public:
        NodeFactory();
        ~NodeFactory(){}

        Node*                  create_program()const;
        VariableNode*          create_variable(const tools::TypeDescriptor *_type, const std::string &_name, Scope* _scope)const;
        VariableRefNode*       create_variable_ref() const;
        LiteralNode*           create_literal(const tools::TypeDescriptor *_type)const;
        FunctionNode*          create_function(const tools::FunctionDescriptor*, NodeType node_type = NodeType_FUNCTION)const;
        Node*                  create_scope()const;
        IfNode*                create_cond_struct()const;
        ForLoopNode*           create_for_loop()const;
        WhileLoopNode*         create_while_loop()const;
        Node*                  create_node()const;
        void                   destroy_node(Node* node)const;
        void                   override_post_process_fct(PostProcessFct f);

    private:
        bool m_post_process_is_overrided;
        std::function<void(Node*)>  m_post_process; // invoked after each node creation, just before to return.
    };

    [[nodiscard]]
    NodeFactory* get_node_factory();
    NodeFactory* init_node_factory();
    void         shutdown_node_factory(NodeFactory *pFactory); // Undo init_node_factory()
}