#pragma once

#include <nodable/core/reflection/R.h>
#include <memory> // std::shared_ptr
#include <nodable/core/types.h>
#include <nodable/core/Member.h> // for Type enum
#include <nodable/core/Visibility.h>
#include <nodable/core/Way.h>

namespace Nodable
{

    /**
     * @brief The Properties class is a Member* container for a given Node.
     */
	class Properties
	{
	public:
		explicit Properties(Node* owner);
		virtual ~Properties();
        template<typename T>
        Member* add(const char* _name, Visibility _visibility, Way _flags )
        {
            auto v = Member::new_with_type<T>(this);
            v->set_name(_name);
            v->set_visibility(_visibility);
            v->set_allowed_connection(_flags);
            m_props[std::string(_name)] = v;
            return v;
        }
		Member*             add(const char*, Visibility, std::shared_ptr<const R::MetaType>, Way);
        bool                has(const char* _name);
		bool                has(const Member*);
		Member*             get(const char* _name)const { return m_props.at(_name); };
        Node*               get_owner()const { return m_owner; };
		const Members&      get_members()const { return m_props; };
        Member*             get_first_member_with(Way, std::shared_ptr<const R::MetaType>) const;
	private:
	    Node*    m_owner;
		Members  m_props;
	};
}