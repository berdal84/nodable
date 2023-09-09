#pragma once

#include <observe/event.h>
#include <string>
#include <memory>
#include <algorithm>

#include "fw/core/assertions.h"
#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/core/Pool.h"

#include "ComponentBag.h"
#include "DirectedEdge.h"
#include "Property.h"
#include "PropertyBag.h"
#include "SlotBag.h"
#include "TDirectedEdge.h"
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
        REFLECT_BASE_CLASS()
        size_t get_slot_count(Relation edge, Way way) const;
        POOL_REGISTRABLE_WITH_CUSTOM_IMPLEM(Node);
	public:
        // Data

        ID<Node>          parent;
        std::string       name;
        Graph*            parent_graph;
        PropertyBag       props;
        SlotBag           slots;
        bool              dirty; // TODO: use flags
        bool              flagged_to_delete; // TODO: use flags

        observe::Event<SlotBag::Event> on_slot_change;
        observe::Event<ID<Node>>       on_name_change;

        // Code

        Node(std::string  _label = "UnnamedNode");
        Node(Node&&) = default;
        Node& operator=(Node&&) = default;
        virtual ~Node() = default;

        std::vector<DirectedEdge> filter_edges(Relation edge) const;
        std::vector<DirectedEdge>    edges() const;
        std::vector<ID<Node>> successors() const;
        std::vector<ID<Node>> children() const;
        bool                 allows_more(Relation);
        void                 set_parent(ID<Node>);
        void                 set_name(const char*);
		void                 add_edge(DirectedEdge);
		void                 remove_edge(DirectedEdge);
        size_t               incoming_edge_count()const;
        size_t               outgoing_edge_count()const;
        size_t               edge_count(Way)const;
        std::vector<ID<Node>>get_predecessors() const;
        Slot&slot(Way way = Way::InOut) const;
        Slot&                get_slot(Property*, Way) const;
        Slot&                get_slot(Property::ID, Way) const;
        Slot&                get_slot(const char* _name, Way) const;
        const fw::iinvokable*get_connected_invokable(const char *property_name) const; // TODO: can't remember to understand why I needed this...
        DirectedEdge                 get_edge_heading(Property::ID) const;
        DirectedEdge                 get_edge_heading(const char* name) const;
        bool                 has_edge_heading(size_t property) const;
        bool                 has_edge_heading(const char* name) const;
        Property*            get_prop_at(size_t pos);
        const Property*      get_prop_at(size_t pos) const;
        Property*            get_prop(const char* _name);
        const Property*      get_prop(const char* _name) const;
        std::vector<DirectedEdge>    get_input_edges(const std::vector<Property::ID>& properties) const;
        std::vector<ID<Component>> get_components();

        template<typename ValueT, typename ...ArgsT>
        size_t add_prop(ArgsT...args)
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
    private:
        ComponentBag m_components;
    };
}

static_assert(std::is_move_assignable_v<ndbl::Node>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Node>, "Should be move constructible");
