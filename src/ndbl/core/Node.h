#pragma once

#include <observe/event.h>
#include <string>
#include <memory>
#include <algorithm>

#include "tools/core/assertions.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "TComponentBag.h"
#include "DirectedEdge.h"
#include "Property.h"
#include "PropertyBag.h"
#include "constants.h"
#include "NodeComponent.h"
#include "SlotFlag.h"
#include "Slot.h"
#include "NodeType.h"

namespace ndbl
{
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

    typedef int NodeFlags;
    enum NodeFlag_
    {
        NodeFlag_NONE                = 0,
        NodeFlag_DEFAULT             = NodeFlag_NONE,
        NodeFlag_IS_DIRTY            = 1 << 0,
        NodeFlag_TO_DELETE           = 1 << 1,
        NodeFlag_ALL                 = ~NodeFlag_NONE,
    };
	/**
		The role of this class is to provide connectable Objects as Nodes.

		A node is an Object (composed with Properties) that can be linked
	    together in order to create graphs.

		Every Node has_flags a parent Graph. All nodes are built from a Graph,
	    which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
    class Node
	{
    public:


        // Data

        std::string       name;
        PropertyBag       props;
        Token             after_token;
        Graph*            parent_graph = nullptr;
        observe::Event<Node*> on_name_change;

        // Code
        Node() = default;
        virtual ~Node();

        void         init(NodeType type, const std::string& name);
        NodeType     type() const { return m_type; }
        bool         is_instruction() const;
        bool         is_unary_operator() const;
        bool         is_binary_operator() const;
        bool         can_be_instruction() const;
        bool         has_flags(NodeFlags flags)const { return (m_flags & flags) == flags; };
        void         set_flags(NodeFlags flags) { m_flags |= flags; }
        void         clear_flags(NodeFlags flags = NodeFlag_ALL) { m_flags &= ~flags; }

        // Slot related
        //-------------

        Slot*                add_slot(SlotFlags, size_t _capacity, size_t _position = 0);
        Slot*                add_slot(SlotFlags, size_t _capacity, Property*);
        void                 set_name(const char*);
        Node*                find_parent() const;
        size_t               adjacent_slot_count(SlotFlags )const;
        Slot&                get_slot_at(size_t);
        const Slot&          get_slot_at(size_t) const;
        Slot&                get_nth_slot(size_t, SlotFlags );
        std::vector<Slot*>   filter_slots( SlotFlags ) const;
        std::vector<Slot*>   filter_adjacent_slots(SlotFlags) const;
        Slot*                find_slot( SlotFlags ); // implicitly THIS_PROPERTY's slot
        const Slot*          find_slot( SlotFlags ) const; // implicitly THIS_PROPERTY's slot
        Slot*                find_slot_at( SlotFlags, size_t _position ); // implicitly THIS_PROPERTY's slot
        const Slot*          find_slot_at( SlotFlags, size_t _position ) const; // implicitly THIS_PROPERTY's slot
        Slot*                find_slot_by_property_name(const char* _property_name, SlotFlags );
        const Slot*          find_slot_by_property_name(const char* property_name, SlotFlags ) const;
        Slot*                find_slot_by_property_type(SlotFlags _way, const tools::type *_type);
        Slot*                find_slot_by_property(const Property*, SlotFlags );
        const Slot*          find_slot_by_property(const Property*, SlotFlags ) const;
        Slot*                find_adjacent_at(SlotFlags, size_t _index ) const;
        bool                 should_be_constrain_to_follow_output(const Node* _output ) const;
        size_t               slot_count(SlotFlags) const;
        std::vector<Slot*>&  slots() { return m_slots; }
        const std::vector<Slot*>& slots() const { return m_slots; }
        std::vector<Node*> filter_adjacent(SlotFlags) const;
        std::vector<Node*> successors() const;
        std::vector<Node*> rchildren() const; // reversed children
        std::vector<Node*> children() const;
        std::vector<Node*> inputs() const;
        std::vector<Node*> outputs() const;
        std::vector<Node*> predecessors() const;

        // Property related
        //-----------------

        Property*            add_prop(const tools::type*, const char* /* name */, PropertyFlags = PropertyFlag_NONE);
        Property*            get_prop_at(size_t);
        const Property*      get_prop_at(size_t) const;
        Property*            get_prop(const char* _name);
        const Property*      get_prop(const char* _name) const;
        const tools::IInvokable*get_connected_invokable(const char *property_name) const; // TODO: can't remember to understand why I needed this...
        bool                 has_input_connected( const Property*) const;

        template<typename ValueT>
        Property* add_prop(const char* _name, PropertyFlags _flags = PropertyFlag_NONE)
        { return props.add<ValueT>(_name, _flags); }

        // Component related
        //------------------

        std::vector<NodeComponent*> get_components();

        template<class C>
        void add_component(C* component)
        { return m_components.add( component ); }

        template<class C>
        const C* get_component() const
        { return static_cast<const C*>( m_components.get<C*>() );  }

        template<class C>
        C* get_component()
        { return const_cast<C*>( static_cast<const Node*>(this)->get_component<C>() ); }

        template<class C>
        bool has_component() const
        { return m_components.has<C*>(); }

        bool is_conditional() const;

    protected:
        NodeType           m_type             = NodeType_DEFAULT;
        NodeFlags          m_flags            = NodeFlag_DEFAULT;
        Property*          m_this_as_property = nullptr; // Short had for props.at( 0 )
        std::vector<Slot*> m_slots;
    private:
        TComponentBag<NodeComponent*> m_components;

        REFLECT_BASE_CLASS()
        POOL_REGISTRABLE(Node)
    };
}
