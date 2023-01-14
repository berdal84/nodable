#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include <nodable/core/types.h> // forward declarations and common stuff
#include <nodable/core/Node.h> // base class
#include <nodable/core/Member.h>
#include <nodable/core/reflection/reflection>
#include "Scope.h"

namespace ndbl
{
    // forward decl
    class InstructionNode;

	/**
		@brief VariableNode is a Node having a single Member and is identifiable by a name.
		The wrapped Member's name is Node::VALUE_MEMBER_NAME and can be linked to other members.
	*/
	class VariableNode : public Node
    {
        using token_ptr = std::shared_ptr<Token>;
        using token_cptr = std::shared_ptr<const Token>;
	public:
		explicit VariableNode(type);
		~VariableNode() override = default;

        /** Check if variable is declared (could be only a reference to an undeclared variable) */
		inline bool      is_declared()const { return m_is_declared; }
        /** Get variable identifier (ex: for "double toto;", "toto" is the identifier )*/
		const char*      get_identifier()const { return m_identifier.c_str(); };
        /** Get variable's value (as a Member) */
		Member*          get_value()const { return m_value; }
        /** Get the token for the variable's type (ex: for "double toto", { word: "double", suffix: " "} is the type token)*/
        token_cptr       get_type_token() const { return m_type_token; }
        /** Get the token for the assignment operator (ex: for "double toto = 10.0", { word: "=", prefix: " ", suffix: " "} is the assignment token)*/
        token_cptr       get_assignment_operator_token() const { return m_assignment_operator_token; }
        /** Get variable identifier (ex: for "double toto ;", { word: "toto", suffix: " ;"} is the identifier token )*/
        token_cptr       get_identifier_token() const { return m_identifier_token; }
        /** Get the instruction where this variable is declared */
        const InstructionNode* get_declaration_instr()const { return m_declaration_instr; }
        /** Get variable scope*/
        IScope*          get_scope() { return m_scope; }
        /** Set variable identifier (ex: for "double toto;", "toto" is the identifier )*/
        void             set_identifier(const char*);
        /** Set the token for the variable's type (ex: for "double toto", { word: "double", suffix: " "} is the type token)*/
        void             set_type_token(token_ptr token) { m_type_token = token; }
        /** Set the token for the assignment operator (ex: for "double toto = 10.0", { word: "=", prefix: " ", suffix: " "} is the assignment token)*/
        void             set_assignment_operator_token(token_ptr token) { m_assignment_operator_token = token; }
        /** Set variable identifier (ex: for "double toto ;", { word: "toto", suffix: " ;"} is the identifier token )*/
        void             set_identifier_token(token_ptr token) { m_identifier_token = token; }
        /** Write a new value into the variable*/
        template<class T> void         set(T _value) { m_value->set(_value); };
        /** Set the declared flag value. Parser need to know if a variable is declared or not to avoid duplicates*/
        void             set_declared(bool b = true) { m_is_declared = b; }
        /** Set the scope of the variable */
        void             set_scope(IScope* _scope) { m_scope = _scope; }
        /** Set the instruction where this variable is declared */
        void             set_declaration_instr(InstructionNode* _instr) { m_declaration_instr = _instr; }
    private:
        /** Variable's value is stored in this Member*/
	    Member*                m_value;
        bool                   m_is_declared;
        InstructionNode*       m_declaration_instr;
        token_ptr              m_type_token;
        token_ptr              m_assignment_operator_token;
        token_ptr              m_identifier_token;
		std::string            m_identifier;
        IScope*                m_scope;

		REFLECT_DERIVED_CLASS(Node)
    };
}