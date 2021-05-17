#pragma once

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Variant.h>
#include <nodable/Visibility.h>
#include <nodable/Token.h>
#include <nodable/Way.h>

#include <string>

namespace Nodable::core
{

    // forward declarations
    class Node;
    class Properties;

    /**
     * The role of a Member is to store a value in an underlying Variant object. A Member always has an Object as owner and
     * should do not exists alone.
     * Like for a regular class member in a lot of programming languages you can set the Visibility and the Type of a Member.
     * But in Nodable, a Member can also be connected (see Wire) to another Member on its input or output Connector.
     */
	class Member
    {
	public:
        Member();
		explicit Member(const std::string&);
        explicit Member(int);
        explicit Member(bool);
        explicit Member(double);
        explicit Member(const char *);
        ~Member();

        [[nodiscard]] bool allowsConnection(Way wayFlags)const { return (m_wayFlags & wayFlags) == wayFlags; }
		[[nodiscard]] bool hasInputConnected()const;
        [[nodiscard]] inline bool isDefined() const { return m_data.isDefined(); }
		[[nodiscard]] bool isType(Type type)const { return m_data.isType(type); }
        [[nodiscard]] bool equals(const Member *)const;

		void setConnectorWay(Way wayFlags) { m_wayFlags = wayFlags; }
		void setSourceExpression(const char* expr) { m_sourceExpression = expr; }
		void setInputMember(Member*);
		void setName(const char* name) { m_name = name; }
		void set(const Member* other) { m_data.set(&other->m_data); }
		void set(const Member& other) { m_data.set(&other.m_data); }
		void set(double);
        void set(const char*);
        void set(bool);
		inline void set(int val) { set((double)val); }
		void set(const std::string& val) { set(val.c_str());}
		void setType(Type type) { m_data.setType(type); }
		void setVisibility(Visibility _visibility) { m_visibility = _visibility; }
        void setSourceToken(const Token* _token);
        void setOwner(Node* _owner) { this->m_owner = _owner; }
        void setParentProperties(Properties *_parent) { this->m_parentProperties = _parent; }

		[[nodiscard]] inline Node*                 getOwner()const { return m_owner; };
        [[nodiscard]] inline Properties*           getParentProperties()const { return m_parentProperties; }
		[[nodiscard]] inline Member*               getInputMember()const { return m_inputMember; }
        [[nodiscard]] inline const std::string&    getName()const { return m_name; }
		[[nodiscard]] inline Type                  getType()const { return m_data.getType(); }
		[[nodiscard]] inline std::string           getTypeAsString()const { return m_data.getTypeAsString(); }
        [[nodiscard]] inline Visibility            getVisibility()const { return m_visibility; }
        [[nodiscard]] Way                          getConnectorWay()const { return m_wayFlags; }
        [[nodiscard]] inline const Token*          getSourceToken() const { return &m_sourceToken; }
        [[nodiscard]] inline Token*                getSourceToken() { return &m_sourceToken; }
        [[nodiscard]] inline const Variant*        getData()const { return &m_data; }

		inline explicit operator int()const       { return (int)this->m_data; }
        inline explicit operator bool()const      { return (bool)this->m_data;}
        inline explicit operator double()const    { return (double)this->m_data;}
		inline explicit operator std::string ()const{return (std::string)this->m_data;}

        /**
         * This member will digest another.
         * - source token ownership will be transfered to this
         * - _member will be deleted.
         * @param _member
         */
        void digest(Member *_member);

    private:
        Visibility 		  m_visibility;
        Node*             m_owner;
		Properties*       m_parentProperties;
		Member*           m_inputMember;
		Way               m_wayFlags;
        Token             m_sourceToken;
        std::string       m_sourceExpression;
		std::string       m_name;
		Variant       	  m_data;
    };
}