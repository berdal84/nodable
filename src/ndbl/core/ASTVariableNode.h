#pragma once

#include <string>
#include <memory> // std::shared_ptr

#include "tools/core/reflection/reflection"
#include "tools/core/types.h"

#include "ASTScope.h"
#include "ASTNode.h"
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
	class ASTVariableNode : public ASTNode
    {
	public:
		~ASTVariableNode() override {};

        void               init(const tools::TypeDescriptor* _type, const char* _identifier);
        bool               has_vflags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
        void               set_vflags(VariableFlags flags) { m_vflags |= flags; }
        void               clear_vflags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
        ASTScope*          get_scope();
        const ASTScope*    get_scope() const;
        void               reset_scope(ASTScope* _scope = nullptr);
        const tools::TypeDescriptor* get_type() const { return m_value->get_type(); }
        const ASTToken&       get_type_token() const { return m_type_token; }
        std::string        get_identifier() const { return get_identifier_token().word_to_string(); }
        const ASTToken&       get_identifier_token() const { return m_value->token(); }
        ASTToken&             get_identifier_token() { return m_value->token(); }
        const ASTToken&       get_operator_token() const { return m_operator_token; }
        void               set_type_token(const ASTToken& tok) { m_type_token = tok; }
        void               set_identifier_token(const ASTToken& tok) { m_value->set_token(tok); }
        void               set_operator_token(const ASTToken& tok) { m_operator_token = tok; }

        // Aliases

        Slot*              decl_out() { return m_as_declaration_slot; }
        const Slot*        decl_out() const { return m_as_declaration_slot; }
        Slot*              ref_out() { return m_as_reference_slot; }
        const Slot*        ref_out() const { return m_as_reference_slot; }

    private:
        ASTToken              m_type_token       = ASTToken::s_null; // [int] var  =
        ASTToken              m_operator_token   = ASTToken::s_null; //  int  var [=]
        VariableFlags      m_vflags           = VariableFlag_NONE;
        ASTNode*           m_scope            = nullptr;
        Slot*              m_as_declaration_slot = nullptr;
        Slot*              m_as_reference_slot   = nullptr;
		REFLECT_DERIVED_CLASS()
    };
}
