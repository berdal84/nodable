#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class
#include "mirror.h"
#include "Language.h"
#include <functional>

namespace Nodable{

	/* BinaryOperationComponent is an interface for all binary operations */
	class FunctionComponent : public Component {
	public:
		FunctionComponent(const Language* _language):language(_language){};
		virtual ~FunctionComponent(){};
		virtual void updateResultSourceExpression() const = 0;
		void         setResult(Member* _value) { result = _value; };
	protected:
		const Language* language;
		Member* result = nullptr;
		MIRROR_CLASS(FunctionComponent)(
			MIRROR_PARENT(Component)
		);
	};

	/* BinaryOperationComponent is an interface for all binary operations
	*
	* Note for later: This class and all derivate should be destroyed and replaced by an "OperatorComponent"
	*                 using prototypes but with different serialization and parsing methods.
	*/
	class BinaryOperationComponent: public FunctionComponent {
	public:		
		BinaryOperationComponent(const Language* _language) :FunctionComponent(_language) {};
		virtual ~BinaryOperationComponent(){};
		void                setLeft	 (Member* _value){left= _value;};
		void                setRight (Member* _value){right = _value;};		
		void                setOperatorAsString(const char* _s) { operatorAsString = _s; }
		void                updateResultSourceExpression() const override;
		std::string         getOperatorAsString()const{return operatorAsString;}

	protected:
		Member* 	left 	= nullptr;
		Member* 	right 	= nullptr;		
		std::string operatorAsString = "";
		MIRROR_CLASS(BinaryOperationComponent)(
			MIRROR_PARENT(FunctionComponent)
		);
	};

	/* Implementation of the BinaryOperationComponent as a Sum */
	class Add : public BinaryOperationComponent{
	public:
		Add(const Language* _language) :BinaryOperationComponent(_language) {};
		bool update()override;
		MIRROR_CLASS(Add)(
			MIRROR_PARENT(BinaryOperationComponent)
		);
	};

	/* Implementation of the BinaryOperationComponent as a Subtraction */
	class Subtract : public BinaryOperationComponent{
	public:
		Subtract(const Language* _language) :BinaryOperationComponent(_language) {};
		bool update()override;
		MIRROR_CLASS(Subtract)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as a Multiplication */
	class Multiply : public BinaryOperationComponent{
	public:
		Multiply(const Language* _language) :BinaryOperationComponent(_language) {};
		bool update()override;
		MIRROR_CLASS(Multiply)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as a Division */
	class Divide : public BinaryOperationComponent{
	public:
		Divide(const Language* _language) :BinaryOperationComponent(_language) {};
		bool update()override;
		MIRROR_CLASS(Divide)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as an assignment */
	class Assign : public BinaryOperationComponent{
	public:
		Assign(const Language* _language) :BinaryOperationComponent(_language) {};
		bool update()override;
		MIRROR_CLASS(Assign)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/**
	  * MultipleArgFunctionComponent is a class able to eval a function (using its prototype and a language)
	  */
	class MultipleArgFunctionComponent : public FunctionComponent {
	public:
		MultipleArgFunctionComponent(FunctionPrototype _prototype, const Language* _language);
		~MultipleArgFunctionComponent() {};

		void setArg(size_t _index, Member* _value) { args[_index] = _value; };
		bool update()override;
		void updateResultSourceExpression() const override;		
	protected:
		std::vector<Member*> args;
		FunctionPrototype prototype;

		MIRROR_CLASS(MultipleArgFunctionComponent)(
			MIRROR_PARENT(FunctionComponent)
			);
	};
}
