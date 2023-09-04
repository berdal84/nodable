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
#include "Relation.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class NodeFactory;

    enum class ConnectFlag
    {
        SIDE_EFFECTS_OFF = 0,
        SIDE_EFFECTS_ON,
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

        ID<Node>                    create_root();
        ID<InstructionNode>         create_instr();
        ID<VariableNode>            create_variable(const fw::type *_type, const std::string &_name, ID<Scope> _scope);
        ID<LiteralNode>             create_literal(const fw::type *_type);
        template<typename T>
        ID<LiteralNode>             create_literal() { return create_literal(fw::type::get<T>()); }
        ID<Node>                    create_abstract_function(const fw::func_type *_invokable, bool _is_operator = false); // Create and append a new abstract (without known implementation)  function of a given type.
        ID<Node>                    create_function(const fw::iinvokable *_invokable, bool _is_operator = false);
        ID<Node>                    create_abstract_operator(const fw::func_type *_invokable);  // Create a new abstract (without known implementation) operator.
        ID<Node>                    create_operator(const fw::iinvokable *_invokable);
        ID<Node>                    create_scope();
        ID<ConditionalStructNode>   create_cond_struct();
        ID<ForLoopNode>             create_for_loop();
        ID<WhileLoopNode>           create_while_loop();
        ID<Node>                    create_node();
        void                        destroy(ID<Node> _node);
        void                        ensure_has_root();
        ID<Node>                    get_root()const { return m_root; }
        bool                        is_empty() const;
        bool                        is_dirty() const { return m_is_dirty; }
        void                        set_dirty(bool value = true) { m_is_dirty = value; }
        void                        clear();  // Delete all nodes, wires, edges and reset scope.
        std::vector<ID<Node>>&      get_node_registry() {return m_node_registry;}
        const std::vector<ID<Node>>& get_node_registry()const {return m_node_registry;}
        std::multimap<Relation, Edge>& get_edge_registry() {return m_edge_registry;}

        // edge related

        Edge         connect(Edge, ConnectFlag);
        Edge         connect(Slot _tail, Relation, Slot _head, ConnectFlag);
        Edge         connect(Connector _tail, Relation, Connector _head, ConnectFlag);
        Edge         connect(Slot, Slot);
        Edge         connect(Node*, InstructionNode*);
        Edge         connect(Slot, VariableNode*);
        void         disconnect(Edge, ConnectFlag);

    private:
        // register management
        void                        add(ID<Node> _node);    // Add a given node to the registry.
        void                        remove(ID<Node> _node); // Remove a given node from the registry.
        void                        remove(Edge);  // Remove a given edge from the registry.

        bool                     m_is_dirty;
		std::vector<ID<Node>>    m_node_registry;       // registry to store all the nodes from this graph.
        std::multimap<Relation, Edge> m_edge_registry;       // registry ot all the edges (directed edges) between the registered nodes' properties.
		ID<Node>                 m_root;                // Graph root (main scope), without it a graph cannot be compiled.
		const NodeFactory*       m_factory;             // Node factory (can be headless or not depending on the context: app, unit tests, cli).
    };
}