#pragma once
#include <memory>

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"
#include "ASTNode.h"

namespace ndbl
{
	class ASTFunctionNode : public ASTNode
    {
	public:
        void                        init(ASTNodeType node_type, const tools::FunctionDescriptor* func_type);
        Slot*                       get_arg_slot(size_t i) const { return m_argument_slot[i]; }
        const std::vector<Slot*>&   get_arg_slots() const { return m_argument_slot; }
		const tools::FunctionDescriptor*      get_func_type()const { return m_func_type; }
        const ASTToken&                get_identifier_token() const { return m_identifier_token; }
        void                        set_identifier_token(const ASTToken& tok) { m_identifier_token = tok; }
        Slot*                       lvalue_in() const { return m_argument_slot[0]; }
        Slot*                       rvalue_in() const { return m_argument_slot[1]; }
    protected:
        ASTToken                       m_identifier_token = ASTToken::s_null;
        const tools::FunctionDescriptor*      m_func_type; // not owned
        std::vector<Slot*>          m_argument_slot;
        std::vector<Property*>      m_argument_props;

        REFLECT_DERIVED_CLASS()
    };
}
