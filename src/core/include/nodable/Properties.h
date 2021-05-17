#pragma once

#include <mirror.h>

#include <nodable/Nodable.h>
#include <nodable/Member.h> // for Type enum
#include <nodable/Visibility.h>
#include <nodable/Way.h>

namespace Nodable::core
{

	class Properties
	{
	public:
		Properties(Node* owner);
		virtual ~Properties();
		Member*             add(const char*, Visibility, Type, Way);
		bool                has(const Member* _value);
		Member*             get(const std::string& _name)const;
		Member*             get(const char* _name)const;
		const Members&      getMembers()const;
		Member*             getFirstWithConn(Way)const;

	private:
	    Node*    m_owner;
		Members  m_props;
	};
}