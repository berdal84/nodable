#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "Scope.h"
#include "Node.h"
#include "Property.h"

namespace ndbl
{

    typedef int VariableFlags;
    enum VariableFlags_
    {
        VariableFlag_NONE = 0,
        VariableFlag_DECLARED    = 1 << 0,
        VariableFlag_INITIALIZED = 1 << 1,
        VariableFlag_ALL         = ~VariableFlag_NONE
    };

	/**
		@brief VariableNode is a Node having a single Property and is identifiable by a name.
		The wrapped Property's name is Node::VALUE_MEMBER_NAME and can be linked to other properties.
	*/
	class VariableNode : public Node
    {
	public:
		~VariableNode() override {};

        void               init(const tools::TypeDesc* _type, const char* _identifier);
        bool               has_vflags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
        void               set_vflags(VariableFlags flags) { m_vflags |= flags; }
        void               clear_vflags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
        Property*          property();
        const Property*    get_value() const;
        Scope*             get_scope();
        void               reset_scope(Scope* _scope = nullptr);
        Slot&              input_slot(); // input slot for variable initialisation
        const Slot&        input_slot() const; // input slot for variable initialisation
        Slot&              output_slot(); // output slot to reference this variable
        const Slot&        output_slot() const; // output slot to reference this variable
        const tools::TypeDesc* get_type() const { return m_identifier->get_type(); }
        const Token&       get_type_token() const { return m_type_token; }
        std::string        get_identifier() const { return get_identifier_token().word_to_string(); }
        const Token&       get_identifier_token() const { return m_identifier->get_token(); }
        Token&             get_identifier_token() { return m_identifier->get_token(); }
        const Token&       get_operator_token() const { return m_operator_token; }
        void               set_type_token(const Token& tok) { m_type_token = tok; }
        void               set_identifier_token(const Token& tok) { m_identifier->set_token(tok); }
        void               set_operator_token(const Token& tok) { m_operator_token = tok; }
    private:
        Token              m_type_token       = Token::s_null; // [int] var  =
        Property*          m_identifier       = nullptr;       //  int [var] =
        Token              m_operator_token   = Token::s_null; //  int  var [=]
        VariableFlags      m_vflags = VariableFlag_NONE;
        Node*              m_scope  = nullptr;

		REFLECT_DERIVED_CLASS()
    };
}
