#include <nodable/Variant.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <cassert>
#include <nodable/Nodable.h>
#include <nodable/String.h>
#include <nodable/Node.h>

using namespace Nodable;

Variant::Variant(): m_isDefined(false)
{
}

Variant::~Variant(){};

Reflect::Type Variant::getType()const
{
	return Variant::s_nodableTypeByIndex.at(data.index());
}

bool  Variant::isType(Reflect::Type _type)const
{
	return getType() == _type;
}

void Variant::set(double _var)
{
	switch( getType() ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case Reflect::Type_String:
		{
			data.emplace<std::string>( std::to_string(_var) );
			break;
		}

        default:
		{
			data.emplace<double>( _var );
			break;
		}
	}
	m_isDefined = true;
}

void Variant::set(const std::string& _var)
{
   this->set(_var.c_str());
}

void Variant::set(const char* _var)
{
    switch (getType()) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case Reflect::Type_String:
        {
            data = _var;
        }

        default:
        {
            data = std::string(_var);
        }
    }
    m_isDefined = true;
}

void Variant::set(bool _var)
{
	switch(getType()) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		case Reflect::Type_String:
		{
			data.emplace<std::string>( _var ? "true" : "false" );
			break;
		}

		case Reflect::Type_Double:
		{
			data.emplace<double>( _var ? double(1) : double(0) );
			break;
		}

		default:
		{
			data.emplace<bool>( _var );
			break;
		}
	}
    m_isDefined = true;
}

bool Variant::isDefined()const
{
	return m_isDefined;
}

void Variant::undefine()
{
	m_isDefined = false;
}

void Variant::set(Node* _node)
{
    setType( Reflect::Type_Pointer ); // TODO: remove this
    data = _node;
    m_isDefined = true;
}

void Variant::set(const Variant* _other)
{
    setType(_other->getType());  // TODO: remove this
	data = _other->data;
    m_isDefined = _other->m_isDefined;
}

std::string Variant::getTypeAsString()const
{
    std::string result;

	if (  getType() == Reflect::Type_Pointer ) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        Node* _node = mpark::get<Node*>( data );
        if ( _node )
        {
            result.append( _node->get_class()->get_name() );
            result.append( "*" );
        }
        else
        {
            result.append( "Node*" );
        }
    }
	else
    {
        result.append( Reflect::to_string( getType() ) );
    }

	return result;
}

void Variant::setType(Reflect::Type _type) // TODO: remove this
{
	if (getType() != _type) // TODO: consider using State pattern (a single context with n possible states implementing an interface)
	{
		undefine();

		// Set a default value (this will change the type too)
		switch (_type)
		{
		case Reflect::Type_String:
			data.emplace<std::string>();
			break;
		case Reflect::Type_Double:
			data.emplace<double>();
			break;
		case Reflect::Type_Boolean:
			data.emplace<bool>();
			break;
		default:
            data.emplace<std::nullptr_t>();
			break;
		}
	}

}

template<>
[[nodiscard]] i64_t Variant::convert_to<i64_t>()const
{
    switch (getType())
    {
        case Reflect::Type_String:  return double( mpark::get<std::string>(data).size());
        case Reflect::Type_Double:  return mpark::get<double>(data);
        case Reflect::Type_Boolean: return mpark::get<bool>(data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] double Variant::convert_to<double>()const
{
    switch (getType())
    {
        case Reflect::Type_String:  return double( mpark::get<std::string>(data).size());
        case Reflect::Type_Double:  return mpark::get<double>(data);
        case Reflect::Type_Boolean: return mpark::get<bool>(data);
        default:           return double(0);
    }
}

template<>
[[nodiscard]] int Variant::convert_to<int>()const
{
	return (int)this->convert_to<double>();
}

template<>
[[nodiscard]] bool Variant::convert_to<bool>()const
{
    switch (getType())
    {
        case Reflect::Type_String:  return !mpark::get<std::string>(data).empty();
        case Reflect::Type_Double:  return mpark::get<double>(data) != 0.0F;
        case Reflect::Type_Boolean: return mpark::get<bool>(data);
        default:           return false;
    }
}

template<>
[[nodiscard]] std::string Variant::convert_to<std::string>()const
{
    std::string result;

    switch (getType())  // TODO: consider using State pattern (a single context with n possible states implementing an interface)
    {
        case Reflect::Type_String:
        {
            result.append( mpark::get<std::string>(data) );
            break;
        }

        case Reflect::Type_Double:
        {
            result.append(  String::from(mpark::get<double>(data)) );
            break;
        }

        case Reflect::Type_Boolean:
        {
            result.append( mpark::get<bool>(data) ? "true" : "false" );
            break;
        }

        case Reflect::Type_Pointer:
        {
            result.append("[") ;
            result.append(mpark::get<Node *>(data)->get_label() );
            result.append("]") ;
            break;
        }

        default:
        {
            result.append("<?>");
            break;
        }
    }

    return result;
}

Variant::operator int()const          { return (int)(double)*this; }
Variant::operator double()const       { return convert_to<double>(); }
Variant::operator bool()const         { return convert_to<bool>(); }
Variant::operator std::string ()const { return convert_to<std::string>(); }

void Variant::define()
{
    /* declare variant as "defined" without affecting a value, existing data will be used */
    m_isDefined = true;
}
