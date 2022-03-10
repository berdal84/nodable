#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include <nodable/Nodable.h> // forward declarations and common stuff
#include <nodable/Node.h> // base class
#include <nodable/Member.h>
#include <nodable/R.h>
#include "Scope.h"

namespace Nodable
{
	/**
		@brief The role of this class is to wrap a single Member as a Node identifiable with a name.

		The variable can be accessed through a single Member named Node::VALUE_MEMBER_NAME.
		The value member can be linked to other node members.
	*/
	class VariableNode : public Node
    {
	public:
		explicit VariableNode(std::shared_ptr<const R::MetaType>);
		~VariableNode() override = default;

		inline bool      is_declared()const { return m_is_declared; }
		inline bool      is_defined()const { return m_value->is_defined(); }
		inline void      undefine() { m_value->undefine(); set_dirty(true); }
		const char*      get_name()const { return m_name.c_str(); };
		Member*          get_value()const { return m_value; }
        std::shared_ptr<const Token> get_type_token() const { return m_type_token; }
        std::shared_ptr<const Token> get_assignment_operator_token() const { return m_assignment_operator_token; }
        std::shared_ptr<const Token> get_identifier_token() const { return m_identifier_token; }
		bool             eval() const override;
        void             set_name(const char*);
        void             set_type_token(std::shared_ptr<Token> token) { m_type_token = token; }
        void             set_assignment_operator_token(std::shared_ptr<Token> token) { m_assignment_operator_token = token; }
        void             set_identifier_token(std::shared_ptr<Token> token) { m_identifier_token = token; }
        template<class T> void         set(T _value) { m_value->set(_value); };
        template<class T> void         set(T* _value){ m_value->set(_value); };
        void             set_declared(bool b = true) { m_is_declared = b; }
        IScope*          get_scope() { return m_scope; }
        void             set_scope(IScope* _scope) { m_scope = _scope; }
    private:
	    Member*     m_value;
        bool        m_is_declared;
        std::shared_ptr<Token> m_type_token;
        std::shared_ptr<Token> m_assignment_operator_token;
        std::shared_ptr<Token> m_identifier_token;
		std::string m_name;
        IScope* m_scope;

		R_DERIVED(VariableNode)
        R_EXTENDS(Node)
        R_END
    };
}