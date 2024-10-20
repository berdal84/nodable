#pragma once
#include <memory>     // std::shared_ptr
#include <functional> // std::function

#include "tools/core/reflection/reflection"

#include "ASTForLoopNode.h"
#include "ASTConditionalNode.h"
#include "ASTWhileLoopNode.h"
#include "ASTScopeInterface.h"
#include "ASTFunctionNode.h"
#include "ASTVariableRefNode.h"

namespace ndbl
{
    // forward declarations
    class ASTScopeInterface;
    class Nodlang;
    class ASTLiteralNode;
    class ASTForLoopNode;
    class ASTWhileLoopNode;
    class ASTConditionalNode;
    class ASTVariableRefNode;
    class ASTComponentFactory;

    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to update_world_matrix after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class ASTNodeFactory
    {
        typedef std::function<void(ASTNode*)> PostProcessFct;
    public:
        ASTNodeFactory();
        ~ASTNodeFactory();

        ASTNode*                  create_program()const;
        ASTVariableNode*          create_variable(const tools::TypeDescriptor *_type, const std::string &_name, ASTScope* _scope)const;
        ASTVariableRefNode*       create_variable_ref() const;
        ASTLiteralNode*           create_literal(const tools::TypeDescriptor *_type)const;
        ASTFunctionNode*          create_function(const tools::FunctionDescriptor*, ASTNodeType node_type = ASTNodeType_FUNCTION)const;
        ASTNode*                  create_scope()const;
        ASTConditionalNode*       create_cond_struct()const;
        ASTForLoopNode*           create_for_loop()const;
        ASTWhileLoopNode*         create_while_loop()const;
        ASTNode*                  create_node()const;
        void                      destroy_node(ASTNode* node)const;
        void                      override_post_process_fct(PostProcessFct f);

    private:
        ASTComponentFactory*      m_component_factory;
        bool m_post_process_is_overrided;
        std::function<void(ASTNode*)>  m_post_process; // invoked after each node creation, just before to return.
    };

    [[nodiscard]]
    ASTNodeFactory* get_node_factory();
    ASTNodeFactory* init_node_factory();
    void            shutdown_node_factory(ASTNodeFactory *pFactory); // Undo init_node_factory()
}