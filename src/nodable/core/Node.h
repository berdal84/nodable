#pragma once

#include <observe/event.h>
#include <string>
#include <memory>
#include <algorithm>

#include "fw/core/assertions.h"
#include "fw/core/reflection/reflection"
#include "fw/core/types.h"

#include "Component.h"
#include "Components.h"
#include "DirectedEdge.h"
#include "PropertyGrp.h"
#include "Slots.h"
#include "constants.h"

namespace ndbl {

    // forward declarations
    class Graph;

    /**
     * Distinguish between all possible update result
     */
    enum class UpdateResult
    {
        Success_NoChanges,
        Success_WithChanges,
    };

	/**
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed with Properties) that can be linked
	    together in order to create graphs.

		Every Node has a parent Graph. All nodes are built from a Graph,
	    which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
	class Node
	{
	public:

        // Data

        std::string                     name;
        Node*                           parent;
        Graph*                          parent_graph;
        PropertyGrp                     props;
        Property*                       as_property; // Property to this instance
        Components                      components;
        Slots<Node*>                    successors;
        Slots<Node*>                    predecessors;
        Slots<Node*>                    children;
        Slots<Node*>                    outputs;
        Slots<Node*>                    inputs;
        bool                            dirty; // TODO: use flags
        bool                            flagged_to_delete; // TODO: use flags
        std::set<const DirectedEdge*>   edges;
        observe::Event<Node*, Edge_t>   on_edge_added; // TODO: share a unique event
        observe::Event<Node*, Edge_t>   on_edge_removed; // TODO: share a unique event
        observe::Event<Node*>           on_name_change; // TODO: share a unique event

        // Code

        explicit Node(std::string  _label = "UnnamedNode");

        Node (const Node&) = delete;

        Node& operator= (const Node&) = delete;

		virtual ~Node() {}

        void                 set_parent(Node* _node);
        void                 set_name(const char *_label);
		void                 add_edge(const DirectedEdge* edge);
		void                 remove_edge(const DirectedEdge*);
        size_t               incoming_edge_count()const;
        size_t               outgoing_edge_count()const;
        const fw::iinvokable*get_connected_invokable(const Property *each_edge); // TODO: can't remember to understand why I needed this...
        bool                 is_connected_with(const Property *_localProperty);

        template<class T>
        inline T* as()
        { return fw::cast<T>(this); }

        template<class T>
        inline const T* as()const
        { return fw::cast<const T>(this); }

        template<class T>
        inline bool is()const
        { return fw::cast<const T>(this) != nullptr; }

        template<class T>
        static void get_components(const std::vector<Node*>& _in_nodes, std::vector<T*>& _out_components)
        {
            _out_components.reserve(_in_nodes.size());
            for (auto eachNode : _in_nodes)
            {
                if (T* view = eachNode->components.get<T>())
                {
                    _out_components.push_back(view);
                }
            }
        }

        template<class T>
        inline T* get_component()
        { return components.get<T>(); }

        template<class T>
        inline T* get_component() const
        { return components.get<T>(); }

        template<class T>
        inline bool has_component() const
        { return components.has<T>(); }

        template<typename T, typename... Args>
        T* add_component(Args... args)
        { return components.add<T>(args...); }

		REFLECT_BASE_CLASS()
    };
}
