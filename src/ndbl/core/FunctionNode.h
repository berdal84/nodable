#pragma once
#include <memory>

#include "tools/core/types.h"
#include "tools/core/reflection/reflection"
#include "Node.h"

namespace ndbl
{
	class FunctionNode : public Node
    {
	public:
        void                        init(NodeType node_type, const tools::FunctionDescriptor* func_type);
        const std::vector<Slot*>&   get_arg_slots() const { return m_argument_slot; }
		const tools::FunctionDescriptor*      get_func_type()const { return m_func_type; }
        const Token&                get_identifier_token() const { return m_identifier_token; }
        void                        set_identifier_token(const Token& tok) { m_identifier_token = tok; }
        Slot*                       get_lvalue() const { return m_argument_slot[0]; }
        Slot*                       get_rvalue() const { return m_argument_slot[1]; }
    protected:
        Token                       m_identifier_token = Token::s_null;
        const tools::FunctionDescriptor*      m_func_type; // not owned
        std::vector<Slot*>          m_argument_slot;
        std::vector<Property*>      m_argument_props;

        REFLECT_DERIVED_CLASS()
    };
}
