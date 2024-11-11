#pragma once

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
#include "Scope.h"

namespace ndbl
{
    // forward declarations
    class Graph;
    class NodeFactory;

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

		A child_node is an Object (composed with Properties) that can be linked
	    together in order to create_new graphs.

		Every Node has a parent Graph. All child_node are built from a Graph,
	    which first create an instance of this class (or derived) and then
		add some Component on it.
	*/
    class Node
	{
    public:
        friend class Graph;
        friend class NodeFactory;
        friend class Scope;

        // Code
        Node() = default;
        virtual ~Node();

        SIGNAL(on_destroy);
        SIGNAL(on_name_change, const char *);

        void                 init(NodeType type, const std::string& name);
        bool                 update();
        inline NodeType      type() const { return m_type; }
        bool                 is_invokable() const;
        inline bool          has_flags(NodeFlags flags)const { return (m_flags & flags) == flags; };
        inline void          set_flags(NodeFlags flags) { m_flags |= flags; }
        inline void          clear_flags(NodeFlags flags = NodeFlag_ALL) { m_flags &= ~flags; }
        inline Graph*        graph() { return m_graph; }
        inline const Graph*  graph() const { return m_graph; }
        inline const std::string& name() const { return m_name; };
        inline Token&        suffix() { return m_suffix; };
        inline const Token&  suffix() const { return m_suffix; };
        void                 set_suffix(const Token& token);
        const PropertyBag&   props() const;
        inline const Property* value() const { return m_value; }
        inline Property*     value() { return m_value; }
        Slot*                value_in();
        const Slot*          value_in() const;
        Slot*                value_out();
        const Slot*          value_out() const;
        Slot*                flow_in();
        const Slot*          flow_in() const;
        Slot*                flow_out();
        const Slot*          flow_out() const;
        bool                 is_a_scope() const { return m_is_a_scope; }
        Scope*               internal_scope() const { ASSERT(m_is_a_scope); return m_scope; }
        bool                 has_parent() const { return parent() != nullptr; }
        Scope*               parent() const { return m_is_a_scope ? m_scope->parent() : m_scope; };

        // Slot related
        //-------------

        Slot*                add_slot(Property *, SlotFlags, size_t _capacity, size_t _position = 0);
        void                 set_name(const char*);
        size_t               adjacent_slot_count(SlotFlags )const;
        Slot&                slot_at(size_t);
        const Slot&          slot_at(size_t) const;
        std::vector<Slot*>   filter_slots( SlotFlags ) const;
        std::vector<Slot*>   filter_adjacent_slots(SlotFlags) const;
        Slot*                find_slot( SlotFlags ); // implicitly DEFAULT_PROPERTY's slot
        const Slot*          find_slot( SlotFlags ) const; // implicitly DEFAULT_PROPERTY's slot
        Slot*                find_slot_at( SlotFlags, size_t _position ); // implicitly DEFAULT_PROPERTY's slot
        const Slot*          find_slot_at( SlotFlags, size_t _position ) const; // implicitly DEFAULT_PROPERTY's slot
        Slot*                find_slot_by_property_name(const char* name, SlotFlags );
        const Slot*          find_slot_by_property_name(const char* name, SlotFlags ) const;
        Slot*                find_slot_by_property_type(SlotFlags _way, const tools::TypeDescriptor *_type);
        Slot*                find_slot_by_property(const Property*, SlotFlags );
        const Slot*          find_slot_by_property(const Property*, SlotFlags ) const;
        Slot*                find_adjacent_at(SlotFlags, size_t _index ) const;
        size_t               slot_count(SlotFlags) const;
        std::vector<Slot*>&  slots() { return m_slots; }
        const std::vector<Slot*>& slots() const { return m_slots; }

        // cached adjacent nodes accessors

        inline const std::vector<Node*>& flow_outputs() const { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_OUT); }
        inline const std::vector<Node*>& inputs() const { return m_adjacent_nodes_cache.get( SlotFlag_INPUT ); }
        inline const std::vector<Node*>& outputs() const { return m_adjacent_nodes_cache.get( SlotFlag_OUTPUT ); }
        inline const std::vector<Node*>& flow_inputs() const { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_IN ); }

        // Property related
        //-----------------

        Property*            add_prop(const tools::TypeDescriptor*, const char* /* name */, PropertyFlags = PropertyFlag_NONE);
        Property*            get_prop(const char* _name);
        const Property*      get_prop(const char* _name) const;
        const tools::FunctionDescriptor* get_connected_function_type(const char *property_name) const; //
        bool                 has_input_connected( const Property*) const;

        template<typename ValueT>
        Property* add_prop(const char* _name, PropertyFlags _flags = PropertyFlag_NONE)
        { return m_props.add<ValueT>(_name, _flags); }

        // Component related
        //------------------

        std::vector<NodeComponent*> get_components();

        template<class C>
        void add_component(C* component)
        { return m_components.add( component ); }

        template<class C>
        C* get_component() const
        { return static_cast<C*>( m_components.get<C*>() );  }

        template<class C>
        C* get_component()
        { return const_cast<C*>( static_cast<const Node*>(this)->get_component<C>() ); }

        template<class C>
        bool has_component() const
        { return m_components.has<C*>(); }

        void               init_internal_scope();

    protected:
        void               on_slot_change(Slot::Event event, Slot *slot);

        std::string        m_name;
        PropertyBag        m_props;
        Token              m_suffix = Token{};
        Graph*             m_graph = nullptr;
        NodeType           m_type  = NodeType_DEFAULT;
        NodeFlags          m_flags = NodeFlag_IS_DIRTY;
        Property*          m_value = nullptr; // Short had for props.at( 0 )
        std::vector<Slot*> m_slots;
        Scope*             m_scope = nullptr; // parent_scope or internal_scope
        bool               m_is_a_scope = false;
        std::unordered_map<size_t,  std::vector<Slot*>> m_slots_by_property; // property's hash to Slots

        struct AdjacentNodesCache
        {
            const std::vector<Node*>& get(SlotFlags flags) const;
            void                      set_dirty() { _cache.clear(); }
            const Node* _node;
            std::unordered_map<SlotFlags, std::vector<Node*>> _cache;
        };

        AdjacentNodesCache m_adjacent_nodes_cache = {this};
    private:
        TComponentBag<NodeComponent*> m_components;

        REFLECT_BASE_CLASS()
        POOL_REGISTRABLE(Node)
    };
}
