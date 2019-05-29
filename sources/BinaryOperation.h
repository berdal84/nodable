#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class

namespace Nodable{
	/* BinaryOperationComponent is an interface for all binary operations */
	class BinaryOperationComponent: public Component{
	public:		
		COMPONENT_CONSTRUCTOR(BinaryOperationComponent);
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
	};

	/* Implementation of the BinaryOperationComponent as a Sum */
	class Add : public BinaryOperationComponent{
	public:
		COMPONENT_CONSTRUCTOR(Add);
		void        update()override;	
	};

	/* Implementation of the BinaryOperationComponent as a Substraction */
	class Substract : public BinaryOperationComponent{
	public:
		COMPONENT_CONSTRUCTOR(Substract);
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as a Multiplication */
	class Multiply : public BinaryOperationComponent{
	public:
		COMPONENT_CONSTRUCTOR(Multiply);
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as a Division */
	class Divide : public BinaryOperationComponent{
	public:
		COMPONENT_CONSTRUCTOR(Divide);
		void update()override;
	};

	/* Implementation of the BinaryOperationComponent as an assignment */
	class Assign : public BinaryOperationComponent{
	public:
		COMPONENT_CONSTRUCTOR(Assign);
		void update()override;
	};
}
