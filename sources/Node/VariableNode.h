#pragma once

#include <string>

#include <Nodable.h>    // forward declarations and common stuff
#include <Node.h>       // base class
#include <Member.h>

namespace Nodable{
	
	/**
		@brief The role of this class is to wrap a variable as a Node.

		The variable can be accessed through a single Member called "value".
		The value member can be linked to other node members.
	*/
	class VariableNode : public Node {
	public:
		VariableNode();
		~VariableNode() override = default;

		[[nodiscard]] inline bool             isSet()const { return value()->isDefined(); }
		[[nodiscard]] inline bool             isType(Type _type)const { return value()->isType(_type); }
		[[nodiscard]] inline const char*      getName()const { return m_name.c_str(); };
		[[nodiscard]] inline Member*          value()const { return m_props.get("value"); }
        [[nodiscard]] inline std::string      getTypeAsString ()const { return value()->getTypeAsString(); }
        [[nodiscard]] inline const Token*     getTypeToken() const { return m_typeToken; }
        [[nodiscard]] inline const Token*     getAssignmentOperatorToken() const { return m_assignmentOperatorToken; }
        [[nodiscard]] inline const Token*     getIdentifierToken() const { return m_identifierToken; }

        void        setName         (const char*);
        inline void setTypeToken(Token* token) { m_typeToken = token; }
        inline void setAssignmentOperatorToken(Token* token) { m_assignmentOperatorToken = token; }
        inline void setIdentifierToken(Token* token) { m_identifierToken = token; }
        template<class Value> inline void set(Value _value) { value()->set(_value); };
        template<class Value> inline void set(Value* _value){ value()->set(_value); };

    private:
        Token*      m_typeToken;
        Token*      m_assignmentOperatorToken;
        Token*      m_identifierToken;
		std::string m_name;

		MIRROR_CLASS(VariableNode)(
			MIRROR_PARENT(Node)
		);
    };
}