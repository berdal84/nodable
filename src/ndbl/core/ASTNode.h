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
#include "ASTNodeComponent.h"
#include "SlotFlag.h"
#include "Slot.h"
#include "ASTNodeType.h"

namespace ndbl
{
    // forward declarations
    class ASTGraph;

    /**
     * Distinguish between all possible update_world_matrix result
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
	    together in order to create_new graphs.

		Every Node has_flags a parent Graph. All nodes are built from a Graph,
	    which first create_new an instance of this class (or derived) and then
		add some Component on it.
	*/
    class ASTNode
	{
    public:
        friend ASTGraph;
        
        // Code
        ASTNode() = default;
        virtual ~ASTNode();

        void                 init(ASTNodeType type, const std::string& name);
        ASTNodeType          type() const { return m_type; }
        bool                 is_conditional() const;
        bool                 is_instruction() const;
        bool                 is_unary_operator() const;
        bool                 is_binary_operator() const;
        bool                 is_invokable() const;
        bool                 can_be_instruction() const;
        bool                 has_flags(NodeFlags flags)const { return (m_flags & flags) == flags; };
        void                 set_flags(NodeFlags flags) { m_flags |= flags; }
        void                 clear_flags(NodeFlags flags = NodeFlag_ALL) { m_flags &= ~flags; }
        ASTGraph*            graph() { return m_graph; }
        const ASTGraph*      graph() const { return m_graph; }
        const std::string&   name() const { return m_name; };
        ASTToken&               suffix() { return m_suffix; };
        const ASTToken&         suffix() const { return m_suffix; };
        void                 set_suffix(const ASTToken& token);
        const PropertyBag&   props() const;
        observe::Event<ASTNode*>& on_name_change() { return m_on_name_change; };
        const Property*      value() const { return m_value; }
        Property*            value() { return m_value; }
        Slot*                value_in();
        const Slot*          value_in() const;
        Slot*                value_out();
        const Slot*          value_out() const;

        // Slot related
        //-------------

        Slot*                add_slot(Property *, SlotFlags, size_t _capacity, size_t _position = 0);
        void                 set_name(const char*);
        ASTNode*             find_parent() const;
        size_t               adjacent_slot_count(SlotFlags )const;
        Slot&                slot_at(size_t);
        const Slot&          slot_at(size_t) const;
        Slot&                nth_slot(size_t, SlotFlags );
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
        bool                 should_be_constrain_to_follow_output(const ASTNode* _output ) const;
        size_t               slot_count(SlotFlags) const;
        std::vector<Slot*>&  slots() { return m_slots; }
        const std::vector<Slot*>& slots() const { return m_slots; }
        std::vector<ASTNode*>   filter_adjacent(SlotFlags) const;
        std::vector<ASTNode*>   successors() const;
        std::vector<ASTNode*>   rchildren() const; // reversed children
        std::vector<ASTNode*>   children() const;
        std::vector<ASTNode*>   inputs() const;
        std::vector<ASTNode*>   outputs() const;
        std::vector<ASTNode*>   predecessors() const;

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

        std::vector<ASTNodeComponent*> get_components();

        template<class C>
        void add_component(C* component)
        { return m_components.add( component ); }

        template<class C>
        C* get_component() const
        { return static_cast<C*>( m_components.get<C*>() );  }

        template<class C>
        C* get_component()
        { return const_cast<C*>( static_cast<const ASTNode*>(this)->get_component<C>() ); }

        template<class C>
        bool has_component() const
        { return m_components.has<C*>(); }

    protected:

        std::string        m_name;
        PropertyBag        m_props;
        ASTToken              m_suffix;
        ASTGraph*          m_graph = nullptr;
        ASTNodeType        m_type  = ASTNodeType_DEFAULT;
        NodeFlags          m_flags = NodeFlag_DEFAULT;
        Property*          m_value = nullptr; // Short had for props.at( 0 )
        std::vector<Slot*> m_slots;
        observe::Event<ASTNode*> m_on_name_change;
    private:
        TComponentBag<ASTNodeComponent*> m_components;

        REFLECT_BASE_CLASS()
        POOL_REGISTRABLE(ASTNode)
    };
}
