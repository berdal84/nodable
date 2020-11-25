#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include "Variant.h"
#include "Connector.h"
#include "Visibility.h"

#include <string>

namespace Nodable
{
    /**
     * The role of a Member is to store a value in an underlying Variant object. A Member always has an Object as owner and
     * should do not exists alone.
     * Like for a regular class member in a lot of programming languages you can set the Visibility and the Type of a Member.
     * But in Nodable, a Member can also be connected (see Wire) to another Member on its input or output Connector.
     */
	class Member
    {
	public:
        /**
         * Construct a new Member
         * @param _owner is the Object responsible for the Member.
         */
		explicit Member(Object* _owner);

		~Member();

        /**
         * Get if allows connections in a given direction
         * @return
         */
		[[nodiscard]] bool allowsConnection(Way)const;

		/**
		 * Get if the input is connected
		 * @return
		 */
		[[nodiscard]] bool hasInputConnected()const;

		/**
		 * To know if the member is defined.
		 * A member is undefined by default, it will stay until "set()" is called.
		 * @return
		 */
		[[nodiscard]] bool isDefined()const;

		/**
		 * Check if the Member's type corresponds to a given type or not.
		 * @return
		 */
		[[nodiscard]] bool isType(Type)const;


		/**
		 * Check for equality with another Member
		 * @return
		 */
		bool equals(const Member *)const;

		/**
		 * Set the direction of the Member's connection
		 */
		void setConnectorWay(Way);

		/**
		 * Set the source expression string for this Member.
		 * Nothing will be computed, the string will be stored for later use.
		 * TODO: remove this method.
		 */
		void setSourceExpression(const char*);

		/**
		 * Set an input Member. Can be nullptr to reset input.
		 */
		void setInputMember(Member*);

		/**
		 * Set a name.
		 */
		void setName(const char*);

        /**
         * Set value given another Member
         */
		void set(const Member*);

        /**
         * Set value given another Member
         */
		void set(const Member&);

        /**
         * Set value given a floating point number.
         */
		void set(double);

        /**
         * Set value given an integer.
         */
		void set(int);

        /**
         * Set value given a string.
         */
		void set(const std::string&);

        /**
         * Set value given a string.
         */
		void set(const char* _value);

        /**
         * Set value given a boolean.
         */
		void set(bool _value);

		/**
		 * Set a Type.
		 * Type can be changed at any time but data will be lost (cf. Variant::setType).
		 * @param _type
		 */
		void setType(Type _type);

		/**
		 * Change the visibility.
		 * Consult Visibility enum for more information.
		 * @param _v
		 */
		void setVisibility(Visibility _v);

		/**
		 * Get the owner Object
		 * @return a pointer to the owner Object.
		 */
		[[nodiscard]] Object* getOwner()const;

		/**
		 * Get the input connected Member
		 * @return a pointer to the connected Member.
		 */
		[[nodiscard]] Member* getInputMember()const;

		/**
		 * Get the Member's name.
		 * @return a std::string reference to the internal name member.
		 */
		[[nodiscard]] const std::string&  getName()const;

		/**
		 * Type getter
		 * @return this.data.getType()
		 */
		[[nodiscard]] Type getType()const;

        /**
         * Typename getter
         * @return this.data.getTypeAsString()
         */
		[[nodiscard]] std::string getTypeAsString()const;

		/**
		 * Get the Visibility
		 * @return
		 */
        [[nodiscard]] Visibility getVisibility()const;

        /**
         * Get the available connector ways.
         * Read Way enum documentation.
         * @return
         */
        [[nodiscard]] Way getConnectorWay()const;

        /**
         * Get the input Connector
         * @return a pointer to the input Connector
         */
        [[nodiscard]] const Connector* input() const;

        /**
         * Get the output Connector
         * @return a pointer to the output Connector
         */
        [[nodiscard]] const Connector* output() const;

        /**
         * Cast the underlying data to an integer.
         * @return
         */
		inline explicit operator int()const
		{
		    return (int)data;
		}

        /**
         * Cast the underlying data to a boolean.
         * @return
         */
        inline explicit operator bool()const
        {
            return (bool)data;
        }

        /**
         * Cast the underlying data to a floating point number.
         * @return
         */
        inline explicit operator double()const
        {
            return (double)data;
        }

        /**
         * Cast the underlying data to a string.
         * @return
         */
		inline explicit operator std::string()const
		{
		    return (std::string)data;
		}

	private:
        /**
         * The Object that owns this. Owner is responsible to create/delete this.
         */
		Object* owner;

		/**
		 * A possible input connected Member.
		 */
		Member* inputMember = nullptr;

		/**
		 * The source expression that the value of this Member should be the result.
		 */
		std::string         sourceExpression;

		/**
		 * A name for this Member.
		 */
		std::string 		name 				= "Unknown";

		/**
		 * The underlying data.
		 */
		Variant       		data;

		/**
		 * The visibility of this Member.
		 */
		Visibility 		    visibility = Visibility::Default;

		/**
		 * The input connector, nullptr (default) means no input connection is allowed.
		 */
		Connector*          in                  = nullptr;

        /**
         * The input connector, nullptr (default) means no output connections are allowed.
         */
		Connector*          out                 = nullptr;
	};
}