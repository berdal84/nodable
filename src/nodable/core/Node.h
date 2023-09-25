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
        POOL_REGISTRABLE(Node)
	public:
        // Data

        std::string       name;
        Graph*            parent_graph;
        PropertyBag       props;
        SlotBag           slots;
        bool              dirty; // TODO: use flags
        bool              flagged_to_delete; // TODO: use flags

        observe::Event<PoolID<Node>> on_name_change;

        // Code

        Node(std::string  _label = "UnnamedNode");
        Node(Node&&);
        Node& operator=(Node&&);
        virtual ~Node() = default;

        virtual void init();
        std::vector<PoolID<Node>> filter_adjacent(SlotFlags) const;
        std::vector<PoolID<Node>> successors() const;
        std::vector<PoolID<Node>> children() const;
        std::vector<PoolID<Node>> inputs() const;
        std::vector<PoolID<Node>> outputs() const;
        std::vector<PoolID<Node>> predecessors() const;
        void                 set_name(const char*);
        size_t               adjacent_count(SlotFlags )const;
        std::vector<PoolID<Node>>get_predecessors() const;
        PoolID<Node>         get_parent() const;
        Slot*                find_slot( SlotFlags ); // implicitly THIS_PROPERTY's slot
        Slot&                get_slot(ID8<Slot>);
        Slot*                find_slot(ID<Property>, SlotFlags );
        const Slot*          find_slot(ID<Property>, SlotFlags ) const;
        Slot*                find_slot(const char* property_name, SlotFlags );
        const Slot*          find_slot(const char* property_name, SlotFlags ) const;
        size_t               get_slot_count(SlotFlags) const;
        Slot&                find_nth_slot( u8_t, SlotFlags );
        Slot*                get_first_slot(SlotFlags _way, const fw::type *_type);
        const fw::iinvokable*get_connected_invokable(const char *property_name) const; // TODO: can't remember to understand why I needed this...
        std::vector<SlotRef> filter_adjacent_slots(SlotFlags) const;
        std::vector<Slot*>   filter_slots(SlotFlags) const;
        bool                 has_edge_heading(ID<Property>) const;
        bool                 has_edge_heading(const char* name) const;
        Property*            get_prop_at(ID<Property>);
        const Property*      get_prop_at(ID<Property>) const;
        Property*            get_prop(const char* _name);
        const Property*      get_prop(const char* _name) const;
        std::vector<Slot*>   get_slots(const std::vector<ID<Property>>&, SlotFlags) const;
        std::vector<PoolID<Component>> get_components();
        void set_slot_capacity(SlotFlags _way, u8_t _n);
        ID<Property>         add_prop(const fw::type*, const char* /* name */, PropertyFlags = PropertyFlag_DEFAULT);
        ID8<Slot>            add_slot(ID<Property>, SlotFlags, u8_t _capacity = 1);
        Node*                last_child();
        bool                 has_input_connected( const ID<Property>& ) const;
        std::vector<Slot*>   get_all_slots( ID<Property> ) const;

        template<typename ValueT>
        ID<Property> add_prop(const char* _name, PropertyFlags _flags = PropertyFlag_DEFAULT)
        { return props.add<ValueT>(_name, _flags); }

        template<class ComponentT>
        void add_component(PoolID<ComponentT> component)
        { return m_components.add( component ); }

        template<class ComponentT>
        PoolID<ComponentT> get_component() const
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
