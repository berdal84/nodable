#pragma once

#include <string>
#include <memory>
#include <algorithm>

#include "tools/core/assertions.h"
#include "tools/core/memory/memory.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "constants.h"
#include "ASTSlotLink.h"
#include "ASTNodeProperty.h"
#include "ASTNodePropertyBag.h"
#include "ASTNodeSlotFlag.h"
#include "ASTNodeSlot.h"
#include "ASTNodeType.h"
#include "ASTScope.h"

namespace ndbl
{
    // forward declarations
    class Graph;
    class ASTNodeFactory;

    typedef int ASTNodeFlags;
    enum ASTNodeFlag_
    {
        ASTNodeFlag_NONE     = 0,
        ASTNodeFlag_IS_DIRTY = 1 << 0,
        ASTNodeFlag_ALL      = ~ASTNodeFlag_NONE,
        ASTNodeFlag_DEFAULT  = ASTNodeFlag_NONE,
    };

    class ASTNode : public tools::Component
	{
    public:
        DECLARE_REFLECT_override
        POOL_REGISTRABLE(ASTNode)

        friend class Graph;
        friend class ASTNodeFactory;
        friend class ASTScope;

        // Code
        ASTNode() {}
        ~ASTNode();

        SIGNAL(on_destroy);

        void                 init(ASTNodeType type, const std::string& name);
        bool                 update();
        ASTNodeType          type() const { return m_type; }
        bool                 is_invokable() const;
        bool                 is_expression() const;
        inline bool          has_flags(ASTNodeFlags flags)const { return (m_flags & flags) == flags; };
        inline void          set_flags(ASTNodeFlags flags) { m_flags |= flags; }
        inline void          clear_flags(ASTNodeFlags flags = ASTNodeFlag_ALL) { m_flags &= ~flags; }
        inline Graph*        graph() { return m_graph; }
        inline const Graph*  graph() const { return m_graph; }
        inline ASTToken&        suffix() { return m_suffix; };
        inline const ASTToken&  suffix() const { return m_suffix; };
        void                 set_suffix(const ASTToken& token);
        const ASTNodePropertyBag&   props() const;
        inline const ASTNodeProperty* value() const { return m_value; }
        inline ASTNodeProperty*     value() { return m_value; }
        ASTNodeSlot*                value_in();
        const ASTNodeSlot*          value_in() const;
        ASTNodeSlot*                value_out();
        const ASTNodeSlot*          value_out() const;
        ASTNodeSlot*                flow_in();
        const ASTNodeSlot*          flow_in() const;
        ASTNodeSlot*                flow_out();
        const ASTNodeSlot*          flow_out() const;
        bool                 is_orphan() const { return m_parent_scope == nullptr; }
        bool                 has_scope() const { return m_parent_scope != nullptr; }
        ASTScope*               scope() const { return m_parent_scope; };
        void                 init_internal_scope(size_t sub_scope_count = 0);
        bool                 has_internal_scope() const { return m_internal_scope != nullptr; }
        ASTScope*               internal_scope() const { return m_internal_scope; }

        // Slot related
        //-------------

        ASTNodeSlot*                add_slot(ASTNodeProperty *, SlotFlags, size_t _capacity, size_t _position = 0);
        size_t                      adjacent_slot_count(SlotFlags )const;
        ASTNodeSlot&                slot_at(size_t);
        const ASTNodeSlot&          slot_at(size_t) const;
        std::vector<ASTNodeSlot*>   filter_slots(SlotFlags ) const;
        std::vector<ASTNodeSlot*>   filter_adjacent_slots(SlotFlags) const;
        ASTNodeSlot*                find_slot(SlotFlags ); // implicitly DEFAULT_PROPERTY's slot
        const ASTNodeSlot*          find_slot(SlotFlags ) const; // implicitly DEFAULT_PROPERTY's slot
        ASTNodeSlot*                find_slot_at(SlotFlags, size_t _position ); // implicitly DEFAULT_PROPERTY's slot
        const ASTNodeSlot*          find_slot_at(SlotFlags, size_t _position ) const; // implicitly DEFAULT_PROPERTY's slot
        ASTNodeSlot*                find_slot_by_property_name(const char* name, SlotFlags );
        const ASTNodeSlot*          find_slot_by_property_name(const char* name, SlotFlags ) const;
        ASTNodeSlot*                find_slot_by_property_type(SlotFlags _way, const tools::TypeDescriptor *_type);
        ASTNodeSlot*                find_slot_by_property(const ASTNodeProperty*, SlotFlags );
        const ASTNodeSlot*          find_slot_by_property(const ASTNodeProperty*, SlotFlags ) const;
        ASTNodeSlot*                find_adjacent_at(SlotFlags, size_t _index ) const;
        size_t               slot_count(SlotFlags) const;
        std::vector<ASTNodeSlot*>&  slots() { return m_slots; }
        const std::vector<ASTNodeSlot*>& slots() const { return m_slots; }

        // cached adjacent nodes accessors

        bool                      has_flow_adjacent() const;
        const std::vector<ASTNode*>& flow_outputs() const { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_OUT); }
        const std::vector<ASTNode*>& inputs() const       { return m_adjacent_nodes_cache.get(SlotFlag_INPUT); }
        const std::vector<ASTNode*>& outputs() const      { return m_adjacent_nodes_cache.get(SlotFlag_OUTPUT); }
        const std::vector<ASTNode*>& flow_inputs() const  { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_IN); }

        // Property related
        //-----------------

        ASTNodeProperty*            add_prop(const tools::TypeDescriptor*, const char* /* name */, PropertyFlags = PropertyFlag_NONE);
        ASTNodeProperty*            get_prop(const char* _name);
        const ASTNodeProperty*      get_prop(const char* _name) const;
        const tools::FunctionDescriptor* get_connected_function_type(const char *property_name) const; //
        bool                 has_input_connected( const ASTNodeProperty*) const;

        template<typename ValueT>
        ASTNodeProperty* add_prop(const char* _name, PropertyFlags _flags = PropertyFlag_NONE)
        { return m_props.add<ValueT>(_name, _flags); }

    protected:
        void               on_slot_change(ASTNodeSlot::Event event, ASTNodeSlot *slot);

        ASTNodePropertyBag        m_props;
        ASTToken              m_suffix = ASTToken{};
        Graph*             m_graph = nullptr;
        ASTNodeType           m_type  = ASTNodeType_DEFAULT;
        ASTNodeFlags          m_flags = ASTNodeFlag_IS_DIRTY;
        ASTNodeProperty*          m_value = nullptr; // Short had for props.at( 0 )
        std::vector<ASTNodeSlot*> m_slots;
        ASTScope*             m_parent_scope = nullptr;
        ASTScope*             m_internal_scope = nullptr;
        std::unordered_map<size_t,  std::vector<ASTNodeSlot*>> m_slots_by_property; // property's hash to Slots

        struct AdjacentNodesCache
        {
            const std::vector<ASTNode*>& get(SlotFlags flags) const;
            void                      set_dirty() { _cache.clear(); }
            const ASTNode* _node;
            std::unordered_map<SlotFlags, std::vector<ASTNode*>> _cache;
        };

        AdjacentNodesCache m_adjacent_nodes_cache = {this};
    };
}
