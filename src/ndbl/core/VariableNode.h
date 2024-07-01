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

    public:
        Token              type_token                = Token::s_null;
        Token              assignment_operator_token = Token::s_null;
        Token              identifier_token          = Token::s_null;
    private:
        Property*          m_value  = nullptr;
        VariableFlags      m_vflags = VariableFlag_NONE;
        Node*              m_scope  = nullptr;

		REFLECT_DERIVED_CLASS()
    };
}
