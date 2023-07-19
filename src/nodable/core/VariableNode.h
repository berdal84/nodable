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
		explicit VariableNode(const fw::type *, const char*identifier);
		~VariableNode() override = default;

        /** Check if variable is declared (could be only a reference to an undeclared variable) */
		inline bool      is_declared()const { return m_is_declared; }
        /** Get variable's value (as a Property) */
        Property *       get_value()const { return m_value; }
        /** Get the instruction where this variable is declared */
        const InstructionNode* get_declaration_instr()const { return m_declaration_instr; }
        /** Get variable scope*/
        IScope*          get_scope() { return m_scope; }
        /** Write a new value into the variable*/
        template<class T> void         set(T _value) { m_value->set(_value); };
        /** Set the declared flag value. Parser need to know if a variable is declared or not to avoid duplicates*/
        void             set_declared(bool b = true) { m_is_declared = b; }
        /** Set the scope of the variable */
        void             set_scope(IScope* _scope) { m_scope = _scope; }
        /** Set the instruction where this variable is declared */
        void             set_declaration_instr(InstructionNode* _instr) { m_declaration_instr = _instr; }
    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        /** Variable's value is stored in this Property*/
        Property *             m_value;
        bool                   m_is_declared;
        InstructionNode*       m_declaration_instr;
        IScope*                m_scope;

		REFLECT_DERIVED_CLASS()
    };
}