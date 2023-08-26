#pragma once

#include <observe/event.h>
#include <string>
#include <memory>
#include <algorithm>

#include "fw/core/assertions.h"
#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/core/Pool.h"

#include "Components.h"
#include "DirectedEdge.h"
#include "PropertyBag.h"
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
        SUCCES_WITHOUT_CHANGES,
        SUCCESS_WITH_CHANGES,
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
        ID<Node>          parent;
        std::string       name;
        Graph*            parent_graph;
        PropertyBag       props;
        Slots<ID<Node>>   successors;
        Slots<ID<Node>>   predecessors;
        Slots<ID<Node>>   children;
        Slots<ID<Node>>   outputs;
        Slots<ID<Node>>   inputs;
        bool                            dirty; // TODO: use flags
        bool                            flagged_to_delete; // TODO: use flags
        std::set<const DirectedEdge*>   edges;
        observe::Event<ID<Node>, SlotEvent, Edge_t> on_slot_change;
        observe::Event<ID<Node>>                    on_name_change;

        // Code

        Node(std::string  _label = "UnnamedNode");
        Node(Node&&) = default;
        Node& operator=(Node&&) = default;
        virtual ~Node() = default;

        Property*            as_prop() { return get_prop(k_this_property_name ); }
        void                 set_parent(ID<Node> _node);
        void                 set_name(const char *_label);
		void                 add_edge(const DirectedEdge* edge);
		void                 remove_edge(const DirectedEdge*);
        size_t               incoming_edge_count()const;
        size_t               outgoing_edge_count()const;
        const fw::iinvokable*get_connected_invokable(const Property *_local_property) const; // TODO: can't remember to understand why I needed this...
        bool                 is_connected_with(const Property *property_id);
        std::vector<ID<Component>> get_components();

        Property* get_prop(const char* _name)
        { return props.get(_name); }

        Property* get_prop(const char* _name) const
        { return props.get(_name); }

        template<typename ValueT, typename ...ArgsT>
        Property* add_prop(ArgsT...args)
        { return props.add<ValueT>(args...); }

        template<class ComponentT>
        void add_component(ID<ComponentT> component)
        { return m_components.add( component ); }

        template<class ComponentT>
        ID<ComponentT> get_component() const
        { return m_components.get<ComponentT>(); }

        template<class ComponentT>
        bool has_component() const
        { return m_components.has<ComponentT>(); }

		REFLECT_BASE_CLASS()
        POOL_REGISTRABLE_WITH_CUSTOM_IMPLEM(Node);
    private:
        Components m_components;
    };
}

static_assert(std::is_move_assignable_v<ndbl::Node>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Node>, "Should be move constructible");
