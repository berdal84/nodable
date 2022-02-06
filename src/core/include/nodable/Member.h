#pragma once

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Variant.h>
#include <nodable/Visibility.h>
#include <nodable/Token.h>
#include <nodable/Way.h>

#include <string>
#include <vector>

namespace Nodable
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
        explicit Member(Node*);
		explicit Member(const std::string&);
        explicit Member(int);
        explicit Member(bool);
        explicit Member(double);
        explicit Member(const char *);
        ~Member();

        [[nodiscard]] bool allowsConnection(Way wayFlags)const { return (m_wayFlags & wayFlags) == wayFlags; }
		[[nodiscard]] bool hasInputConnected()const;
        [[nodiscard]] inline bool isDefined() const { return get_variant().isDefined(); }
                      inline void undefine() { get_variant().undefine(); }
		[[nodiscard]] bool isType(Reflect::Type type)const { return get_variant().isType(type); }
        [[nodiscard]] bool equals(const Member *)const;

		void setConnectorWay(Way wayFlags) { m_wayFlags = wayFlags; }
		void setSourceExpression(const char* expr) { m_sourceExpression = expr; }
		void setInput(Member*, ConnBy_ _connect_by = ConnectBy_Ref);
		void setName(const char* name) { m_name = name; }
		void set(const Member* other) { get_variant().set(&other->m_variant); }
		void set(const Member& other) { get_variant().set(&other.m_variant); }
		void set(Node*);
		void set(double);
        void set(const char*);
        void set(bool);
		inline void set(int val) { set((double)val); }
		void set(const std::string& val) { set(val.c_str());}
		void setType(Reflect::Type type) { get_variant().setType(type); }
		void setVisibility(Visibility _visibility) { m_visibility = _visibility; }
        void setSourceToken(const Token* _token);
        void setOwner(Node* _owner) { this->m_owner = _owner; }
        void setParentProperties(Properties *_parent) { this->m_parentProperties = _parent; }

		[[nodiscard]] inline Node*                 getOwner()const { return m_owner; };
        [[nodiscard]] inline Properties*           getParentProperties()const { return m_parentProperties; }
		[[nodiscard]] inline Member*               getInput()const { return m_input; }
		[[nodiscard]] inline std::vector<Member*>& getOutputs() { return m_outputs; }
        [[nodiscard]] inline const std::string&    getName()const { return m_name; }
		[[nodiscard]] inline Reflect::Type         getType()const { return get_variant().getType(); }
		[[nodiscard]] inline std::string           getTypeAsString()const { return get_variant().getTypeAsString(); }
        [[nodiscard]] inline Visibility            getVisibility()const { return m_visibility; }
        [[nodiscard]] Way                          getConnectorWay()const { return m_wayFlags; }
        [[nodiscard]] inline const Token*          getSourceToken() const { return &m_sourceToken; }
        [[nodiscard]] inline Token*                getSourceToken() { return &m_sourceToken; }
        [[nodiscard]] inline const Variant*        getData()const { return &get_variant(); }

        template<typename T> inline explicit operator T*()     { return get_variant(); }
        template<typename T> inline explicit operator T()const { return get_variant().convert_to<T>(); }
        template<typename T> inline explicit operator T&()     { return get_variant(); }
        template<typename T> inline T convert_to()const        { return get_variant().convert_to<T>(); }

        /**
         * This member will digest another.
         * - source token ownership will be transfered to this
         * - _member will be deleted.
         * @param _member
         */
        void digest(Member *_member);

        bool is_connected_by(ConnBy_ by);

        void define();

    private:
        // TODO: implem AbstractMember, implement Value and Reference, remove this get_variant()
        Variant& get_variant(){ return (m_input && m_connected_by == ConnectBy_Ref) ? m_input->m_variant : m_variant; }
        const Variant& get_variant()const{ return (m_input && m_connected_by == ConnectBy_Ref) ? m_input->m_variant : m_variant; }

        Member*           m_input;
        ConnBy_           m_connected_by;
        Visibility 		  m_visibility;
        Node*             m_owner;
		Properties*       m_parentProperties;
		std::vector<Member*> m_outputs;
		Way               m_wayFlags;
        Token             m_sourceToken;
        std::string       m_sourceExpression;
		std::string       m_name;
		Variant       	  m_variant;
    };
}