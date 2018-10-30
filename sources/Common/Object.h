#pragma once
#include "Nodable.h"
#include "Value.h"   // for Type_ enum

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
		void                addMember         (const char* /*name*/, Visibility_ = Visibility_Default, Type_ = Type_Unknown);

		bool                hasMember         (Value* _value);

		/* Set deleted flag on. Will be deleted by its controller next frame */
		void                deleteNextFrame   (){deleted = true;}

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
		Value*              getMember         (const std::string& _name)const;

		/* Returns a pointer to the member value identified by its name.
		Or nullptr if this member doesn't exists. */
		Value*              getMember         (const char* _name)const;
		
		/* Return all members of this object */
		const Members&      getMembers        ()const;

		/* this method is automatically called when a member value changed */
		virtual void        onMemberValueChanged(const char* _name){};

		/* Set a new _value to the member _name.
		Side effect : set dirty all nodes connected directly or inderiectly to one of its outputs.*/
		template<typename T>
		void                setMember         (const char* _name, T _value);		

		bool                needsToBeDeleted  (){return deleted;}

		template<typename T>
		T                   getAs();

	private:
		Members             members;
		bool                deleted = false;
	};

	template<typename T>
	void Object::setMember      (const char* _name, T _value)
	{
		members[std::string(_name)]->setValue(_value);
		this->onMemberValueChanged(_name);	
	}

	template<typename T>
	T Object::getAs()
	{
		T casted;
		NODABLE_VERIFY(casted = dynamic_cast<T>(this));
		return casted;
	}
}