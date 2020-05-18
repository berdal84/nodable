#pragma once
#include "Nodable.h"
#include "Member.h"   // for Type_ enum
#include "mirror_macros.h"

namespace Nodable
{
	/*
		Object is the base class for all classes that needs to have members (std::string key => Nodable::Value value)
	*/

	class Object
	{
	public:
		Object();
		virtual ~Object();

		/* Adds a new member identified by its _name. */
		Member*             add         (const char*, Visibility_ = Default, Type_ = Type_Unknown, Connection_ = Connection_Default);

		bool                has         (Member* _value);

		/* Set deleted flag on. Will be deleted by its controller next frame */
		void                deleteNextFrame   (){deleted = true;}

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
		Member*              get         (const std::string& _name)const;

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
		Member*              get         (const char* _name)const;
		
		/* Return all members of this object */
		const Members&      getMembers        ()const;

		/* Return the first member that has this connection type (cf. Connection_ enum definition) or nullptr if no member is found.*/
		Member*             getFirstWithConn(Connection_)const;

		/* this method is automatically called when a member value changed */
		virtual void        onMemberValueChanged(const char* _name){};

		bool                needsToBeDeleted  (){return deleted;}

		template<typename T>
		void set(const char* _name, T _value)
		{
			members[std::string(_name)]->set(_value);
			this->onMemberValueChanged(_name);
		}

		template<typename T>
		T* as()
		{
			return reinterpret_cast<T*>(this);
		}

	private:
		Members             members;
		bool                deleted = false;

	public:
		MIRROR_CLASS(Object)();
	};	
}