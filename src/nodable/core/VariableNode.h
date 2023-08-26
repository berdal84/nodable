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
        VariableNode(VariableNode&& other): Node(std::move(other)) {};
		explicit VariableNode(const fw::type *, const char* identifier);
		~VariableNode() override = default;
        VariableNode& operator=(VariableNode&&) = default;

        /** Check if variable is declared (could be only a reference to an undeclared variable) */
		inline bool      is_declared()const { return m_is_declared; }
        /** Get variable's value (as a Property) */
        Property*        property()const { return m_value; }
        /** Get the instruction where this variable is declared */
        const ID<InstructionNode> get_declaration_instr()const { return m_declaration_instr; }
        /** Get variable scope*/
        ID<Scope>        get_scope();
        /** Set the declared flag value. Parser need to know if a variable is declared or not to avoid duplicates*/
        void             set_declared(bool b = true) { m_is_declared = b; }
        /** Set the scope of the variable */
        void             reset_scope(Scope* _scope = nullptr);
        /** Set the instruction where this variable is declared */
        void             set_declaration_instr(ID<InstructionNode> _instr) { m_declaration_instr = _instr; }

        const fw::type*  type() const;
        fw::variant*     value();

        fw::variant& operator * () const { return *property()->value(); }
        fw::variant* operator -> () const { return property()->value(); }


    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        Property*           m_value;
        bool                m_is_declared;
        ID<InstructionNode> m_declaration_instr;
        ID<Node>            m_scope;

		REFLECT_DERIVED_CLASS()
    };
}

static_assert(std::is_move_assignable_v<ndbl::VariableNode>, "Should be move assignable");
static_assert(std::is_move_constructible_v<ndbl::VariableNode>, "Should be move constructible");
