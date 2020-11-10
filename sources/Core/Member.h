#pragma once

#include "Type.h"
#include "Way.h"
#include "Visibility.h"
#include "Variant.h"
#include "Connector.h"

#include <memory>

namespace Nodable
{
    class Object;

	class Member
    {
	public:
		Member() = default;
		~Member() = default;

		[[nodiscard]] bool  allowsConnections(Way)const;
		[[nodiscard]] bool  isEditable()const;
		[[nodiscard]] bool  isSet()const;
        [[nodiscard]] bool  isType(Type)const;
        [[nodiscard]] bool  equals(const Member *)const;

		void                setAllowedConnections(Way _flags);
		void                setSourceExpression(const char*);
		void                setInputMember(std::shared_ptr<Member>);
		void  		        setName(const char*);
		void                setOwner(Object*);
		void                set(const std::shared_ptr<Member>&);
		void                set(double);
		void                set(int);
		void                set(std::string);
		void                set(const char* _value);
		void                set(bool _value);
		void                setType(Type _type);
		void                setVisibility(Visibility _v);

		/** Get the value of the inputMember Member and set it to this Member.
		    Warning: be sure the member has an inputMember before calling this (getInputMember()!=nullptr)*/
		void                updateValueFromInputMemberValue();

		[[nodiscard]] Object*             getOwner()const;
		[[nodiscard]] std::shared_ptr<Member> getInputMember()const;
		[[nodiscard]] const std::string&  getName()const;
		[[nodiscard]] std::string         getSourceExpression()const;
		[[nodiscard]] Type                getType()const;
		[[nodiscard]] std::string         getTypeAsString()const;
        [[nodiscard]] Visibility          getVisibility()const { return visibility; }
        [[nodiscard]] Way                 getConnectorWay()const;
        [[nodiscard]] const Connector*    input() const { return in.get(); }
        [[nodiscard]] const Connector*    output() const { return out.get(); }

		inline operator bool()const        { return data; }
		inline operator double()const      { return data; }
		inline operator int()const         { return data; }
		inline operator std::string()const { return data; }

	private:
		Object*     		owner       		= nullptr;
        std::shared_ptr<Member> inputMember;
		std::string         sourceExpression;
		std::string 		name 				= "Unknown";
		Variant       		data;
		Visibility 		    visibility 			= Visibility::Default;
		std::unique_ptr<Connector> in;
		std::unique_ptr<Connector> out;
	};
}