#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class
#include "mirror.h"

namespace Nodable{
	/* BinaryOperationComponent is an interface for all binary operations */
	class BinaryOperationComponent: public Component{
	public:		
		BinaryOperationComponent() {};
		virtual ~BinaryOperationComponent(){};

		void                setLeft	 (Member* _value){left= _value;};
		void                setRight (Member* _value){right = _value;};
		void                setResult(Member* _value){result = _value;};
		void                setOperatorAsString(const char* _s) { operatorAsString = _s; }

		void                updateResultSourceExpression() const;

		std::string         getOperatorAsString()const{return operatorAsString;}
		/* return true if op needs to be evaluated before nextOp */
		static  bool        NeedsToBeEvaluatedFirst(std::string op, std::string nextOp);
	protected:
		Member* 	left 	= nullptr;
		Member* 	right 	= nullptr;
		Member* 	result 	= nullptr;
		std::string operatorAsString = "";
		MIRROR_CLASS(BinaryOperationComponent)(
			MIRROR_PARENT(Component)
		);
	};

	/* Implementation of the BinaryOperationComponent as a Sum */
	class Add : public BinaryOperationComponent{
	public:
		Add() {};
		bool update()override;
		MIRROR_CLASS(Add)(
			MIRROR_PARENT(BinaryOperationComponent)
		);
	};

	/* Implementation of the BinaryOperationComponent as a Subtraction */
	class Substract : public BinaryOperationComponent{
	public:
		Substract() {};
		bool update()override;
		MIRROR_CLASS(Substract)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as a Multiplication */
	class Multiply : public BinaryOperationComponent{
	public:
		Multiply() {};
		bool update()override;
		MIRROR_CLASS(Multiply)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as a Division */
	class Divide : public BinaryOperationComponent{
	public:
		Divide() {};
		bool update()override;
		MIRROR_CLASS(Divide)(
			MIRROR_PARENT(BinaryOperationComponent));
	};

	/* Implementation of the BinaryOperationComponent as an assignment */
	class Assign : public BinaryOperationComponent{
	public:
		Assign() {};
		bool update()override;
		MIRROR_CLASS(Assign)(
			MIRROR_PARENT(BinaryOperationComponent));
	};
}
