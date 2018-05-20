#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class

namespace Nodable{
	/* BinaryOperationComponent is an interface for all binary operations */
	class BinaryOperationComponent: public Component{
	public:		
		BinaryOperationComponent(){};
		virtual ~BinaryOperationComponent(){};

		void                setLeft	 (Value* _value){left= _value;};
		void                setRight (Value* _value){right = _value;};
		void                setResult(Value* _value){result = _value;};
		/* return true if op needs to be evaluated before nextOp */
		static  bool        NeedsToBeEvaluatedFirst(std::string op, std::string nextOp);
	protected:
		Value* 	left 	= nullptr;
		Value* 	right 	= nullptr;
		Value* 	result 	= nullptr;
	};

	/* Implementation of the BinaryOperationComponent as a Sum */
	class Add : public BinaryOperationComponent{
	public:
		Add(){setMember("class", "Add");};
		void        update()override;
	};

	/* Implementation of the BinaryOperationComponent as a Substraction */
	class Substract : public BinaryOperationComponent{
	public:
		Substract(){setMember("class", "Substract");};
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as a Multiplication */
	class Multiply : public BinaryOperationComponent{
	public:
		Multiply(){setMember("class", "Multiply");};
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as a Division */
	class Divide : public BinaryOperationComponent{
	public:
		Divide(){setMember("class", "Divide");};
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as an assignment */
	class Assign : public BinaryOperationComponent{
	public:
		Assign(){setMember("class", "Add");};
		void update()override;
	};
}
