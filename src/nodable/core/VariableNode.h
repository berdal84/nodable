#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"

#include "Scope.h"
#include "Node.h"
#include "Property.h"

namespace ndbl
{
	/**
		@brief VariableNode is a Node having a single Property and is identifiable by a name.
		The wrapped Property's name is Node::VALUE_MEMBER_NAME and can be linked to other properties.
	*/
	class VariableNode : public Node
    {
	public:
		VariableNode();
        VariableNode(VariableNode&&) = default;
		explicit VariableNode(const fw::type *, const char* identifier);
		~VariableNode() override = default;
        VariableNode& operator=(VariableNode&&) = default;

        void             init() override;
		bool             is_declared()const { return m_is_declared; }
        Property*        property();
        const Property*  property()const;
        const PoolID<Node> get_declaration_instr()const { return m_declaration_instr; }
        PoolID<Scope>    get_scope();
        Slot *find_value_typed_slot( SlotFlags _flags );
        void             set_declared(bool b = true) { m_is_declared = b; }
        void             reset_scope(Scope* _scope = nullptr);
        void             set_declaration_instr(PoolID<Node> _instr) { FW_ASSERT(_instr->is_instruction()); m_declaration_instr = _instr; }
        const fw::type*  type() const;
        fw::variant*     value();
        fw::variant& operator * () { return *property()->value(); }
        fw::variant* operator -> () { return property()->value(); }
        const fw::variant& operator * () const { return *property()->value(); }
        const fw::variant* operator -> () const { return property()->value(); }

        const Slot &input_slot() const;

    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        ID<Property>            m_value_property_id;
        bool                    m_is_declared;
        const fw::type*         m_type;
        PoolID<Node>            m_declaration_instr;
        PoolID<Node>            m_scope;

		REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::VariableNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::VariableNode>, "Should be move constructible");
