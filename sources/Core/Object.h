#pragma once

#include <memory>
#include <map>
#include <string>

#include "mirror.h"

#include "Nodable.h"
#include "Visibility.h"
#include "Type.h"
#include "Way.h"
#include "Member.h"

namespace Nodable
{

	/*
		Object is the base class for all classes that needs to have members (std::string key => Nodable::Value value)
	*/

	class Object
	{
	public:
		Object();
		virtual ~Object() = default;

		/* Adds a new member identified by its _name. */
        std::shared_ptr<Member> add(const char*, Visibility = Visibility::Default, Type = Type::Any, Way = Way::Default);

		bool has(std::shared_ptr<Member> _value);

		/* Set deleted flag on. Will be deleted by its controller next frame */
		void deleteNextFrame   (){deleted = true;}

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
        [[nodiscard]] std::shared_ptr<Member> get(const std::string& _name)const;

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
        [[nodiscard]] std::shared_ptr<Member> get(const char* _name)const;
		
		/* Return all members of this object */
		[[nodiscard]] const std::map<std::string, std::shared_ptr<Member>>& getMembers()const;

		/* Return the first member that has this connection type (cf. Way enum definition) or nullptr if no member is found.*/
        [[nodiscard]] std::shared_ptr<Member> getFirstWithConn(Way)const;

		/* this method is automatically called when a member value changed */
		virtual void onMemberValueChanged(const char* _name){};

		[[nodiscard]] bool needsToBeDeleted  () const{return deleted;}

		template<typename T>
		void set(const char* _name, T _value)
		{
			members[std::string(_name)]->set(_value);
			this->onMemberValueChanged(_name);
		}

		template<typename T>
		T* as()
		{
			auto casted = dynamic_cast<T*>(this);
			NODABLE_ASSERT(casted != nullptr);
			return casted;
		}
	private:
		std::map<std::string, std::shared_ptr<Member>> members;
		bool                deleted = false;

	public:
		MIRROR_CLASS(Object)();
	};	
}