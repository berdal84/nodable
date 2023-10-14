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

        explicit Node(std::string  _label = "UnnamedNode");
        Node(Node&&) noexcept ;
        Node& operator=(Node&&) noexcept ;
        virtual ~Node() = default;

        virtual void init();
        std::vector<PoolID<Node>> filter_adjacent(SlotFlags) const;
        std::vector<PoolID<Node>> successors() const;
        std::vector<PoolID<Node>> rchildren() const;
        std::vector<PoolID<Node>> children() const;
        std::vector<PoolID<Node>> inputs() const;
        std::vector<PoolID<Node>> outputs() const;
        std::vector<PoolID<Node>> predecessors() const;
        void                 set_name(const char*);
        PoolID<Node>         find_parent() const;
        size_t               adjacent_count(SlotFlags )const;
        ID8<Slot>            add_slot(SlotFlags, u8_t _capacity);
        ID8<Slot>            add_slot(SlotFlags, u8_t _capacity, ID<Property>);
        Slot&                get_slot_at(ID8<Slot>);
        size_t               get_slot_count(SlotFlags) const;
        Slot&                get_nth_slot(u8_t, SlotFlags );
        Slot*                find_slot( SlotFlags ); // implicitly THIS_PROPERTY's slot
        const Slot*          find_slot( SlotFlags ) const; // implicitly THIS_PROPERTY's slot
        Slot*                find_slot(ID<Property>, SlotFlags );
        const Slot*          find_slot(ID<Property>, SlotFlags ) const;
        Slot*                find_slot_by_name(const char* property_name, SlotFlags );
        const Slot*          find_slot_by_name(const char* property_name, SlotFlags ) const;
        Slot*                find_slot_by_type(SlotFlags _way, const fw::type *_type);
        std::vector<SlotRef> filter_adjacent_slots(SlotFlags) const;
        std::vector<Slot*>   filter_slots(SlotFlags) const;
        ID<Property>         add_prop(const fw::type*, const char* /* name */, PropertyFlags = PropertyFlag_DEFAULT);
        Property*            get_prop_at(ID<Property>);
        const Property*      get_prop_at(ID<Property>) const;
        Property*            get_prop(const char* _name);
        const Property*      get_prop(const char* _name) const;
        const fw::iinvokable*get_connected_invokable(const char *property_name) const; // TODO: can't remember to understand why I needed this...
        bool                 has_input_connected( const ID<Property>& ) const;
        std::vector<PoolID<Component>> get_components();

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

    protected:
        ID<Property> m_this_property_id;
    private:
        ComponentBag m_components;
    };
}

static_assert(std::is_move_assignable_v<ndbl::Node>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::Node>, "Should be move constructible");
