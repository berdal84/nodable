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

        void               init(const tools::type* _val_type, const char* _identifier);
        bool               has_vflags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
        void               set_vflags(VariableFlags flags) { m_vflags |= flags; }
        void               clear_vflags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
        Property*          property();
        const Property*    get_value() const;
        Scope*             get_scope();
        void               reset_scope(Scope* _scope = nullptr);
        Slot&              input_slot();
        const Slot&        input_slot() const;
        Slot&              output_slot();
        const Slot&        output_slot() const;
        const tools::type* get_value_type() const;
        const Token&       get_type_token() const { return m_type_token; }
        const Token&       get_identifier_token() const { return m_identifier->get_token(); }
        Token&             get_identifier_token() { return m_identifier->get_token(); }
        const Token&       get_operator_token() const { return m_operator_token; }
        void               set_type_token(const Token& tok) { m_type_token = tok; }
        void               set_identifier_token(const Token& tok) { m_identifier->set_token(tok); }
        void               set_operator_token(const Token& tok) { m_operator_token = tok; }
    private:
        Token              m_type_token       = Token::s_null; // [int] var  =   42
        Property*          m_identifier       = nullptr;       //  int [var] =   42
        Token              m_operator_token   = Token::s_null; //  int  var [=]  42
        Property*          m_value            = nullptr;       //  int  var  =  [42]
        VariableFlags      m_vflags = VariableFlag_NONE;
        Node*              m_scope  = nullptr;

		REFLECT_DERIVED_CLASS()
    };
}
