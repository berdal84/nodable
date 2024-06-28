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
	/**
		@brief VariableNode is a Node having a single Property and is identifiable by a name.
		The wrapped Property's name is Node::VALUE_MEMBER_NAME and can be linked to other properties.
	*/
	class VariableNode : public Node
    {
	public:
		VariableNode();
		explicit VariableNode(const tools::type *, const char* identifier);
		~VariableNode() override {};

        void             init() override;
		bool             is_declared()const { return m_is_declared; }
        bool             is_initialized() const { return m_is_initialized; };
        Property*        property();
        const Property*  property()const;
        Scope*           get_scope();
        void             set_declared(bool b = true) { m_is_declared = b; }
        void             reset_scope(Scope* _scope = nullptr);
        const tools::type*  get_value_type() const;
        tools::variant*     get_value();
        tools::variant& operator * () { return *property()->value(); }
        tools::variant* operator -> () { return property()->value(); }
        const tools::variant& operator * () const { return *property()->value(); }

        const tools::variant* operator -> () const { return property()->value(); }
        Slot       &input_slot();
        const Slot &input_slot() const;
        Slot       &output_slot();

        const Slot &output_slot() const;

        void initialize();

        void deinitialize();

    public:
        Token  type_token;
        Token  assignment_operator_token;
        Token  identifier_token;
    private:
        Property*   m_value_property{};
        bool        m_is_declared    = false;
        bool        m_is_initialized = false;
        Node*       m_scope{};
        const tools::type* m_type;

		REFLECT_DERIVED_CLASS()
    };
}
