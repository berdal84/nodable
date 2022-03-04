#pragma once

#include <nodable/R.h>

#include <nodable/Nodable.h>
#include <nodable/Member.h> // for Type enum
#include <nodable/Visibility.h>
#include <nodable/Way.h>

namespace Nodable
{

    /**
     * @brief The Properties class is a Member* container for a given Node.
     */
	class Properties
	{
	public:
		Properties(Node* owner);
		virtual ~Properties();
		Member*             add(const char*, Visibility, const R::Type*, Way);
        bool                has(const std::string& _name);
		bool                has(const Member* _value);
		Member*             get(const std::string& _name)const { return m_props.at(_name.c_str()); };
		Member*             get(const char* _name)const { return m_props.at(_name); };
        Node*               get_owner()const { return m_owner; };
		const Members&      get_members()const { return m_props; };
		Member*             get_first_member_with_conn(Way)const;

	private:
	    Node*    m_owner;
		Members  m_props;
	};
}