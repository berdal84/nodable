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
    // forward decl
    class InstructionNode;

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
        const PoolID<InstructionNode> get_declaration_instr()const { return m_declaration_instr; }
        PoolID<Scope>    get_scope();
        Slot&            get_value_slot(SlotFlags);
        void             set_declared(bool b = true) { m_is_declared = b; }
        void             reset_scope(Scope* _scope = nullptr);
        void             set_declaration_instr(PoolID<InstructionNode> _instr) { m_declaration_instr = _instr; }
        const fw::type*  type() const;
        fw::variant*     value();
        fw::variant& operator * () { return *property()->value(); }
        fw::variant* operator -> () { return property()->value(); }
        const fw::variant& operator * () const { return *property()->value(); }
        const fw::variant* operator -> () const { return property()->value(); }

    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        ID<Property>            m_value_property_id;
        bool                    m_is_declared;
        const fw::type*         m_type;
        PoolID<InstructionNode> m_declaration_instr;
        PoolID<Node>            m_scope;

		REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::VariableNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::VariableNode>, "Should be move constructible");
