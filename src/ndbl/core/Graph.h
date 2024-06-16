#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include "ndbl/core/Node.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/WhileLoopNode.h"
#include "tools/core/reflection/class.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "IScope.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class NodeFactory;
    class GraphView;

    typedef int ConnectFlags;
    enum ConnectFlag_
    {
        ConnectFlag_NONE               = 0,
        ConnectFlag_ALLOW_SIDE_EFFECTS = 1 << 0,
    };

    enum NodeType : u16_t {
        NodeType_BLOCK_CONDITION,
        NodeType_BLOCK_FOR_LOOP,
        NodeType_BLOCK_WHILE_LOOP,
        NodeType_BLOCK_SCOPE,
        NodeType_BLOCK_PROGRAM,
        NodeType_VARIABLE_BOOLEAN,
        NodeType_VARIABLE_DOUBLE,
        NodeType_VARIABLE_INTEGER,
        NodeType_VARIABLE_STRING,
        NodeType_LITERAL_BOOLEAN,
        NodeType_LITERAL_DOUBLE,
        NodeType_LITERAL_INTEGER,
        NodeType_LITERAL_STRING,
        NodeType_INVOKABLE,
    };

    /**
     * @brief To manage a graph (nodes and edges)
     */
	class Graph
	{
	public:
 		Graph(NodeFactory* factory);
		~Graph();

        observe::Event<Node*> on_add;

        void                        set_view(GraphView* view = nullptr);
        UpdateResult                update();

        // node related

        Node*                    create_node(); // Create a raw node.
        Node*                    create_node(NodeType, const tools::func_type* _signature = nullptr); // Create a given node type in a simple way.
        Node*                    create_root();
        VariableNode*            create_variable(const tools::type *_type, const std::string &_name, Scope* _scope);
        VariableNode*            create_variable_decl(const tools::type* _type, const char*  _name, Scope*  _scope);
        template<typename T>
        VariableNode* create_variable_decl(const char*  _name = "var", Scope* _scope = {})
        { return create_variable_decl( tools::type::get<T>(), _name, _scope); }

        LiteralNode*             create_literal(const tools::type *_type);
        template<typename T>
        LiteralNode*             create_literal() { return create_literal( tools::type::get<T>()); }
        Node*                    create_abstract_function(const tools::func_type *_invokable, bool _is_operator = false); // Create and append a new abstract (without known implementation)  function of a given type.
        Node*                    create_function(const tools::IInvokable*_invokable, bool _is_operator = false);
        Node*                    create_abstract_operator(const tools::func_type *_invokable);  // Create a new abstract (without known implementation) operator.
        Node*                    create_operator(const tools::IInvokable*_invokable);
        Node*                    create_scope();
        IfNode*                  create_cond_struct();
        ForLoopNode*             create_for_loop();
        WhileLoopNode*           create_while_loop();
        void                     destroy(Node* _node);
        void                     ensure_has_root();
        Node*                    get_root() const { return m_root; }
        GraphView*               get_view() const { return m_view; };
        bool                     is_empty() const;
        bool                     is_dirty() const { return m_is_dirty; }
        void                     set_dirty(bool value = true) { m_is_dirty = value; }
        void                     clear();  // Delete all nodes, wires, edges and reset scope.
        std::vector<Node*>&      get_node_registry() {return m_node_registry;}
        const std::vector<Node*>& get_node_registry()const {return m_node_registry;}
        std::multimap<SlotFlags, DirectedEdge>& get_edge_registry() {return m_edge_registry;}

        // edge related

        DirectedEdge* connect(Slot& _first, Slot& _second, ConnectFlags = ConnectFlag_NONE );
        DirectedEdge* connect_to_variable(Slot& _out, VariableNode& _variable );
        DirectedEdge* connect_or_merge(Slot& _out, Slot& _in);
        void          disconnect( const DirectedEdge& _edge, ConnectFlags flags = ConnectFlag_NONE );


    private:
        // registries management
        void         add(Node* _node);     // Add a given node to the registry.
        void         remove(Node* _node);  // Remove a given node from the registry.
        void         remove(DirectedEdge); // Remove a given edge from the registry.

		std::vector<Node*>               m_node_registry;        // registry to store all the nodes from this graph.
        std::multimap<SlotFlags , DirectedEdge> m_edge_registry; // registry ot all the edges (directed edges) between the registered nodes' properties.
        Node*              m_root;             // Graph root (main scope), without it a graph cannot be compiled.
        const NodeFactory* m_factory{nullptr};
        bool               m_is_dirty{false};
        GraphView*         m_view{nullptr};    // non-owned
    };
}