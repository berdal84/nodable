#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include "nodable/core/Node.h"
#include "nodable/core/NodeFactory.h"
#include "nodable/core/WhileLoopNode.h"
#include "fw/core/reflection/class.h"
#include "fw/core/reflection/reflection"
#include "fw/core/types.h"

#include "IScope.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class NodeFactory;

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
        NodeType_FUNCTION,
        NodeType_OPERATOR,
    };

    /**
     * @brief To manage a graph (nodes and edges)
     */
	class Graph
	{
	public:
 		explicit Graph(const NodeFactory*);
		~Graph();

        UpdateResult                update();

        // node related

        PoolID<Node>                    create_node(); // Create a basic node.
        PoolID<Node>                    create_node(NodeType, const char* _signature_hint = nullptr); // Create a given node type in a simple way.
        PoolID<Node>                    create_root();
        PoolID<VariableNode>            create_variable(const fw::type *_type, const std::string &_name, PoolID<Scope> _scope);
        PoolID<VariableNode>            create_variable_decl(const fw::type* _type, const char*  _name, PoolID<Scope>  _scope);
        template<typename T>
        PoolID<VariableNode> create_variable_decl(const char*  _name = "var", PoolID<Scope> _scope = {})
        { return create_variable_decl( fw::type::get<T>(), _name, _scope); }

        PoolID<LiteralNode>             create_literal(const fw::type *_type);
        template<typename T>
        PoolID<LiteralNode>             create_literal() { return create_literal(fw::type::get<T>()); }
        PoolID<Node>                    create_abstract_function(const fw::func_type *_invokable, bool _is_operator = false); // Create and append a new abstract (without known implementation)  function of a given type.
        PoolID<Node>                    create_function(const fw::iinvokable *_invokable, bool _is_operator = false);
        PoolID<Node>                    create_abstract_operator(const fw::func_type *_invokable);  // Create a new abstract (without known implementation) operator.
        PoolID<Node>                    create_operator(const fw::iinvokable *_invokable);
        PoolID<Node>                    create_scope();
        PoolID<IfNode>                  create_cond_struct();
        PoolID<ForLoopNode>             create_for_loop();
        PoolID<WhileLoopNode>           create_while_loop();
        void                            destroy(PoolID<Node> _node);
        void                            ensure_has_root();
        PoolID<Node>                    get_root()const { return m_root; }
        bool                            is_empty() const;
        bool                            is_dirty() const { return m_is_dirty; }
        void                            set_dirty(bool value = true) { m_is_dirty = value; }
        void                            clear();  // Delete all nodes, wires, edges and reset scope.
        std::vector<PoolID<Node>>&      get_node_registry() {return m_node_registry;}
        const std::vector<PoolID<Node>>& get_node_registry()const {return m_node_registry;}
        std::multimap<SlotFlags, DirectedEdge>& get_edge_registry() {return m_edge_registry;}

        // edge related

        DirectedEdge* connect(Slot& _first, Slot& _second, ConnectFlags = ConnectFlag_NONE );
        DirectedEdge* connect_to_variable(Slot& _out, VariableNode& _variable );
        DirectedEdge* connect_or_merge(Slot& _out, Slot& _in);
        void          disconnect( const DirectedEdge& _edge, ConnectFlags flags = ConnectFlag_NONE );

    private:
        // register management
        void         add(PoolID<Node> _node); // Add a given node to the registry.
        void         remove(PoolID<Node> _node); // Remove a given node from the registry.
        void         remove(DirectedEdge); // Remove a given edge from the registry.

		std::vector<PoolID<Node>>               m_node_registry;       // registry to store all the nodes from this graph.
        std::multimap<SlotFlags , DirectedEdge> m_edge_registry;       // registry ot all the edges (directed edges) between the registered nodes' properties.
        PoolID<Node>       m_root;                // Graph root (main scope), without it a graph cannot be compiled.
        const NodeFactory* m_factory;             // Node factory (can be headless or not depending on the context: app, unit tests, cli).
        bool               m_is_dirty;
    };
}