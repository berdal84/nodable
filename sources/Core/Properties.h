#pragma once
#include "Nodable.h"
#include "Member.h"   // for Type enum
#include "mirror.h"
#include "Visibility.h"
#include "Way.h"

namespace Nodable
{

	class Properties
	{
	public:
		Properties(Node* owner);
		virtual ~Properties();
		Member*             add(const char*, Visibility = Visibility::Default, Type = Type_Any, Way = Way_Default);
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