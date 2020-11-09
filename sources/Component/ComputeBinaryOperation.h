#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class
#include "mirror.h"
#include "Language.h"
#include "ComputeFunction.h"
#include <functional>

namespace Nodable{


	/* BinaryOperationComponent is an interface for all binary operations
	*
	* Note for later: This class and all derivate should be destroyed and replaced by an "OperatorComponent"
	*                 using prototypes but with different serialization and parsing methods.
	*/
	class ComputeBinaryOperation: public ComputeFunction {
	public:
	    ComputeBinaryOperation(): ComputeFunction() {}
		ComputeBinaryOperation(std::shared_ptr<const Operator>    _operator,
                               std::shared_ptr<const Language>    _language);
		~ComputeBinaryOperation() {};
		void                setLValue(std::shared_ptr<Member> _value);
		void                setRValue(std::shared_ptr<Member> _value);
		void                updateResultSourceExpression() const override;
		[[nodiscard]] inline std::shared_ptr<const Operator> ope()const { return std::static_pointer_cast<const Operator>(this->function); };
	protected:		
		MIRROR_CLASS(ComputeBinaryOperation)(
			MIRROR_PARENT(ComputeFunction)
		);
	};
}
