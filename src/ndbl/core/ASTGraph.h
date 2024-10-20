#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include "ndbl/core/ASTNode.h"
#include "ndbl/core/ASTNodeFactory.h"
#include "ndbl/core/ASTWhileLoopNode.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "ASTScopeInterface.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class ASTNodeFactory;
    class GraphView;
    class ASTVariableRefNode;

    typedef int ConnectFlags;
    enum ConnectFlag_
    {
        ConnectFlag_NONE               = 0,
        ConnectFlag_ALLOW_SIDE_EFFECTS = 1 << 0,
    };

    enum CreateNodeType
    {
        CreateASTNodeType_BLOCK_CONDITION,
        CreateASTNodeType_BLOCK_FOR_LOOP,
        CreateASTNodeType_BLOCK_WHILE_LOOP,
        CreateASTNodeType_BLOCK_SCOPE,
        CreateASTNodeType_BLOCK_PROGRAM,
        CreateASTNodeType_VARIABLE_BOOLEAN,
        CreateASTNodeType_VARIABLE_DOUBLE,
        CreateASTNodeType_VARIABLE_INTEGER,
        CreateASTNodeType_VARIABLE_STRING,
        CreateASTNodeType_LITERAL_BOOLEAN,
        CreateASTNodeType_LITERAL_DOUBLE,
        CreateASTNodeType_LITERAL_INTEGER,
        CreateASTNodeType_LITERAL_STRING,
        CreateASTNodeType_FUNCTION,
    };

    /**
     * @brief To manage a graph (nodes and edges)
     */
	class ASTGraph
	{
	public:
 		ASTGraph(ASTNodeFactory* factory);
		~ASTGraph();

        observe::Event<ASTNode*> on_add;

        void                     set_view(GraphView* view = nullptr);
        UpdateResult             update();

        // node related

        ASTNode*                 create_node(); // Create a raw node.
        ASTNode*                 create_node(CreateNodeType, const tools::FunctionDescriptor* _signature = nullptr); // Create a given node type in a simple way.
        ASTNode*                 create_root();
        ASTVariableNode*         create_variable(const tools::TypeDescriptor *_type, const std::string &_name, ASTScope* _scope);
        ASTVariableRefNode*      create_variable_ref();
        ASTVariableNode*         create_variable_decl(const tools::TypeDescriptor* _type, const char*  _name, ASTScope*  _scope);
        template<typename T>
        ASTVariableNode* create_variable_decl(const char*  _name = "var", ASTScope* _scope = {})
        { return create_variable_decl(tools::type::get<T>(), _name, _scope); }

        ASTLiteralNode*          create_literal(const tools::TypeDescriptor *_type);
        template<typename T>
        ASTLiteralNode*          create_literal() { return create_literal(tools::type::get<T>()); }
        ASTFunctionNode*         create_function(const tools::FunctionDescriptor*);
        ASTFunctionNode*         create_operator(const tools::FunctionDescriptor*);
        ASTNode*                 create_scope();
        ASTConditionalNode*      create_cond_struct();
        ASTForLoopNode*          create_for_loop();
        ASTWhileLoopNode*        create_while_loop();
        void                     destroy(ASTNode* _node);
        void                     ensure_has_root();
        ASTNode*                 get_root() const { return m_root; }
        GraphView*               get_view() const { return m_view; };
        bool                     is_empty() const;
        bool                     is_dirty() const { return m_is_dirty; }
        void                     set_dirty(bool value = true) { m_is_dirty = value; }
        void                     clear();  // Delete all nodes, wires, edges and reset scope.
        std::vector<ASTNode*>&      get_node_registry() {return m_node_registry;}
        const std::vector<ASTNode*>& get_node_registry()const {return m_node_registry;}
        std::multimap<SlotFlags, DirectedEdge>& get_edge_registry() {return m_edge_registry;}

        // edge related

        DirectedEdge* connect(Slot& _first, Slot& _second, ConnectFlags = ConnectFlag_NONE );
        DirectedEdge* connect_to_variable(Slot& _out, ASTVariableNode& _variable );
        DirectedEdge* connect_or_merge(Slot& _out, Slot& _in);
        void          disconnect( const DirectedEdge& _edge, ConnectFlags flags = ConnectFlag_NONE );


    private:
        // registries management
        void         add(ASTNode* _node);     // Add a given node to the registry.
        void         remove(ASTNode* _node);  // Remove a given node from the registry.
        void         remove(DirectedEdge); // Remove a given edge from the registry.

		std::vector<ASTNode*>               m_node_registry;        // registry to store all the nodes from this graph.
        std::multimap<SlotFlags , DirectedEdge> m_edge_registry; // registry ot all the edges (directed edges) between the registered nodes' properties.
        ASTNode*              m_root{nullptr};             // Graph root (main scope), without it a graph cannot be compiled.
        const ASTNodeFactory* m_factory{nullptr};
        bool               m_is_dirty{false};
        GraphView*         m_view{nullptr};    // non-owned
    };
}