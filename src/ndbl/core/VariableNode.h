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
        VariableFlag_INITIALIZED = 1 << 1
    };

	/**
		@brief VariableNode is a Node having a single Property and is identifiable by a name.
		The wrapped Property's name is Node::VALUE_MEMBER_NAME and can be linked to other properties.
	*/
	class VariableNode : public Node
    {
	public:
        typedef tools::type    type;
        typedef tools::variant variant;

		VariableNode();
		explicit VariableNode(const tools::type *, const char* identifier);
		~VariableNode() override {};

        void             init() override;
        bool             has_flags(VariableFlags flags)const { return (m_flags & flags) == flags; };
        void             set_flags(VariableFlags flags) { m_flags |= flags; }
        void             clear_flags(VariableFlags flags) { m_flags &= ~flags; }
        Property*        property();
        const Property*  property()const;
        Scope*           get_scope();
        void             reset_scope(Scope* _scope = nullptr);
        const tools::type*get_value_type() const;
        variant*         get_value();

        variant& operator * () { return *property()->value(); }
        variant* operator -> () { return property()->value(); }
        const variant& operator * () const { return *property()->value(); }
        const variant* operator -> () const { return property()->value(); }

        Slot       &input_slot();
        const Slot &input_slot() const;
        Slot       &output_slot();
        const Slot &output_slot() const;

    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        Property*          m_value_property{};
        VariableFlags      m_flags = VariableFlag_NONE;
        Node*              m_scope = nullptr;
        const tools::type* m_type  = tools::type::any();

		REFLECT_DERIVED_CLASS()
    };
}
