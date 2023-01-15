#pragma once

#include <nodable/core/types.h> // for constants and forward declarations
#include <nodable/core/reflection/variant.h>
#include <nodable/core/Visibility.h>
#include <nodable/core/Token.h>
#include <nodable/core/Way.h>

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

namespace ndbl
{
    // forward declarations
    class Node;
    class PropertyGrp;

    /**
     * @class The class store a value (as a variant) and is owned by a PropertyGroup
     *
     * A property is like a property in OOP, you can set its visibility, type, and value.
     * In Nodable, a property can also be connected (see DirectedEdge) to another property is they "way" allows it.
     */
	class Property
    {
    public:
        template<typename T>
        explicit Property(PropertyGrp * _parent_properties, T* _value): Property(_parent_properties) { m_variant.set(_value); };
        explicit Property(PropertyGrp * = nullptr);
        explicit Property(PropertyGrp *, const std::string&);
        explicit Property(PropertyGrp *, int);
        explicit Property(PropertyGrp *, bool);
        explicit Property(PropertyGrp *, double);
        explicit Property(PropertyGrp *, const char *);
        ~Property();

        void digest(Property *_property);
        bool is_connected_by_ref() const;
        bool allows_connection(Way _flag)const { return (m_allowed_connection & _flag) == _flag; }
        bool has_input_connected()const;
		void set_allowed_connection(Way wayFlags) { m_allowed_connection = wayFlags; }
		void set_input(Property *);
		void set_name(const char* _name) { m_name = _name; }
        void set(Node* _value);
        void set(const Property & _other) { get_pointed_variant() = _other.m_variant; }
        template<typename T> void set(T _value)
        {
            get_pointed_variant().set(_value);
        }

		void set_type(type _type) { get_pointed_variant().ensure_is_type(_type); }
		void set_visibility(Visibility _visibility) { m_visibility = _visibility; }
        void set_src_token(const std::shared_ptr<Token> _token);
        void set_owner(Node* _owner) { m_owner = _owner; }

		Node*                 get_owner()const { return m_owner; };
        Property *               get_input()const { return m_input; }
		std::vector<Property *>& get_outputs() { return m_outputs; }
        const std::string&    get_name()const { return m_name; }
        const type&           get_type()const { return get_pointed_variant().get_type(); }
        Visibility            get_visibility()const { return m_visibility; }
        Way                   get_allowed_connection()const { return m_allowed_connection; }
        const std::shared_ptr<Token> get_src_token() const { return m_sourceToken; }
		std::shared_ptr<Token>       get_src_token() { return m_sourceToken; }
        const variant*        get_variant()const { return &get_pointed_variant(); }
        variant*              get_variant() { return &get_pointed_variant(); }


        template<typename T> inline explicit operator T*()     { return get_pointed_variant(); }
        template<typename T> inline explicit operator const T*() const { return get_pointed_variant(); }
        template<typename T> inline explicit operator T()const { return get_pointed_variant().convert_to<T>(); }
        template<typename T> inline explicit operator T&()     { return get_pointed_variant(); }
        template<typename T> inline T convert_to()const        { return get_pointed_variant().convert_to<T>(); }

        typedef int Flags;
        enum Flags_ {
            Flags_none         = 0,
            Flags_initialize   = 1,
            Flags_define       = 1 << 1,
            Flags_reset_value  = 1 << 2
		};

        void             ensure_is_defined(bool _value);
        bool             is_connected_to_variable() const;
        VariableNode*    get_connected_variable();
        qword&           get_underlying_data();

		static Property *                new_with_type(PropertyGrp * , type , Flags = Flags_none);
		static std::vector<variant*>& get_variant(std::vector<Property *> _in, std::vector<variant*>& _out);
    private:

        // TODO: implem AbstractProperty, implement Value and Reference, remove this get_variant()
		variant&       get_pointed_variant()     { return is_connected_by_ref() ? m_input->m_variant : m_variant; }
        const variant& get_pointed_variant()const{ return is_connected_by_ref() ? m_input->m_variant : m_variant; }

        Property *           m_input;
        Visibility 		  m_visibility;
        Node*             m_owner;
        PropertyGrp *       m_parentProperties;
		std::vector<Property *>   m_outputs;
		Way                    m_allowed_connection;
		std::shared_ptr<Token> m_sourceToken;
		std::string            m_name;
		ndbl::variant       m_variant;
    };
}