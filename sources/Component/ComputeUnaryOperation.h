#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Component.h" // base class
#include "mirror.h"
#include "Language.h"
#include "ComputeFunction.h"
#include <functional>

namespace Nodable {


	/* BinaryOperationComponent is an interface for all binary operations
	*
	* Note for later: This class and all derivate should be destroyed and replaced by an "OperatorComponent"
	*                 using prototypes but with different serialization and parsing methods.
	*/
	class ComputeUnaryOperation : public ComputeFunction {
	public:
	    ComputeUnaryOperation();
		ComputeUnaryOperation(const Operator*, const Language*);
		~ComputeUnaryOperation() {};
		void                setLValue(Member* _value);
		void                updateResultSourceExpression() const override;
		inline const Operator* getOperator()const { return reinterpret_cast<const Operator*>(this->function); };

	protected:
		MIRROR_CLASS(ComputeUnaryOperation)(
			MIRROR_PARENT(ComputeFunction)
			);
	};
}
