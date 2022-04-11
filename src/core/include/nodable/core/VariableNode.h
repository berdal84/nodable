#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include <nodable/core/types.h> // forward declarations and common stuff
#include <nodable/core/Node.h> // base class
#include <nodable/core/Member.h>
#include <nodable/core/reflection/reflection>
#include "Scope.h"

namespace Nodable
{
    // forward decl
    class InstructionNode;

	/**
		@brief The role of this class is to wrap a single Member as a Node identifiable with a name.

		The variable can be accessed through a single Member named Node::VALUE_MEMBER_NAME.
		The value member can be linked to other node members.
	*/
	class VariableNode : public Node
    {
	public:
		explicit VariableNode(type);
		~VariableNode() override = default;

		inline bool      is_declared()const { return m_is_declared; }
		inline bool      is_initialized()const { return m_value->get_data()->is_initialized(); }
		inline void      set_initialized(bool _initialized) { m_value->get_data()->set_initialized(_initialized); set_dirty(true); }
		const char*      get_name()const { return m_name.c_str(); };
		Member*          get_value()const { return m_value; }
        bool             eval()const override;
        std::shared_ptr<const Token> get_type_token() const { return m_type_token; }
        std::shared_ptr<const Token> get_assignment_operator_token() const { return m_assignment_operator_token; }
        std::shared_ptr<const Token> get_identifier_token() const { return m_identifier_token; }
        void             set_name(const char*);
        void             set_type_token(std::shared_ptr<Token> token) { m_type_token = token; }
        void             set_assignment_operator_token(std::shared_ptr<Token> token) { m_assignment_operator_token = token; }
        void             set_identifier_token(std::shared_ptr<Token> token) { m_identifier_token = token; }
        template<class T> void         set(T _value) { m_value->set(_value); };
        template<class T> void         set(T* _value){ m_value->set(_value); };
        void             set_declared(bool b = true) { m_is_declared = b; }
        IScope*          get_scope() { return m_scope; }
        void             set_scope(IScope* _scope) { m_scope = _scope; }
        void             set_declaration_instr(InstructionNode* _instr) { m_declaration_instr = _instr; }
        const InstructionNode* get_declaration_instr()const { return m_declaration_instr; }
    private:
	    Member*     m_value;
        bool        m_is_declared;
        InstructionNode*       m_declaration_instr;
        std::shared_ptr<Token> m_type_token;
        std::shared_ptr<Token> m_assignment_operator_token;
        std::shared_ptr<Token> m_identifier_token;
		std::string m_name;
        IScope* m_scope;

		R_CLASS_DERIVED(VariableNode)
        R_CLASS_EXTENDS(Node)
        R_CLASS_END
    };
}