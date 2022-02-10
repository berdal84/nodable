#pragma once

#include <string>

#include <nodable/Nodable.h> // forward declarations and common stuff
#include <nodable/Node.h> // base class
#include <nodable/Member.h>
#include <nodable/Reflect.h>

namespace Nodable{
	
	/**
		@brief The role of this class is to wrap a single Member as a Node identifiable with a name.

		The variable can be accessed through a single Member named "value".
		The value member can be linked to other node members.
	*/
	class VariableNode : public Node
    {
	public:
		explicit VariableNode(Reflect::Type);
		~VariableNode() override = default;

		[[nodiscard]] inline bool             is_declared()const { return m_type_token != nullptr; }
		[[nodiscard]] inline bool             is_defined()const { return m_value->is_defined(); }
		              inline void             undefine() { m_value->undefine(); setDirty(true); }
		[[nodiscard]] inline const char*      get_name()const { return m_name.c_str(); };
		[[nodiscard]] inline Member*          get_value()const { return m_value; }
        [[nodiscard]] inline const Token*     get_type_token() const { return m_type_token; }
        [[nodiscard]] inline const Token*     get_assignment_operator_token() const { return m_assignment_operator_token; }
        [[nodiscard]] inline const Token*     get_identifier_token() const { return m_identifier_token; }
		                     bool             eval() const override;
                             void             set_name(const char*);
                      inline void             set_type_token(Token* token) { m_type_token = token; }
                      inline void             set_assignment_operator_token(Token* token) { m_assignment_operator_token = token; }
                      inline void             set_identifier_token(Token* token) { m_identifier_token = token; }
        template<class T> inline void         set(T _value) { m_value->set(_value); };
        template<class T> inline void         set(T* _value){ m_value->set(_value); };

    private:
	    Member*     m_value;
        Token*      m_type_token;
        Token*      m_assignment_operator_token;
        Token*      m_identifier_token;
		std::string m_name;

		REFLECT_DERIVED(VariableNode)
        REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}