#pragma once

#include <nodable/Nodable.h> // for constants and forward declarations
#include <nodable/Variant.h>
#include <nodable/Visibility.h>
#include <nodable/Token.h>
#include <nodable/Way.h>

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

namespace Nodable
{
    // forward declarations
    class Node;
    class Properties;

    /**
     * @brief The role of a Member is to store a value for a Properties object (its parent).
     *
     * Like for a regular class member in a lot of programming languages, you can set visibility and type.
     * But in Nodable, a Member can also be connected (see Wire) to another Member using their input/outputs.
     */
	class Member
    {
	public:
        explicit Member(Properties*);
        explicit Member(Properties*, Node*);
		explicit Member(Properties*, const std::string&);
        explicit Member(Properties*, int);
        explicit Member(Properties*, bool);
        explicit Member(Properties*, double);
        explicit Member(Properties*, const char *);
        ~Member();

        void digest(Member *_member);
        void define();
        void undefine() { get_variant().undefine(); }
        bool is_defined() const { return get_variant().is_defined(); }
        bool is_connected_by(ConnBy_ by);
        bool is_type(std::shared_ptr<const R::Type> _type)const { return get_variant().is(_type); }
        bool equals(const Member *)const;
        bool allows_connection(Way _flag)const { return (m_allowed_connection & _flag) == _flag; }
        bool has_input_connected()const;

		void set_allowed_connection(Way wayFlags) { m_allowed_connection = wayFlags; }
		void set_input(Member*, ConnBy_ _connect_by = ConnectBy_Ref);
		void set_name(const char* _name) { m_name = _name; }
		void set(const Member* _other) { get_variant().set(&_other->m_variant); }
		void set(const Member& _other) { get_variant().set(&_other.m_variant); }
		void set(Node*);
		void set(double);
        void set(const char*);
        void set(bool);
		void set(int val) { set((double)val); }
		void set(const std::string& _val) { set(_val.c_str());}
		void set_type(std::shared_ptr<const R::Type> _type) { get_variant().set_type(_type); }
		void set_visibility(Visibility _visibility) { m_visibility = _visibility; }
        void set_src_token(const std::shared_ptr<Token> _token);
        void set_owner(Node* _owner) { m_owner = _owner; }

		Node*                 get_owner()const { return m_owner; };
		Member*               get_input()const { return m_input; }
		std::vector<Member*>& get_outputs() { return m_outputs; }
        const std::string&    get_name()const { return m_name; }
        std::shared_ptr<const R::Type> get_type()const { return get_variant().get_type(); }
        Visibility            get_visibility()const { return m_visibility; }
        Way                   get_allowed_connection()const { return m_allowed_connection; }
        const std::shared_ptr<Token> get_src_token() const { return m_sourceToken; }
		std::shared_ptr<Token>       get_src_token() { return m_sourceToken; }
        const Variant*        get_data()const { return &get_variant(); }

        template<typename T> inline explicit operator T*()     { return get_variant(); }
        template<typename T> inline explicit operator const T*() const { return get_variant(); }
        template<typename T> inline explicit operator T()const { return get_variant().convert_to<T>(); }
        template<typename T> inline explicit operator T&()     { return get_variant(); }
        template<typename T> inline T convert_to()const        { return get_variant().convert_to<T>(); }

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
		Way               m_allowed_connection;
		std::shared_ptr<Token> m_sourceToken;
		std::string       m_name;
		Variant       	  m_variant;
    };
}