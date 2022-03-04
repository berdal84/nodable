#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include <nodable/Nodable.h> // forward declarations and common stuff
#include <nodable/Node.h> // base class
#include <nodable/Member.h>
#include <nodable/R.h>

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
		explicit VariableNode(std::shared_ptr<const R::Type>);
		~VariableNode() override = default;

		[[nodiscard]] inline bool             is_declared()const { return m_is_declared; }
		[[nodiscard]] inline bool             is_defined()const { return m_value->is_defined(); }
		              inline void             undefine() { m_value->undefine();
                          set_dirty(true); }
		[[nodiscard]] inline const char*      get_name()const { return m_name.c_str(); };
		[[nodiscard]] inline Member*          get_value()const { return m_value; }
        [[nodiscard]] inline std::shared_ptr<const Token> get_type_token() const { return m_type_token; }
        [[nodiscard]] inline std::shared_ptr<const Token> get_assignment_operator_token() const { return m_assignment_operator_token; }
        [[nodiscard]] inline std::shared_ptr<const Token> get_identifier_token() const { return m_identifier_token; }
		                     bool             eval() const override;
                             void             set_name(const char*);
                      inline void             set_type_token(std::shared_ptr<Token> token) { m_type_token = token; }
                      inline void             set_assignment_operator_token(std::shared_ptr<Token> token) { m_assignment_operator_token = token; }
                      inline void             set_identifier_token(std::shared_ptr<Token> token) { m_identifier_token = token; }
        template<class T> inline void         set(T _value) { m_value->set(_value); };
        template<class T> inline void         set(T* _value){ m_value->set(_value); };
        void                                  set_declared(bool b = true) { m_is_declared = b; } 
    private:
	    Member*     m_value;
        bool        m_is_declared;
        std::shared_ptr<Token> m_type_token;
        std::shared_ptr<Token> m_assignment_operator_token;
        std::shared_ptr<Token> m_identifier_token;
		std::string m_name;

		R_DERIVED(VariableNode)
        R_EXTENDS(Node)
        R_END
    };
}