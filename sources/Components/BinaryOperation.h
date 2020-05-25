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
	class BinOperatorComponent: public FunctionComponent {
	public:		
		BinOperatorComponent(const std::string&, const Operator, const Language*);
		~BinOperatorComponent(){};
		void                setLeft	 (Member* _value);
		void                setRight (Member* _value);		
		void                setOperatorAsString(const char* _s) { operatorAsString = _s; }
		bool                update()override;
		void                updateResultSourceExpression() const override;
		std::string         getOperatorAsString()const{return operatorAsString;}

	protected:
		Member* 	left 	= nullptr;
		Member* 	right 	= nullptr;	
		const Operator prototype;
		std::string operatorAsString;
		MIRROR_CLASS(BinOperatorComponent)(
			MIRROR_PARENT(FunctionComponent)
		);
	};


	/**
	  * MultipleArgFunctionComponent is a class able to eval a function (using its prototype and a language)
	  */
	class MultipleArgFunctionComponent : public FunctionComponent {
	public:
		MultipleArgFunctionComponent(const Function _prototype, const Language* _language);
		~MultipleArgFunctionComponent() {};

		void setArg(size_t _index, const Member* _value) { args[_index] = _value; };
		bool update()override;
		void updateResultSourceExpression() const override;		
	protected:
		std::vector<const Member*> args;
		const Function prototype;

		MIRROR_CLASS(MultipleArgFunctionComponent)(
			MIRROR_PARENT(FunctionComponent)
			);
	};
}
