#pragma once

#include <nodable/core/reflection/R.h>
#include <memory> // std::shared_ptr
#include <nodable/core/types.h>
#include <nodable/core/Member.h> // for Type enum
#include <nodable/core/Visibility.h>
#include <nodable/core/Way.h>
#include <set>

namespace Nodable
{

    /**
     * @brief The Properties class is a Member* container for a given Node.
     * This class uses several indexes (by adress, name, insertion order).
     */
	class Properties
	{
	public:
		explicit Properties(Node* owner);
		virtual ~Properties();
        template<typename T>
        Member* add(const char* _name, Visibility _visibility, Way _flags )
        {
            NODABLE_ASSERT(!has(_name));
            auto v = Member::new_with_type<T>(this);
            v->set_name(_name);
            v->set_visibility(_visibility);
            v->set_allowed_connection(_flags);
            add_to_indexes(v);
            return v;
        }
		Member*             add(const char*, Visibility, std::shared_ptr<const R::MetaType>, Way);
		// void                remove(const char*); // if we implement that, we have to think about all indexes.
		// void                remove(Member*);     // => implement remove_from_indexes(Member*)
        bool                has(const char*);
		bool                has(const Member*);
		Member*             get(const char* _name)const { return m_members_by_name.at(_name); };
        Node*               get_owner()const { return m_owner; };
		const MemberMap&    by_name()const { return m_members_by_name; };
		const MemberVec&    by_id()const { return m_members_by_id; };
        Member*             get_first_member_with(Way, std::shared_ptr<const R::MetaType>) const;
	private:
	    void                add_to_indexes(Member*);
	    // void                remove_from_indexes(Member*);

	    Node*             m_owner;
        MemberMap         m_members_by_name; // tripple index, by name ...
        std::set<Member*> m_members;         // ... by adress ...
        MemberVec         m_members_by_id;   // ... and by id (insertion order).
	};
}